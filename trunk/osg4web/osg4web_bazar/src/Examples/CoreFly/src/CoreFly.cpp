
#include <CoreFly.h>


#include <osgDB/ReadFile>


using namespace CommonCore;


//CoreFly Costruttore
CoreFly::CoreFly(std::string corename) : CoreBase(corename),
	_FlyManip( new Manipulators::FlyManipulator() )
{
	this->sendNotifyMessage("CoreFly -> Costructing CoreExamplePick Instance.");
}

//CoreFly Distruttore
CoreFly::~CoreFly()
{
	this->sendNotifyMessage("~CoreFly -> Destructing CoreExamplePick Instance.");
}


bool CoreFly::initManipulators()
{
	this->sendNotifyMessage("initManipulators -> Initializing Manipulators.");

	_FlyManip->setNode( _LocalSceneGraph.get() );

	//Attacca al Command Registry il Manipolator CommandSchedule
	//this->addCommandSchedule((CommandSchedule*) _FlyManip.get());

	_KeySwitchManipulator->addMatrixManipulator( '1', "FlyManipulator", _FlyManip.get());
    _KeySwitchManipulator->selectMatrixManipulator(0);
  
	return true;
}


 