
#include <CoreOpenGL/WindowWin32.h>

#include <sstream>
#include <windowsx.h>


///////////////////////////////////////////////////////////////////////////////
// WindowWin32::Win32Stack

WindowWin32::Win32Stack::~Win32Stack()
{
	_activeWindows.clear();
}

WindowWin32::Win32Stack* WindowWin32::getWindowInterface(bool erase)
{ 
	static WindowWin32::Win32Stack* win32stack = new WindowWin32::Win32Stack();

	if(erase)
	{
		delete win32stack;
		win32stack = NULL;
	}

	return win32stack; 
}

void WindowWin32::Win32Stack::registerWindow( HWND hwnd, WindowWin32* window )
{
    if (hwnd) 
		_activeWindows.insert(WindowHandleEntry(hwnd, window));
}

void WindowWin32::Win32Stack::unregisterWindow( HWND hwnd )
{
    if (hwnd) 
		_activeWindows.erase(hwnd);
}

WindowWin32* WindowWin32::Win32Stack::getWindowWin32For( HWND hwnd )
{
	Win32Stack::WindowHandles::const_iterator entry = _activeWindows.find(hwnd);
    return entry==_activeWindows.end() ? 0 : entry->second;
}


///////////////////////////////////////////////////////////////////////////////
// WindowWin32 static function

LRESULT CALLBACK WindowWin32::WindowWin32Proc( HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	WindowWin32 *window =  WindowWin32::getWindowInterface()->getWindowWin32For(hwnd); //(WindowWin32 *)(LONG_PTR) GetWindowLong(hwnd, GWL_USERDATA); //SKAZZATO!!!
	return window ? window->handleNativeWindowWin32Event(hwnd, uMsg, wParam, lParam) : ::DefWindowProc(hwnd, uMsg, wParam, lParam);
}

///////////////////////////////////////////////////////////////////////////////
// WindowWin32

WindowWin32::WindowWin32() : _windowClassesRegistered(false), 
	_initialized(false),
	_realized(false),
	_current(false),
	_hwnd(NULL),
	_hdc(NULL),
	_hglrc(NULL),
	_currentCursor(NULL),
	_windowProcedure(NULL)
{

}

WindowWin32::~WindowWin32()
{
	this->destroyWindow();

	if(WindowWin32::getWindowInterface()->getActiveWindowSize() == 0)
		WindowWin32::getWindowInterface(true);
}

bool WindowWin32::initializeWindow(HWND handle)
{
	if (_initialized)
	{
		logError("WindowWin32::initializeWindow() - Window already created; it cannot be changed", ::GetLastError());
		return false;
	}

	if (handle==0)
	{
		logError("WindowWin32::initializeWindow() - Invalid window handle passed", ::GetLastError());
		return false;
	}

	_hwnd = handle;
	if (_hwnd==0)
	{
		logError("WindowWin32::initializeWindow() - Unable to retrieve native window handle", ::GetLastError());
		return false;
	}

	_hdc = ::GetDC(_hwnd);
	if (_hdc==0)
	{
		logError("WindowWin32::initializeWindow() - Unable to get window device context", ::GetLastError());
		_hwnd = 0;
		return false;
	}

    if (!this->setPixelFormat())
	{
		logError("WindowWin32::initializeWindow() - Unable to set the inherited window pixel format", ::GetLastError());
		_hdc  = 0;
		_hwnd = 0;
		return false;
	}

	if (!this->registerWindowProcedure())
    {
        ::wglDeleteContext(_hglrc);
        _hglrc = 0;
        ::ReleaseDC(_hwnd, _hdc);
        _hdc  = 0;
        _hwnd = 0;
        return false;
    }

	WindowWin32::getWindowInterface()->registerWindow(_hwnd, this);

	this->refreshWindowDimensions();

	_initialized = true;

	return true;
}

bool WindowWin32::setPixelFormat()
{
	PIXELFORMATDESCRIPTOR pixelFormat = {
		sizeof(PIXELFORMATDESCRIPTOR), 1,
		PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL |  PFD_DOUBLEBUFFER, 
		PFD_TYPE_RGBA, 
		16,
		0, 0, 0, 0, 0, 0,
		0,
		0,
		0,
		0, 0, 0, 0,
		16,
		0,
		0,
		PFD_MAIN_PLANE,
		0,
		0, 0, 0
	};

	int pixelFormatIndex = ::ChoosePixelFormat(_hdc, &pixelFormat);
	if (pixelFormatIndex == 0)
	{
		logError("WindowWin32::setPixelFormat() - No matching pixel format found based on traits specified");
		return false;
	}

	::DescribePixelFormat(_hdc, pixelFormatIndex ,sizeof(PIXELFORMATDESCRIPTOR),&pixelFormat);
		
	if( ((pixelFormat.dwFlags & PFD_GENERIC_FORMAT) != 0)  && ((pixelFormat.dwFlags & PFD_GENERIC_ACCELERATED) == 0) )
	{
		logError("WindowWin32::setPixelFormat() - Rendering in software");
	}

	PIXELFORMATDESCRIPTOR pfd;
	::memset(&pfd, 0, sizeof(pfd));
	pfd.nSize    = sizeof(PIXELFORMATDESCRIPTOR);
	pfd.nVersion = 1;

	if (!::SetPixelFormat(_hdc, pixelFormatIndex, &pfd))
	{
		logError("WindowWin32::setPixelFormat() - Unable to set pixel format");
		return false;
	}

	_hglrc = ::wglCreateContext(_hdc);
	if (_hglrc==0)
	{
		logError("WindowWin32::setPixelFormat() - Unable to create OpenGL rendering context", ::GetLastError());
		return false;
	}

	return true;
}

bool WindowWin32::registerWindowProcedure()
{
	::SetLastError(0);
	_windowProcedure = (WNDPROC)::SetWindowLongPtr(_hwnd, GWLP_WNDPROC, LONG_PTR(WindowWin32::WindowWin32Proc));
	unsigned int error = ::GetLastError();

	if (_windowProcedure==0 && error)
	{
		logError("WindowWin32::registerWindowProcedure() - Unable to register window procedure", error);
		return false;
	}

	return true;
}


bool WindowWin32::unregisterWindowProcedure()
{
	if (_windowProcedure == 0 || _hwnd == 0) 
		return true;

	::SetLastError(0);
	WNDPROC wndProc = (WNDPROC)::SetWindowLongPtr(_hwnd, GWLP_WNDPROC, LONG_PTR(_windowProcedure));
	unsigned int error = ::GetLastError();

	if (wndProc == 0 && error)
	{
		logError("WindowWin32::unregisterWindowProcedure() - Unable to unregister window procedure", error);
		return false;
	}

	_windowProcedure = 0;
	return true;
}

bool WindowWin32::makeCurrentImplementation()
{
	if (!_initialized)
	{
		logError("WindowWin32::makeCurrentImplementation() - Window not initialized; cannot do makeCurrent.", 0);
		return false;
	}

	//if(_current)
	//	return true;

	if (!::wglMakeCurrent(_hdc, _hglrc))
	{
		logError("WindowWin32::makeCurrentImplementation() - Unable to set current OpenGL rendering context", ::GetLastError());
		return false;
	}

	_current = true;

	return true;
}

bool WindowWin32::releaseContextImplementation()
{
    if (!::wglMakeCurrent(_hdc, NULL))
    {
        logError("WindowWin32::releaseContextImplementation() - Unable to release current OpenGL rendering context", ::GetLastError());
		return false;
	}

	_current = false;

	return true;
}

void WindowWin32::swapBuffersImplementation()
{
	if (!_initialized) 
		return;
	
	if (!::SwapBuffers(_hdc))
		logError("GraphicsWindowWin32::swapBuffersImplementation() - Unable to swap display buffers", ::GetLastError());
}

void WindowWin32::refreshWindowDimensions()
{
	POINT origin;
	origin.x = 0;
	origin.y = 0;

	::ClientToScreen(_hwnd, &origin);
	
	int windowX = origin.x;
	int windowY = origin.y;
	
	RECT clientRect;
	::GetClientRect(_hwnd, &clientRect);

	int windowWidth = (clientRect.right == 0) ? 1 : clientRect.right ;
	int windowHeight = (clientRect.bottom == 0) ? 1 : clientRect.bottom;

	_windowDimension.set(windowX, windowY, windowWidth, windowHeight);
}

void WindowWin32::destroyWindow()
{
	if (_hdc)
	{
		this->releaseContextImplementation();

        if (_hglrc)
        {
            ::wglDeleteContext(_hglrc);
            _hglrc = 0;
        }

        ::ReleaseDC(_hwnd, _hdc);
        _hdc = 0;
    }

    this->unregisterWindowProcedure();

    if (_hwnd)
    {
        WindowWin32::getWindowInterface()->unregisterWindow(_hwnd);
        _hwnd = 0;
    }

    _initialized = false;
}

void WindowWin32::logError( const std::string& msg )
{
	std::cout << "Error: " << msg.c_str() << std::endl;
}

void WindowWin32::logError( const std::string& msg, unsigned int errorCode )
{
    if (errorCode==0)
    {
        logError(msg);
        return;
    }

	std::cout << "Windows Error #"   << errorCode << ": " << msg.c_str();

    LPVOID lpMsgBuf;

    if (FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                      NULL,
                      errorCode,
                      0, // Default language
                      (LPTSTR) &lpMsgBuf,
                      0,
                      NULL)!=0)
    {
        std::cout << ". Reason: " << LPTSTR(lpMsgBuf) << std::endl;
        ::LocalFree(lpMsgBuf);
    }
    else
    {
		std::cout << ". Error formatting message" << std::endl;
    }
}
