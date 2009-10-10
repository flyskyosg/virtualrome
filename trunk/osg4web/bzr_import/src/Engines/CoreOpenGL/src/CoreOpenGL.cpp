#include <string>
#include <iostream>

#include <CoreOpenGL/CoreOpenGL.h>

#include <gl/gl.h>

using namespace CommonCore;


//CoreOpenGL Costruttore
CoreOpenGL::CoreOpenGL(std::string corename) : CoreInterface(corename),
		_CurrWidth(550), 
		_CurrHeight(550),
		_hWnd(NULL),
		_hRC(NULL),
		_hDC(NULL),
		_theta(0.0f),
		_done(false)
{
#if defined(_DEBUG)
	//Inizializzo il Debug
	if(! this->initializeLogMessages() )
		this->sendWarnMessage("CoreOpenGL -> Error redirecting messages."); 

	this->sendNotifyMessage("CoreOpenGL -> Starting Log Redirection.");
#endif

	this->sendNotifyMessage("CoreOpenGL -> Costructing CoreOpenGL Instance.");

	_InstDir.clear();
}

//CoreOpenGL Distruttore
CoreOpenGL::~CoreOpenGL()
{
	this->sendNotifyMessage("~CoreOpenGL -> Destructing CoreOpenGL Instance.");

#if defined(WIN32)
	wglMakeCurrent( NULL, NULL );
	wglDeleteContext( _hRC );
	ReleaseDC( _hWnd, _hDC );
#else
	//TODO: Linux
#endif

	_hRC = NULL;
	_hDC = NULL;
	_hWnd = NULL;

	this->sendNotifyMessage("~CoreOpenGL -> Stopping Log Redirection if Present.");

	if(this->isLogMessagesInitialized())
		if(! this->restoreLogMessages() )
			this->sendWarnMessage("~CoreOpenGL -> Error closing messages redirection."); 

}

#if defined(WIN32)
bool CoreOpenGL::InitCore(WINDOWIDTYPE mhWnd, std::string instdir, std::string options) 
{
	_InstDir = instdir; //Directory di installazione del Core attuale
	_hWnd = mhWnd;

	this->sendNotifyMessage("InitCore -> Starting Core Initialization.");

	// Local Variable to hold window size data
	RECT rect;
	// Get the current window size
	::GetWindowRect(_hWnd, &rect);
	
	int ww, hh;
	ww = rect.right - rect.left; 
	hh = rect.bottom - rect.top;

	if(ww != 0)
		_CurrWidth = ww;

	if(hh != 0)
		_CurrHeight = hh;


	PIXELFORMATDESCRIPTOR pfd;
	int format;
	
	// get the device context (DC)
	_hDC = GetDC( _hWnd );
	
	// set the pixel format for the DC
	ZeroMemory( &pfd, sizeof( pfd ) );
	pfd.nSize = sizeof( pfd );
	pfd.nVersion = 1;
	pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
	pfd.iPixelType = PFD_TYPE_RGBA;
	pfd.cColorBits = 24;
	pfd.cDepthBits = 16;
	pfd.iLayerType = PFD_MAIN_PLANE;
	format = ChoosePixelFormat( _hDC, &pfd );
	SetPixelFormat( _hDC, format, &pfd );
	
	// create and enable the render context (RC)
	_hRC = wglCreateContext( _hDC );
	wglMakeCurrent( _hDC, _hRC );

	 
	return true;
}
#else //Linux... TODO:
bool CoreOpenGL::InitCore(Display*, WINDOWIDTYPE, std::string, std::string)
{
	//TODO: implement me!!!!

	return false;
}
#endif

//Passata di Rendering
bool CoreOpenGL::RenderScene()
{
	if(!this->isDone())
	{
		// OpenGL animation code goes here
			
		glClearColor( 0.0f, 0.0f, 0.0f, 0.0f );
		glClear( GL_COLOR_BUFFER_BIT );
			
		glPushMatrix();
		glRotatef( _theta, 0.0f, 0.0f, 1.0f );
		glBegin( GL_TRIANGLES );
		glColor3f( 1.0f, 0.0f, 0.0f ); glVertex2f( 0.0f, 1.0f );
		glColor3f( 0.0f, 1.0f, 0.0f ); glVertex2f( 0.87f, -0.5f );
		glColor3f( 0.0f, 0.0f, 1.0f ); glVertex2f( -0.87f, -0.5f );
		glEnd();
		glPopMatrix();
			
		SwapBuffers( _hDC );
			
		_theta += 1.0f;
	}
	else
	{
		this->sendWarnMessage("RenderScene -> Core not initialized, Rendering skipped!");
		return false;
	}

	return true;
}

