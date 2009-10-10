#ifndef __OSG4WEB_COREBASE__
#define __OSG4WEB_COREBASE__ 1


#include <CoreBase/Defines.h>

#include <CommonCore/CommandSchedule.h>
#include <CommonCore/CoreInterface.h>

#include <osg/Group>
#include <osg/GraphicsContext>

#include <osgGA/TrackballManipulator>
#include <osgGA/KeySwitchMatrixManipulator>

#include <osgViewer/Viewer>
#include <osgViewer/ViewerEventHandlers>

#if defined(WIN32)
#include <osgViewer/api/win32/GraphicsWindowWin32>
#else
//TODO: linux
#endif



using namespace OSG4WebCC;

/***********************************************************************
 *
 * OSG CoreBase Class
 *
 ***********************************************************************/
class CoreBase : public CoreInterface, public CommandSchedule
{
public:
	//Costruttore/Distruttore
	CoreBase(std::string corename = OSG4WEB_COREBASE_NAME);
	~CoreBase();
	
	//Return Class Name
	virtual const char* className() { return OSG4WEB_COREBASE_LIBNAME; };
	//Return Class Version
	virtual const char* classVersion() { return OSG4WEB_COREBASE_VERSION; };

	//Core initialization
#if defined(WIN32)
	bool InitCore(WINDOWIDTYPE, std::string, std::string);
#else
	bool InitCore(Display*, WINDOWIDTYPE, std::string, std::string);
#endif

	//Inizializza le Opzioni del core successivamente all'inizializzazione
	virtual void AddStartOptions(std::string str, bool erase = true);

	//Gestore dei Comandi pubblicati nel RegistryComandi
	virtual std::string handleAction(std::string action, std::string argument);
	
	//Gestisce i comandi dalla Shell e Javascript
	std::string DoCommand(std::string command);			//FIXME: togliere virtual
	
	//Rendering pass
	bool RenderScene();
	
	//Setta a True (fine) osgViewer
	void setDone();

	//Chiede ad osgViewer lo stato (True se il rendering è concluso)
	bool isDone();

protected:
	//Inizializzazione della scena
	virtual bool initManipulators(void);
	virtual bool initCameraConfig(void);
	virtual bool initSceneData(void);
	virtual bool initSetupOptions(std::string options, bool erase = true);
	virtual bool setupWindow(osg::Referenced* wd);
	
	//Carica un modello nella scena
	virtual bool loadModel(std::string nodename, bool erase = true);

	//Funzioni per la gestione dell'Update di scena
	virtual void preFrameUpdate(void) {};
	virtual void postFrameUpdate(void) {};

	void clearCommandRegistry();
	void addCommandSchedule(CommandSchedule* cschedule);
	
protected:
	osg::ref_ptr<osg::Camera> _MainCamera;
	osg::ref_ptr<osgViewer::Viewer> _Viewer;
	osg::ref_ptr<osg::GraphicsContext::Traits> _Traits;

	//SceneGraph
	osg::ref_ptr<osg::Group> _LocalSceneGraph;

	//Strutture per i manipolatori
	osg::ref_ptr<osgGA::TrackballManipulator> _TrackballManipulator;
	osg::ref_ptr<osgGA::KeySwitchMatrixManipulator> _KeySwitchManipulator;

	//Dimensioni della Window
	unsigned int _WinRect[4];

private:
	//Registry dei Comandi 
	std::vector< CommandSchedule* > _CommandRegistry;

	//Strutture di inizializzazione scena
	osg::ref_ptr<osg::Referenced> _WinData;

	//Directory di Installazione del Core
	std::string _InstDir;
};


#endif //__OSG4WEB_COREBASE__




