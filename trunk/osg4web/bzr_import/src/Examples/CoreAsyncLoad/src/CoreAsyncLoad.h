#ifndef __OSG4WEB_COREASYNCLOAD__
#define __OSG4WEB_COREASYNCLOAD__ 1


#include <CoreBase/CoreBase.h>

#include <CommonCore/SceneHandlers/LoadThreadsHandler.h>
#include <Defines.h>

using namespace CommonCore;


/***********************************************************************
 *
 * CoreAsyncLoad example class
 *
 ***********************************************************************/
class CoreAsyncLoad : public CoreBase
{
public:
	enum AsyncLoadActions
	{
		UNKNOWN_ACTION = 0,
		ADD_MODEL,
		ADD_MODEL_AND_CLEAN,
		ADD_MODEL_TO_NODE
	};

	//Costruttore/Distruttore
	CoreAsyncLoad(std::string corename = OSG4WEB_COREASYNCLOAD_NAME);
	~CoreAsyncLoad();
	
	//Return Class Name
	virtual const char* className() { return OSG4WEB_COREASYNCLOAD_LIBNAME; };
	//Return Class Version
	virtual const char* classVersion() { return OSG4WEB_COREASYNCLOAD_VERSION; };

	//Inizializza le Opzioni del core successivamente all'inizializzazione
	virtual void AddStartOptions(std::string str, bool erase = true);

	//Attacco gli Handler di Scena
	virtual bool initSceneHandlers();

	//Ridefinizioni del Gestore dei Comandi
	virtual std::string handleAction(std::string argument);

protected:
	//Rinizializzo la scena
	virtual bool initSceneData(void);

	//Carica un modello nella scena
	virtual bool loadModel(std::string nodename, bool erase = true);

	//Carica un modello nella scena
	virtual bool loadModelToNode(std::string arguments);

	//Gestisco il LoaderThreadHandler
	virtual void preFrameUpdate(void);

	//Gestisco i nodi caricati dai Threads
	void handleLoadingThreads();

	//Loading Threads Handler
	osg::ref_ptr<SceneHandlers::LoadThreadsHandler> _mainLTHandler;
};

#endif //__OSG4WEB_COREASYNCLOAD__




