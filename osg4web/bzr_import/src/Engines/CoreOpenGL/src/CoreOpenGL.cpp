#include <string>
#include <iostream>

#include <CoreOpenGL/CoreOpenGL.h>


#include <gl/gl.h>
#include <gl/glu.h>

using namespace CommonCore;

//CoreOpenGL Costruttore
CoreOpenGL::CoreOpenGL(std::string corename) : CoreInterface(corename),
#if defined(WIN32)
		WindowWin32(), //Costruttore di WindowWin32
#else
		//TODO: Linux...
#endif
		_needresize(true),
		_openglinit(false),
		_theta(0.0),
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

	_done = true;

	this->sendNotifyMessage("~CoreOpenGL -> Stopping Log Redirection if Present.");

	if(this->isLogMessagesInitialized())
		if(! this->restoreLogMessages() )
			this->sendWarnMessage("~CoreOpenGL -> Error closing messages redirection.");
}

#if defined(WIN32)
bool CoreOpenGL::InitCore(WINDOWIDTYPE mhWnd, std::string instdir, std::string options) 
{
	this->sendNotifyMessage("InitCore -> Inizializing CoreOpenGL Instance.");

	_InstDir = instdir; //Directory di installazione del Core attuale
	
	return this->initializeWindow(mhWnd);
}

LRESULT CoreOpenGL::handleNativeWindowWin32Event(HWND hWnd, UINT eventmsg, WPARAM wParam, LPARAM lParam)
{
	switch (eventmsg) 
	{
	case WM_MOVE:
	case WM_SIZE:
	{	
		this->refreshWindowDimensions();
		_needresize = true;
	}
		break;
	case WM_ERASEBKGND:
		return (0L);
	default:
		break;
	}
	
	//TODO: fare gli eventi tastiera e mouse

	return ::DefWindowProc(hWnd, eventmsg, wParam, lParam);
}

#else //Linux... TODO:
bool CoreOpenGL::InitCore(Display*, WINDOWIDTYPE, std::string, std::string)
{
	//TODO: implement me!!!!

	return false;
}

//FIXME: finire x linux. Intestazione sbagliata
bool CoreOpenGL::handleWindowEvents(HWND hWnd, UINT eventmsg, WPARAM wParam, LPARAM lParam)
{

	return true;
}

#endif


//Passata di Rendering
bool CoreOpenGL::RenderScene()
{
	if(!this->isDone())
	{
		//IMPORTANTE:	ricordarsi sempre di tenere le direttive OGL all'interno del thread chiamante altrimenti 
		//				gli stati si corrompono e vengono ignorati
		if(!_openglinit)
		{
			this->makeCurrentImplementation();
			this->initializeOpenGL();
			_openglinit = true;
		}

		if(_needresize)
		{
			WindowSupport::WindowDimension dimension = this->getWindowsDimension();
			this->windowResize(dimension.getX(), dimension.getY(), dimension.getWidth(), dimension.getHeight());
			_needresize = false;
		}

		this->renderImplementation();
		this->swapBuffersImplementation();
	}
	else
	{
		this->sendWarnMessage("RenderScene -> Core not initialized, Rendering skipped!");
		return false;
	}

	return true;
}

bool CoreOpenGL::renderImplementation()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glLoadIdentity();
	
	gluLookAt(0.0, 2.5, -11.0, 0.0, 0.75, -5.0, 0.0, 1.0, 0.0);
		
	this->drawDemoGridAndCube();
	
	return true;
}

void CoreOpenGL::drawDemoGridAndCube()
{
	glPushMatrix();
	glRotatef( _theta, 0.0f, 1.0f, 0.0f );
	for(float i = -500; i <= 500; i += 5)
	{
		glBegin(GL_LINES);
			glColor3ub(150, 190, 150);						
			glVertex3f(-500, 0, i);					
			glVertex3f(500, 0, i);
			glVertex3f(i, 0,-500);							
			glVertex3f(i, 0, 500);
		glEnd();
	}
	
	glTranslatef(0,1.0f,0);
	glBegin(GL_QUADS);						
		glColor3f(0.0f,1.0f,0.0f);			
		glVertex3f( 1.0f, 1.0f,-1.0f);		
		glVertex3f(-1.0f, 1.0f,-1.0f);		
		glVertex3f(-1.0f, 1.0f, 1.0f);		
		glVertex3f( 1.0f, 1.0f, 1.0f);		
		glColor3f(1.0f,0.5f,0.0f);			
		glVertex3f( 1.0f,-1.0f, 1.0f);		
		glVertex3f(-1.0f,-1.0f, 1.0f);		
		glVertex3f(-1.0f,-1.0f,-1.0f);		
		glVertex3f( 1.0f,-1.0f,-1.0f);
		glColor3f(1.0f,0.0f,0.0f);			
		glVertex3f( 1.0f, 1.0f, 1.0f);		
		glVertex3f(-1.0f, 1.0f, 1.0f);		
		glVertex3f(-1.0f,-1.0f, 1.0f);		
		glVertex3f( 1.0f,-1.0f, 1.0f);		
		glColor3f(1.0f,1.0f,0.0f);			
		glVertex3f( 1.0f,-1.0f,-1.0f);		
		glVertex3f(-1.0f,-1.0f,-1.0f);		
		glVertex3f(-1.0f, 1.0f,-1.0f);		
		glVertex3f( 1.0f, 1.0f,-1.0f);		
		glColor3f(0.0f,0.0f,1.0f);			
		glVertex3f(-1.0f, 1.0f, 1.0f);		
		glVertex3f(-1.0f, 1.0f,-1.0f);		
		glVertex3f(-1.0f,-1.0f,-1.0f);		
		glVertex3f(-1.0f,-1.0f, 1.0f);		
		glColor3f(1.0f,0.0f,1.0f);			
		glVertex3f( 1.0f, 1.0f,-1.0f);		
		glVertex3f( 1.0f, 1.0f, 1.0f);		
		glVertex3f( 1.0f,-1.0f, 1.0f);		
		glVertex3f( 1.0f,-1.0f,-1.0f);		
	glEnd();
	glPopMatrix();

	_theta += 0.75f;
}

bool CoreOpenGL::windowResize(int windowX, int windowY, int windowWidth, int windowHeight)
{
	glViewport(windowX, windowY, (GLsizei) windowWidth, (GLsizei) windowHeight);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	gluPerspective(45.0f, (GLfloat) windowWidth / (GLfloat) windowHeight, 0.1f, 100.0f);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	return true;
}

void CoreOpenGL::initializeOpenGL()
{
	glShadeModel(GL_SMOOTH);
	glClearColor(0.0f, 0.0f, 0.0f, 0.5f);
	glClearDepth(1.0f);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
}
