#ifndef __OSG4WEB_COREOPENGL__
#define __OSG4WEB_COREOPENGL__ 1


#include <CoreOpenGL/Defines.h>
#include <CoreOpenGL/WindowWin32.h>

#include <CommonCore/CommandSchedule.h>
#include <CommonCore/CoreInterface.h>

#include <map>


using namespace CommonCore;

/***********************************************************************
 *
 * OSG CoreOpenGL Class
 *
 ***********************************************************************/
class CoreOpenGL : public CoreInterface
#if defined(WIN32)
	, public WindowWin32
#else
	//TODO: Linux...
#endif
{
public:

	//Costruttore/Distruttore
	CoreOpenGL(std::string corename = OSG4WEB_COREOPENGL_NAME);
	~CoreOpenGL();
	
	//Return Class Name
	virtual const char* className() { return OSG4WEB_COREOPENGL_LIBNAME; };
	//Return Class Version
	virtual const char* classVersion() { return OSG4WEB_COREOPENGL_VERSION; };

	//Core initialization
#if defined(WIN32)
	bool InitCore(WINDOWIDTYPE, std::string, std::string);
	//Handle Window Event
	virtual LRESULT handleNativeWindowWin32Event(HWND hWnd, UINT eventmsg, WPARAM wParam, LPARAM lParam);
#else
	bool InitCore(Display*, WINDOWIDTYPE, std::string, std::string);

	virtual bool handleWindowEvents(HWND hWnd, UINT eventmsg, WPARAM wParam, LPARAM lParam); //TODO:
#endif

	//Rendering pass
	bool RenderScene();
	
	//Set Rendering Stop
	void setDone() { _done = true; }
	//Check if rendering engine is currently work
	bool isDone() { return _done; }

protected:
	std::string getInstallationDirectory() { return _InstDir; }
	//OpenGL Initialization
	virtual void initializeOpenGL();
	//OpenGL Window Resize
	virtual bool windowResize(int windowX, int windowY, int windowWidth, int windowHeight);
	//OpenGL Rendering Scene
	virtual bool renderImplementation();

private:
	//Demo: Rotating Grid and Cube
	void drawDemoGridAndCube();
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




