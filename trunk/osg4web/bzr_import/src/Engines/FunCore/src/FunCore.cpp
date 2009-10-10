
#include <FunCore/FunCore.h>


#include <osg/MatrixTransform>

#include <osgDB/ReadFile>

#include <osgUtil/Optimizer>

using namespace OSG4WebCC;


//FunCore Costruttore
FunCore::FunCore(std::string corename) : CoreBase(corename),
	_SceneModifier( new SceneHandlers::SceneModifier ),
	_WalkManip( new Manipulators::walkManipulator ),
	_AnimateHandler( NULL ),
	_MainNode( new osg::Group )
{
	this->sendNotifyMessage("FunCore -> Costructing FunCore Instance.");
}

//FunCore Distruttore
FunCore::~FunCore()
{
	this->sendNotifyMessage("~FunCore -> Destructing FunCore Instance.");
}

//Setta le opzioni del core successivamente alla inizializzazione
void FunCore::AddStartOptions(std::string str, bool erase)
{
	this->sendNotifyMessage("AddStartOptions -> Adding Starting Options.");
/*
	this->setCommandAction("LOAD_MODEL");
	this->setCommandAction("STATUSBAR_VALUE");
	this->setCommandAction("STATUSBAR_COLOR");
	this->setCommandAction("STATUSBAR_VISIBILITY");
	this->setCommandAction("SETMESSAGE");
	this->setCommandAction("SETMESSAGE_COLOR");

	this->addCommandSchedule((CommandSchedule*) this);
*/
}

//Ridefinizione dell'albero di scena
bool FunCore::initSceneData()
{
	_AnimateHandler = new SceneHandlers::AnimateViewHandler(_Viewer.get());

	_MainNode->setName("MainNode");

	this->buildMainScene();

	this->addCommandSchedule((CommandSchedule*) _SceneModifier.get());
	this->addCommandSchedule((CommandSchedule*) _AnimateHandler.get());

	_Viewer->addEventHandler(_AnimateHandler.get());
	
	_SceneModifier->setSceneData(_MainNode.get());
	_Viewer->setSceneData(_LocalSceneGraph.get());

	return true;
}


bool FunCore::initManipulators()
{
	this->sendNotifyMessage("initManipulators -> Initializing Manipulators.");

	_WalkManip->setNode(_LocalSceneGraph.get());
	_WalkManip->setViewer( _Viewer.get(), "", "" );
	
	//Aggiungere dopo l'inizializzazione
	_WalkManip->searchDefaultPos();

	//Attacca al Command Registry il Manipolator CommandSchedule
	this->addCommandSchedule((CommandSchedule*) _WalkManip.get());

	_KeySwitchManipulator->addMatrixManipulator( '1', "WalkManipulator", _WalkManip.get());
    _KeySwitchManipulator->selectMatrixManipulator(0);
  
	return true;
}


bool FunCore::buildMainScene()
{
	_LocalSceneGraph->setName("LoadedNode");
	_MainNode->addChild(_LocalSceneGraph.get());


	//TODO:

	return true;
}