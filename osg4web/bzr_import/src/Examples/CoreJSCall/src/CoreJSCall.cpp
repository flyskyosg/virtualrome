
#include <CoreJSCall.h>

#include <osgDB/ReadFile>
#include <osgUtil/Optimizer>

#include <osgViewer/ViewerEventHandlers>

using namespace OSG4WebCC;


//CoreJSCall Costruttore
CoreJSCall::CoreJSCall(std::string corename) : CoreBase(corename)
{
	this->sendNotifyMessage("CoreJSCall -> Costructing CoreJSCall Instance.");
}

//CoreJSCall Distruttore
CoreJSCall::~CoreJSCall()
{
	this->sendNotifyMessage("~CoreJSCall -> Destructing CoreJSCall Instance.");
}

//Ridefinizione dell'albero di scena
bool CoreJSCall::initSceneData()
{
	osg::ref_ptr<osgText::Text> updateText = new osgText::Text;

    // add the HUD subgraph.
	osg::ref_ptr<osg::Group> parent = new osg::Group;

    parent->addChild(_LocalSceneGraph.get());

    // add the handler for doing the picking
    //_Viewer->addEventHandler(new PickHandler(updateText.get()));

	_Viewer->setSceneData(parent.get());

	return true;
}
 