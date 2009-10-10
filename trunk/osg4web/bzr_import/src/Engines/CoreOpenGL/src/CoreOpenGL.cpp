#include <string>
#include <iostream>

#include <CoreOpenGL/CoreOpenGL.h>
#include <CoreOpenGL/Demo/Teapot.h>

using namespace CommonCore;

//CoreOpenGL Costruttore
CoreOpenGL::CoreOpenGL(std::string corename) : CoreInterface(corename), CommandSchedule("COREOPENGL"),
		WindowSupport(),
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

//Setta le opzioni del core successivamente alla inizializzazione
void CoreOpenGL::AddStartOptions(std::string str, bool erase)
{
	this->sendNotifyMessage("AddStartOptions -> Adding Starting Options.");

	//Default value
	this->setCommandAction("UNKNOWN_ACTION");

	//Registro CommandSchedule corrente
	this->addCommandSchedule((CommandSchedule*) this);
	
	this->DoCommand(str); //Passo la stringa a DoCommand che la gestisce
}

//Gestisce i comandi provenineti dalla Shell e da JavaScript 
std::string CoreOpenGL::DoCommand(std::string command)
{
	std::string lcommand, rcommand;

	this->sendNotifyMessage("DoCommand -> CoreOpenGL Command: " + command);
	this->splitActionCommand(command, lcommand, rcommand);

	CommandSchedule* cs = _CommandRegistry[lcommand];
	
	if(cs)
		return cs->handleAction(rcommand);
	
	this->sendWarnMessage("DoCommand -> Command not Handled: " + lcommand + " Argument: " + rcommand);
	return "CORE_BADCOMMAND";
}

#if defined(WIN32)
bool CoreOpenGL::InitCore(WINDOWIDTYPE mhWnd, std::string instdir, std::string options) 
{
	this->sendNotifyMessage("InitCore -> Inizializing CoreOpenGL Instance.");

	_InstDir = instdir; //Directory di installazione del Core attuale
	
	return this->initializeWindow(mhWnd);
}

LRESULT CoreOpenGL::handleNativeWindowSupportEvent(HWND hWnd, UINT eventmsg, WPARAM wParam, LPARAM lParam)
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
		
	this->drawDemoGridAndTeapot();
	
	return true;
}

void CoreOpenGL::drawDemoGridAndTeapot()
{
	glPushMatrix();
	glRotatef( _theta, 0.0f, 1.0f, 0.0f );

	glDisable(GL_LIGHTING);
	for(float i = -500; i <= 500; i += 5)
	{
		glBegin(GL_LINES);
			glColor3ub(50, 130, 50);						
			glVertex3f(-500, 0, i);					
			glVertex3f(500, 0, i);
			glVertex3f(i, 0,-500);							
			glVertex3f(i, 0, 500);
		glEnd();
	}
	glEnable(GL_LIGHTING);
	
	glColor3ub(150, 190, 150);
	glTranslatef(0.0f, 1.3f, 0.0f);
	OpenGLTeapotExample::SolidTeapot(1.8);
	glPopMatrix();

	_theta += 0.75f;
}

bool CoreOpenGL::windowResize(int windowX, int windowY, int windowWidth, int windowHeight)
{
	glViewport(0, 0, (GLsizei) windowWidth, (GLsizei) windowHeight);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	gluPerspective(45.0f, (GLfloat) windowWidth / (GLfloat) windowHeight, 0.1f, 100.0f);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	return true;
}

void CoreOpenGL::initializeOpenGL()
{
	GLfloat ambient[] = {0.0, 0.0, 0.0, 1.0};
	GLfloat diffuse[] = {1.0, 1.0, 1.0, 1.0};
	GLfloat specular[] = {1.0, 1.0, 1.0, 1.0};
	GLfloat position[] = {0.0, 3.0, 3.0, 0.0};

	GLfloat lmodel_ambient[] = {0.2, 0.2, 0.2, 1.0};
	GLfloat local_view[] = {0.0};

	glLightfv(GL_LIGHT0, GL_AMBIENT, ambient);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse);
	glLightfv(GL_LIGHT0, GL_POSITION, position);
	glLightModelfv(GL_LIGHT_MODEL_AMBIENT, lmodel_ambient);
	glLightModelfv(GL_LIGHT_MODEL_LOCAL_VIEWER, local_view);

	glFrontFace(GL_CW);

	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
   
	glShadeModel(GL_SMOOTH);
	glClearColor(0.0f, 0.0f, 0.0f, 0.5f);
	glClearDepth(1.0f);
	glEnable(GL_AUTO_NORMAL);
	glEnable(GL_NORMALIZE);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
}

std::string CoreOpenGL::handleAction(std::string argument)
{
	std::string retstr = "CORE_DONE";
	std::string lcommand, rcommand;

	this->splitActionCommand(argument, lcommand, rcommand);
	this->sendNotifyMessage("handleAction -> Command Found");

	switch(this->getCommandActionIndex(lcommand))
	{
	case UNKNOWN_ACTION:
	default: //UNKNOWN_ACTION
		retstr = "UNKNOWN_ACTION";
		break;
	}
	
	return retstr;
}