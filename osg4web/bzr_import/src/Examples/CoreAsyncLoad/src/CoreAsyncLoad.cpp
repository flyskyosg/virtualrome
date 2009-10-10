
#include <CoreAsyncLoad.h>

#include <osgDB/ReadFile>

#include <osgUtil/Optimizer>

using namespace CommonCore;

/** CoreAsyncLoad Costruttore */
CoreAsyncLoad::CoreAsyncLoad(std::string corename) : CoreBase(corename),
	_mainLTHandler( NULL )
{
	this->sendNotifyMessage("CoreAsyncLoad -> Costructing CoreExampleAsyncLoad Instance.");

	//Attivo l'ottimizzatore per i modelli caricati
	_mainLTHandler->setOptimization(true);
}

/** CoreAsyncLoad Distruttore */
CoreAsyncLoad::~CoreAsyncLoad()
{
	//Esplicito la distruzione
	if(_mainLTHandler.valid())
	{
		//TODO: stop dei loading ...
		_mainLTHandler = NULL;
	}

	this->sendNotifyMessage("~CoreAsyncLoad -> Destructing CoreExampleAsyncLoad Instance.");
}

//Inizializzo gli Handler di Scena
bool CoreAsyncLoad::initSceneHandlers()
{
	CoreBase::initSceneHandlers();

	_mainLTHandler = new SceneHandlers::LoadThreadsHandler(_Traits->width, _Traits->height);
	
	// add the stats handler
	_Viewer->addEventHandler(_mainLTHandler.get());

	return true;
}

/** Ridefinisco i comandi accettati dalla DoCommand per il CommandSchedule "this" */
void CoreAsyncLoad::AddStartOptions(std::string str, bool erase)
{
	this->sendNotifyMessage("AddStartOptions -> Adding Starting Options.");

	this->setCommandScheduleName("ASYNCLOADER");

	//Using: CAL_ADD_MODEL fileaddress (MULTIPLE)
	this->setCommandAction("ADD_MODEL");
	//Using: CAL_ADD_MODEL_AND_CLEAN fileaddress (MULTIPLE)
	this->setCommandAction("ADD_MODEL_AND_CLEAN");
	//Using: CAL_ADD_MODEL_TO_NODE fileaddress parentname (MULTIPLE)
	this->setCommandAction("ADD_MODEL_TO_NODE");

	this->addCommandSchedule((CommandSchedule*) this);
	
	this->DoCommand(str); //Passo la stringa a DoCommand che la gestisce
}

/** Ridefinizione della funzione di Gestione Comandi per CommandSchedule "this" */
std::string CoreAsyncLoad::handleAction(std::string argument)
{
	std::string retstr = "CORE_DONE";
	std::string lcommand, rcommand;

	this->sendNotifyMessage("handleAction -> Command Found");
	this->splitActionCommand(argument, lcommand, rcommand);

	switch(this->getCommandActionIndex(lcommand))
	{
	case ADD_MODEL:
		if( !this->loadModel(rcommand, false) )
			retstr = "CORE_FAILED";
		break;
	case ADD_MODEL_AND_CLEAN: 
		if( !this->loadModel(rcommand, true) )
			retstr = "CORE_FAILED";
		break;
	case ADD_MODEL_TO_NODE:
		if( !this->loadModelToNode(rcommand) )
			retstr = "CORE_FAILED";
		break;
	default: //UNKNOWN_CORE_COMMAND
		retstr = "UNKNOWN_CORE_COMMAND";
		break;
	}
	
	return retstr;
}

/** ridefinizione dell'albero di scena per i modelli */
bool CoreAsyncLoad::initSceneData()
{
	this->sendNotifyMessage("initSceneData -> Building the SceneGraph.");

	osg::ref_ptr<osg::Group> mainnode = new osg::Group;
	
	mainnode->setName("Main Node");

	//Attach Loading Icon
	mainnode->addChild(_mainLTHandler->createLoadingHUD());

	//Attach models Group
	mainnode->addChild(_LocalSceneGraph.get());
	
	//Passo il nodo di scena al quale è possibile aggiungere figli
	_mainLTHandler->setNode(_LocalSceneGraph.get());

	_Viewer->setSceneData(mainnode.get());
	
	return true;
}


/** Carica un modello nella scena sotto il nodo modello Default, erase = true cancella la scena modelli */
bool CoreAsyncLoad::loadModel(std::string nodename, bool erase)
{
	if(erase)
		_LocalSceneGraph->removeChildren(0, _LocalSceneGraph->getNumChildren()); //Clear di tutta la parte di scenegraph modelli
	
	//TODO: gestione delle Options di Loading
	_mainLTHandler->requestLoading(nodename); //FIXME: ritorna false se il nodo è già in coda di caricamento

	return true;
}

/** Carica un modello nella scena e lo attacco al nodo*/
bool CoreAsyncLoad::loadModelToNode(std::string arguments)
{
	std::string nodename, newargs;
	this->splitActionCommand(arguments, nodename, newargs);
	
	//TODO: gestione delle Options di Loading
	_mainLTHandler->requestLoading(nodename, newargs); //FIXME: ritorna false se il nodo è già in coda di caricamento

	return true;
}

/** Gestisco il LoaderThreadHandler */
void CoreAsyncLoad::preFrameUpdate()
{
	this->handleLoadingThreads();
}

/** Gestisce i nodi caricati dai vari Thread di Loading */
void CoreAsyncLoad::handleLoadingThreads()
{
	//Handle dei loading
	osg::ref_ptr<osg::Node> lnode = _mainLTHandler->handleLoading();

	//Se nodo valido lo attacco direttamente
	if( lnode.valid() )
	{
		_LocalSceneGraph->addChild(lnode.get());
		_Viewer->home();
	}

	//Controllo errori di caricamento
	std::vector<std::string> failedln;
	if(_mainLTHandler->handleLoadingErrors(failedln))
	{
		for(unsigned int i = 0; i < failedln.size(); i++)
			this->raiseCommand("ALERT Error loading file: " + failedln.at(i));
	}
}