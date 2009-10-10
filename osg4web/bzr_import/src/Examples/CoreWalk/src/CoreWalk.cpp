
#include <CoreWalk.h>


#include <osg/Material>
#include <osg/PolygonOffset>
#include <osg/PolygonMode>

#include <osgDB/ReadFile>

#include <osgUtil/Optimizer>



using namespace CommonCore;


//CoreWalk Costruttore
CoreWalk::CoreWalk(std::string corename) : CoreBase(corename),
	_WalkManip( new Manipulators::walkManipulator )
{
	this->sendNotifyMessage("CoreWalk -> Costructing CoreWalk Instance.");
}

//CoreWalk Distruttore
CoreWalk::~CoreWalk()
{
	this->sendNotifyMessage("~CoreWalk -> Destructing CoreWalk Instance.");
}

//Carica un modello nella scena e aggiunge lo scribe sul modello. 
bool CoreWalk::loadModel(std::string nodename, bool erase) //Erase non usato
{
	bool ret = false;

	this->sendNotifyMessage("loadModel -> Loading Model: " + nodename);

	osg::ref_ptr<osg::Node> lnode = osgDB::readNodeFile(nodename);

	if(lnode.valid())
	{
		this->sendNotifyMessage("loadModel -> Model Loaded Correctly");

		osgUtil::Optimizer optimizer;
		optimizer.optimize(lnode.get());
		optimizer.reset();

		if(erase)
			_LocalSceneGraph->removeChildren(0, _LocalSceneGraph->getNumChildren());

		_LocalSceneGraph->addChild(lnode.get());
    
		_WalkManip->searchDefaultPos();

		ret = true;
	}
	else
	{
		this->sendWarnMessage("loadModel -> Loading Model Failed. Model Name: " + nodename);
		ret = false; 
	}

	return ret;
}

bool CoreWalk::initManipulators()
{
	this->sendNotifyMessage("initManipulators -> Initializing Manipulators.");

	_WalkManip->setNode(_LocalSceneGraph.get());
	_WalkManip->setViewer( _Viewer.get(), "", "" );

	//Attacca al Command Registry il Manipolator CommandSchedule
	this->addCommandSchedule((CommandSchedule*) _WalkManip.get());

	_KeySwitchManipulator->addMatrixManipulator( '1', "WalkManipulator", _WalkManip.get());
    _KeySwitchManipulator->selectMatrixManipulator(0);
  
	return true;
}