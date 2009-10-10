
#include <FunCore/FunCore.h>


#include <osg/MatrixTransform>

#include <osgDB/ReadFile>

#include <osgUtil/Optimizer>

using namespace OSG4WebCC;


//FunCore Costruttore
FunCore::FunCore(std::string corename) : CoreBase(corename),
	_LoaderThreadHandler( new SceneHandlers::LoadThreadsHandler() ),
	_SceneModifier( new SceneHandlers::SceneModifier ),
	_TooltipHandler( NULL ),
	_WalkManip( new Manipulators::walkManipulator ),
	_AnimateHandler( NULL ),
	_MainNode( new osg::Group ),
	_ModiSceneGraph( new osg::Group ),
	_SupportNode( new osg::Group ),
	_maininit(false)
{
	this->sendNotifyMessage("FunCore -> Costructing FunCore Instance.");
	
	_LoaderThreadHandler->setOptimization(true);

	this->clearCommandActions();
	this->clearCommandRegistry();

	this->setCommandScheduleName("FUNCORE");
	
	this->setCommandAction("SETMAINSCENEGRAPH");
	this->setCommandAction("ADDFILETONODE");
	this->setCommandAction("SETOPTIMIZATION");
/*
	this->setCommandAction("STATUSBAR_VALUE");
	this->setCommandAction("STATUSBAR_COLOR");
	this->setCommandAction("STATUSBAR_VISIBILITY");
	this->setCommandAction("SETMESSAGE");
	this->setCommandAction("SETMESSAGE_COLOR");
*/
	this->addCommandSchedule((CommandSchedule*) this);
}

//FunCore Distruttore
FunCore::~FunCore()
{
	if(_LoaderThreadHandler.valid())
		_LoaderThreadHandler = NULL;

	if(_WalkManip.valid())
		_WalkManip = NULL;

	if(_SceneModifier.valid())
		_SceneModifier = NULL;

	if(_AnimateHandler.valid())
		_AnimateHandler = NULL;

	if(_TooltipHandler.valid())
		_TooltipHandler = NULL;

	this->sendNotifyMessage("~FunCore -> Destructing FunCore Instance.");
}

//Setta le opzioni del core successivamente alla inizializzazione
void FunCore::AddStartOptions(std::string str, bool erase)
{
	this->sendNotifyMessage("AddStartOptions -> Adding Starting Options.");

	this->DoCommand(str); //Passo il main scene graph se è presente
}

//Ridefinizione dell'albero di scena
bool FunCore::initSceneData()
{
	_TooltipHandler = new SceneHandlers::TooltipHandler();
	_AnimateHandler = new SceneHandlers::AnimateViewHandler(_Viewer.get());

	this->buildMainScene();

	this->addCommandSchedule((CommandSchedule*) _SceneModifier.get());
	this->addCommandSchedule((CommandSchedule*) _AnimateHandler.get());

	_Viewer->addEventHandler(_AnimateHandler.get());
	_Viewer->addEventHandler(_TooltipHandler.get());
	
	_SceneModifier->setSceneData(_ModiSceneGraph.get());
	_Viewer->setSceneData(_MainNode.get());

	return true;
}


bool FunCore::initManipulators()
{
	this->sendNotifyMessage("initManipulators -> Initializing Manipulators.");

	_WalkManip->setNode(_ModiSceneGraph.get());
	_WalkManip->setViewer( _Viewer.get(), "", "" );
	
	//Attacca al Command Registry il Manipolator CommandSchedule
	this->addCommandSchedule((CommandSchedule*) _WalkManip.get());

	_KeySwitchManipulator->addMatrixManipulator( '1', "WalkManipulator", _WalkManip.get());
	//_KeySwitchManipulator->addMatrixManipulator( '1', "TrackBall", new osgGA::TrackballManipulator);
    _KeySwitchManipulator->selectMatrixManipulator(0);
  
	return true;
}


bool FunCore::buildMainScene()
{
	osg::ref_ptr<osg::Node> loadinghud = _LoaderThreadHandler->createLoadingHUD();
	osg::ref_ptr<osg::Node> tooltiphud = _TooltipHandler->createTooltipHUD();

	//Setto i nomi dei nodi
	_MainNode->setName("Super_Group_Node");
	_ModiSceneGraph->setName("Modi_Scene_Graph");

	//Attacco gli HUD 
	_MainNode->addChild(loadinghud.get());
	_MainNode->addChild(tooltiphud.get());

	//Attacco Nodi
	_MainNode->addChild(_ModiSceneGraph.get());

	//Passo il nodo di scena al quale è possibile aggiungere figli
	_LoaderThreadHandler->setNode(_ModiSceneGraph.get());
	
	//TODO:

	return true;
}

/** Ridefinizione della funzione di Gestione Comandi per CommandSchedule "this" */
std::string FunCore::handleAction(std::string argument)
{
	std::string retstr = "COMMAND_OK";
	std::string lcommand, rcommand;

	this->sendNotifyMessage("handleAction -> Command Found");
	this->splitActionCommand(argument, lcommand, rcommand);

	switch(this->getCommandActionIndex(lcommand))
	{
	case SETMAINSCENEGRAPH:
		{
			if( !this->loadModel(rcommand, false) )
				retstr = "REQUEST_IN_QUEUE";
		}
		break;
	case ADDFILETONODE:
		{
			if( !this->loadModelToNode(rcommand) )
				retstr = "REQUEST_IN_QUEUE";
		}
		break;
	case SETOPTIMIZATION:
		{
			if(rcommand == "TRUE")
				_LoaderThreadHandler->setOptimization(true);
			else if(rcommand == "FALSE")
				_LoaderThreadHandler->setOptimization(true);
			else
				retstr = "BAD_COMMAND";
		}
		break;
	default: //UNKNOWN_COMMAND
		retstr = "UNKNOWN_COMMAND";
		break;
	}
	
	return retstr;
}


bool FunCore::loadModel(std::string nodename, bool erase)
{
	if(erase)
		_ModiSceneGraph->removeChildren(0, _ModiSceneGraph->getNumChildren()); //Clear di tutta la parte di scenegraph modelli

	return _LoaderThreadHandler->requestLoading(nodename); 
}


bool FunCore::loadModelToNode(std::string arguments)
{
	std::string nodename, newargs;
	this->splitActionCommand(arguments, nodename, newargs);
	
	return _LoaderThreadHandler->requestLoading(nodename, newargs); //FIXME: ritorna false se il nodo è già in coda di caricamento
}


void FunCore::preFrameUpdate()
{
	this->handleLoadingThreads();
}


void FunCore::handleLoadingThreads()
{
	//Handle dei loading
	osg::ref_ptr<osg::Node> lnode = _LoaderThreadHandler->handleLoading();

	//Se nodo valido lo attacco direttamente
	if( lnode.valid() )
	{
		if( !_maininit ) //Nodo attaccato diretto è solo il Master di Scena
		{
			_ModiSceneGraph->addChild(lnode.get());

			//TODO: set del manipoilatore di tiziano alla posizione iniziale e parametri di movimento FLY
			_WalkManip->searchDefaultPos();

			_Viewer->home();
			_maininit = true;
		}
		else
			this->raiseCommand("ALERT Scena già inizializzata");
	}

	//Controllo errori di caricamento
	std::vector<std::string> failedln;
	if(_LoaderThreadHandler->handleLoadingErrors(failedln))
	{
		for(unsigned int i = 0; i < failedln.size(); i++)
			this->raiseCommand("ALERT Errore durante il caricamento del file: " + failedln.at(i));
	}
}