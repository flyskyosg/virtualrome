#ifndef __OSG4WEB_COREOPENGL__
#define __OSG4WEB_COREOPENGL__ 1


#include <CoreOpenGL/Defines.h>
#include <CoreOpenGL/WindowWin32.h>

#include <CommonCore/CommandSchedule.h>
#include <CommonCore/CoreInterface.h>

#include <gl/gl.h>
#include <gl/glu.h>

#include <map>


using namespace CommonCore;

/***********************************************************************
 *
 * OSG CoreOpenGL Class
 *
 ***********************************************************************/
class CoreOpenGL : public CoreInterface , public CommandSchedule
#if defined(WIN32)
	, public WindowWin32
#else
	//TODO: Linux...
#endif
{
public:
	enum CoreOpenGLActions
	{
		UNKNOWN_ACTION = 0
	};

	//Costruttore/Distruttore
	CoreOpenGL(std::string corename = OSG4WEB_COREOPENGL_NAME);
	~CoreOpenGL();
	
	//Return Class Name
	virtual const char* className() { return OSG4WEB_COREOPENGL_LIBNAME; };
	//Return Class Version
	virtual const char* classVersion() { return OSG4WEB_COREOPENGL_VERSION; };

	//Gestore dei Comandi pubblicati nel RegistryComandi
	virtual std::string handleAction(std::string argument);

	//Core initialization
#if defined(WIN32)
	bool InitCore(WINDOWIDTYPE, std::string, std::string);
	//Handle Window Event
	virtual LRESULT handleNativeWindowWin32Event(HWND hWnd, UINT eventmsg, WPARAM wParam, LPARAM lParam);
#else
	bool InitCore(Display*, WINDOWIDTYPE, std::string, std::string);

	virtual bool handleWindowEvents(HWND hWnd, UINT eventmsg, WPARAM wParam, LPARAM lParam); //TODO:
#endif

	//Inizializza le Opzioni del core successivamente all'inizializzazione
	virtual void AddStartOptions(std::string str, bool erase = true);
	//Gestisce i comandi dalla Shell e Javascript
	std::string DoCommand(std::string command);

	//Rendering pass
	bool RenderScene();
	
	//Set Rendering Stop
	void setDone() { _done = true; }
	//Check if rendering engine is currently work
	bool isDone() { return _done; }

protected:
	//Return the installation directory
	std::string getInstallationDirectory() { return _InstDir; }
	//Clear the local Command Registtry
	void clearCommandRegistry()	{ _CommandRegistry.clear(); }
	//Add Action to the command schedule
	void addCommandSchedule(CommandSchedule* cschedule) { _CommandRegistry[cschedule->getCommandScheduleName()] = cschedule; }

	//OpenGL Initialization
	virtual void initializeOpenGL();
	//OpenGL Window Resize
	virtual bool windowResize(int windowX, int windowY, int windowWidth, int windowHeight);
	//OpenGL Rendering Scene
	virtual bool renderImplementation();

private:
	//Demo: Rotating Grid and Teapot
	void drawDemoGridAndTeapot();
	//Model Rotation
	float _theta;

private:
	//Registry dei Comandi 
	std::map<std::string, CommandSchedule*> _CommandRegistry;

	//Directory di Installazione del Core
	std::string _InstDir;

	//Inizializzazione
	bool _openglinit;
	bool _needresize;
	bool _done;
};


#endif //__OSG4WEB_COREOPENGL__




