#ifndef __OSG4WEB_COREOPENGL__
#define __OSG4WEB_COREOPENGL__ 1


#include <CoreOpenGL/Defines.h>

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
#else
	bool InitCore(Display*, WINDOWIDTYPE, std::string, std::string);
#endif

	//Rendering pass
	bool RenderScene();
	
	//Setta a True (fine)
	void setDone() { _done = true; }

	bool isDone() { return _done; }

protected:
	std::string getInstallationDirectory() { return _InstDir; }
	
protected:

	//Larghezza corrente della Window
	int _CurrWidth, _CurrHeight;

private:
	//Registry dei Comandi 
	std::map<std::string, CommandSchedule*> _CommandRegistry;

	//Directory di Installazione del Core
	std::string _InstDir;

	float _theta;

	//Window Struct
	WINDOWIDTYPE _hWnd;
	WINDOWHGLRC _hRC;
	WINDOWHDC _hDC;

	//Inizializzazione
	bool _done;
};


#endif //__OSG4WEB_COREOPENGL__




