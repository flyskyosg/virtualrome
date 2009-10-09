
#include <CoreJSCall.h>

#include <osgDB/ReadFile>
#include <osgUtil/Optimizer>

#include <osgViewer/ViewerEventHandlers>

using namespace CommonCore;


//CoreJSCall Costruttore
CoreJSCall::CoreJSCall(std::string corename) : CoreBase(corename)
{
	this->sendNotifyMessage("CoreJSCall -> Costructing CoreJSCall Instance.");
}

//CoreJSCall Distruttore
CoreJSCall::~CoreJSCall()
{
	this->sendNotifyMessage("~CoreJSCall -> Destructing CoreJSCall Instance.");

	if(_ParserHandler.valid())
		_ParserHandler = NULL;

	if(_GetBackHandler.valid())
		_GetBackHandler = NULL;
}

//Ridefinizione dell'albero di scena
bool CoreJSCall::initSceneData()
{
	osg::ref_ptr<osg::Group> parent = new osg::Group;
    parent->addChild(_LocalSceneGraph.get());

	/***************************************************************
	 *
	 * CREO IL GETBACKHANDLER
	 */

	//Creo il GetBack Handler
	_GetBackHandler = new SceneHandlers::GetBackHandler((CommonCore::RaiseEventInterface*) this);
	//Aggiunge la description di comando per usare il getback handler
	_GetBackHandler->setJSCallToNode(parent.get(), "Left Mouse Picking GetBack Handled!");
	
	/***************************************************************
	 *
	 * CREO IL TOOLIPS HANDLER
	 */

	//Creo il Parser Handler
	_ParserHandler = new SceneHandlers::NodeParserHandler;
	
		
	/***************************************************************
	 *
	 * ATTACCO IL GETBACK AL PARSER
	 */
	
	_ParserHandler->addCommand( osgGA::GUIEventAdapter::PUSH, _GetBackHandler.get());
	_Viewer->addEventHandler(_ParserHandler.get());

	//Scene Data
	_Viewer->setSceneData(parent.get());

	return true;
}
 