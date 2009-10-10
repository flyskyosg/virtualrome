#ifndef __OSG4WEB_WINDOWWIN32__
#define __OSG4WEB_WINDOWWIN32__ 1


#define WGL_DRAW_TO_WINDOW_ARB			0x2001
#define WGL_SUPPORT_OPENGL_ARB			0x2010
#define WGL_ACCELERATION_ARB			0x2003
#define WGL_FULL_ACCELERATION_ARB		0x2027
#define WGL_PIXEL_TYPE_ARB				0x2013
#define WGL_TYPE_RGBA_ARB				0x202B
#define WGL_COLOR_BITS_ARB				0x2014
#define WGL_RED_BITS_ARB				0x2015
#define WGL_GREEN_BITS_ARB				0x2017
#define WGL_BLUE_BITS_ARB				0x2019
#define WGL_DEPTH_BITS_ARB				0x2022
#define WGL_DOUBLE_BUFFER_ARB			0x2011

#include <windows.h>
#include <windowsx.h>
#include <gl/gl.h>

#include <map>
#include <iostream>
#include <sstream>

#include <CoreOpenGL/WindowExtra.h>


/***********************************************************************
 *
 * from Roberto Gori
 *
 ***********************************************************************/

template <class T>
std::string toString(T value)
{
	std::stringstream ss;
 ss << value;
 return ss.str();  
}


/***********************************************************************
 *
 * OSG WindowSupport Class
 *
 ***********************************************************************/
class WindowSupport
{
public:
	//Costruttore/Distruttore
	WindowSupport() : _windowClassesRegistered(false), 
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

	~WindowSupport()
	{
		this->destroyWindow();

		if(WindowSupport::getWindowInterface()->getActiveWindowSize() == 0)
			WindowSupport::getWindowInterface(true);
	}

	static LRESULT CALLBACK WindowSupportWin32Proc( HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
	{
		WindowSupport *window =  WindowSupport::getWindowInterface()->getWindowSupportFor(hwnd);
		return window ? window->handleNativeWindowSupportEvent(hwnd, uMsg, wParam, lParam) : ::DefWindowProc(hwnd, uMsg, wParam, lParam);
	}
	
	virtual LRESULT handleNativeWindowSupportEvent(HWND hWnd, UINT eventmsg, WPARAM wParam, LPARAM lParam) { return (0L); };

protected:
	class Win32Stack
	{
	public:
		Win32Stack(){ }
		~Win32Stack() { _activeWindows.clear(); }

		// Map Win32 window handles to WindowSupport instance
	    typedef std::pair< HWND, WindowSupport* >  WindowHandleEntry;
	    typedef std::map<  HWND, WindowSupport* >  WindowHandles;

		void registerWindow( HWND hwnd, WindowSupport* window )
		{
			if (hwnd) 
				_activeWindows.insert(WindowHandleEntry(hwnd, window));
		}

		void unregisterWindow( HWND hwnd )
		{
		    if (hwnd) 
				_activeWindows.erase(hwnd);
		}

		WindowSupport* getWindowSupportFor( HWND hwnd )
		{
			Win32Stack::WindowHandles::const_iterator entry = _activeWindows.find(hwnd);
			return entry==_activeWindows.end() ? 0 : entry->second;
		}


		unsigned int getActiveWindowSize(){ return _activeWindows.size(); };

	private:
		
		WindowHandles _activeWindows;
	};

	//Funzione di inizializzazione della finestra
	bool initializeWindow(HWND handle)
	{
		if (_initialized)
		{
			logError("WindowSupport::initializeWindow() - Window already created; it cannot be changed", ::GetLastError());
			return false;
		}

		if (handle==0)
		{
			logError("WindowSupport::initializeWindow() - Invalid window handle passed", ::GetLastError());
			return false;
		}

		_hwnd = handle;
		if (_hwnd==0)
		{
			logError("WindowSupport::initializeWindow() - Unable to retrieve native window handle", ::GetLastError());
			return false;
		}
	
		_hdc = ::GetDC(_hwnd);
		if (_hdc==0)
		{
			logError("WindowSupport::initializeWindow() - Unable to get window device context", ::GetLastError());
			_hwnd = 0;
			return false;
		}
	
	    if (!this->setPixelFormat())
		{	
			logError("WindowSupport::initializeWindow() - Unable to set the inherited window pixel format", ::GetLastError());
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

		WindowSupport::getWindowInterface()->registerWindow(_hwnd, this);

		this->refreshWindowDimensions();

		_initialized = true;

		return true;
	}

	//Funzione di distruzione di finestra
	void destroyWindow()
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
	        WindowSupport::getWindowInterface()->unregisterWindow(_hwnd);
	        _hwnd = 0;
	    }

	    _initialized = false;
	}

	//Selezione del pixel format ereditato
	bool setPixelFormat()
	{
/*
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
*/
		PIXELFORMATDESCRIPTOR pixelFormat;
		ZeroMemory(&pixelFormat, sizeof(PIXELFORMATDESCRIPTOR));
		pixelFormat.nSize = sizeof(PIXELFORMATDESCRIPTOR);
		pixelFormat.nVersion = 1;
		pixelFormat.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL |  PFD_DOUBLEBUFFER | PFD_SWAP_COPY;
 

		int pixelFormatIndex = ::ChoosePixelFormat(_hdc, &pixelFormat);

		logError("WindowSupport::ChoosePixelFormat()-->" + toString(pixelFormatIndex));


		if (pixelFormatIndex == 0)
		{
			logError("WindowSupport::setPixelFormat() - No matching pixel format found based on traits specified");
			return false;
		}

		::DescribePixelFormat(_hdc, pixelFormatIndex ,sizeof(PIXELFORMATDESCRIPTOR),&pixelFormat);
			
		if( ((pixelFormat.dwFlags & PFD_GENERIC_FORMAT) != 0)  && ((pixelFormat.dwFlags & PFD_GENERIC_ACCELERATED) == 0) )
		{
			logError("WindowSupport::setPixelFormat() - Rendering in software");
		}

		PIXELFORMATDESCRIPTOR pfd;
		::memset(&pfd, 0, sizeof(pfd));
		pfd.nSize    = sizeof(PIXELFORMATDESCRIPTOR);
		pfd.nVersion = 1;
	
		if (!::SetPixelFormat(_hdc, pixelFormatIndex, &pfd))
		{
			logError("WindowSupport::setPixelFormat() - Unable to set pixel format");
			return false;
		}

		_hglrc = ::wglCreateContext(_hdc);
		if (_hglrc==0)
		{
			logError("WindowSupport::setPixelFormat() - Unable to create OpenGL rendering context", ::GetLastError());
			return false;
		}

		return true;
	}

	//Richiesta Make Current per il contesto
	bool makeCurrentImplementation()
	{
		if (!_initialized)
		{
			logError("WindowSupport::makeCurrentImplementation() - Window not initialized; cannot do makeCurrent.", 0);
			return false;
		}

		_holddc = ::wglGetCurrentDC();
		_holdrc = ::wglGetCurrentContext();

		//if(_current)
		//	return true;

		//TODO: controllare mi rallenta tutto da paura

		if (!::wglMakeCurrent(_hdc, _hglrc))
		{
			logError("WindowSupport::makeCurrentImplementation() - Unable to set current OpenGL rendering context", ::GetLastError());
			return false;
		}

		_current = true;

		return true;
	}

	//Ripristina il contesto precedente
	bool restoreCurrentImplementation()
	{
		::wglMakeCurrent( _holddc, _holdrc );
		_current = false;

		return true;
	}

	//Rilascia il contesto
	bool releaseContextImplementation()
	{
	    ::wglMakeCurrent(_hdc, NULL);
	    
		_current = false;

		return true;
	}
	
	//Swap Buffer
	void swapBuffersImplementation()
	{
		if (!_initialized) 
			return;
	
		::SwapBuffers(_hdc);
	}

	//Registrazione delle procedure di finestra (WindProc)
	bool registerWindowProcedure()
	{
		::SetLastError(0);
		_windowProcedure = (WNDPROC)::SetWindowLongPtr(_hwnd, GWLP_WNDPROC, LONG_PTR(WindowSupport::WindowSupportWin32Proc));
		unsigned int error = ::GetLastError();
	
		if (_windowProcedure==0 && error)
		{
			logError("WindowSupport::registerWindowProcedure() - Unable to register window procedure", error);
			return false;
		}

		return true;
	}

	//DeRegistrazione delle procedure di finestra
	bool unregisterWindowProcedure()
	{
		if (_windowProcedure == 0 || _hwnd == 0) 
			return true;

		::SetLastError(0);
		WNDPROC wndProc = (WNDPROC)::SetWindowLongPtr(_hwnd, GWLP_WNDPROC, LONG_PTR(_windowProcedure));
		unsigned int error = ::GetLastError();

		if (wndProc == 0 && error)
		{
			logError("WindowSupport::unregisterWindowProcedure() - Unable to unregister window procedure", error);
			return false;
		}

		_windowProcedure = 0;
		return true;
	}

	void refreshWindowDimensions()
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

	//Check the glcontest is make current
	bool isCurrent() { return _current; }

	//Show simple error messages
	void logError( const std::string& msg )
	{
		std::cout << "Error: " << msg.c_str() << std::endl;
	}

	//Format window error messages
	void logError( const std::string& msg, unsigned int errorCode )
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

	//Get Current WindowDimension structure
	WindowExtra::WindowDimension getWindowsDimension() { return _windowDimension; }

	//Get current active window stack
	static Win32Stack* getWindowInterface(bool erase = false)
	{ 
		static WindowSupport::Win32Stack* win32stack = new WindowSupport::Win32Stack();

		if(erase)
		{
			delete win32stack;
			win32stack = NULL;
		}

		return win32stack; 
	}

private:
	std::string _windowNameWithCursor, _windowNameWithoutCursor;

	bool _windowClassesRegistered;

	bool _initialized;
	bool _realized;
	bool _current;

	WindowExtra::WindowDimension _windowDimension;

	HWND _hwnd;
	HDC _hdc, _holddc;
	HGLRC _hglrc, _holdrc;
	HCURSOR _currentCursor;
	WNDPROC _windowProcedure;

};


#endif //__OSG4WEB_WINDOWWIN32__




