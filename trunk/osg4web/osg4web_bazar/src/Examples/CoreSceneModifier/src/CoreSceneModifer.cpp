
#include <CoreSceneModifier.h>


#include <osg/Material>
#include <osg/PolygonOffset>
#include <osg/PolygonMode>

#include <osgDB/ReadFile>

#include <osgUtil/Optimizer>

using namespace CommonCore;

//CoreSceneModifier Costruttore
CoreSceneModifier::CoreSceneModifier(std::string corename) : CoreBase(corename),
	_SceneModifier(new SceneHandlers::SceneModifier),
	_MainNode(new osg::Group)
{
	this->sendNotifyMessage("CoreSceneModifier -> Costructing CoreExampleSceneModifier Instance.");
}

//CoreSceneModifier Distruttore
CoreSceneModifier::~CoreSceneModifier()
{
	this->sendNotifyMessage("~CoreSceneModifier -> Destructing CoreExampleSceneModifier Instance.");
	if(_AnimateHandler.valid())
		_AnimateHandler = NULL;
	if(_SceneModifier.valid())
		_SceneModifier = NULL;
	if(_ParserHandler.valid())
		_ParserHandler = NULL;

	if(_GetBackHandler.valid())
		_GetBackHandler = NULL;

}

//Carica un modello nella scena e aggiunge lo scribe sul modello. 
bool CoreSceneModifier::loadModel(std::string nodename, bool erase) //Erase non usato
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

		_MainNode->removeChildren(0, _MainNode->getNumChildren()); //forzo la rimozione dei nodi caricati
		_LocalSceneGraph->removeChildren(0, _LocalSceneGraph->getNumChildren()); //forzo la rimozione dei nodi caricati

		//Ricostruisco la scena
		this->buildMainScene();
    
		lnode->setName(nodename);
		_LocalSceneGraph->addChild(lnode.get());

		_Viewer->home();

		ret = true;
	}
	else
	{
		this->sendWarnMessage("loadModel -> Loading Model Failed. Model Name: " + nodename);
		ret = false; 
	}

	return ret;
}


//Ridefinizione dell'albero di scena
bool CoreSceneModifier::initSceneData()
{
	this->sendNotifyMessage("initSceneData -> Building the SceneGraph.");

	_MainNode->setName("MainNode");

	this->buildMainScene();

	_SceneModifier->setSceneData(_MainNode.get());

	//Attacca al Command Registry lo SceneModifier CommandSchedule
	this->addCommandSchedule((CommandSchedule*) _SceneModifier.get());

	
	_AnimateHandler = new SceneHandlers::AnimateViewHandler(_Viewer.get());

	_Viewer->addEventHandler(_AnimateHandler.get());

	this->addCommandSchedule((CommandSchedule*) _AnimateHandler.get());

	/***************************************************************
	 *
	 * CREO IL GETBACKHANDLER
	 */

	//Creo il GetBack Handler
	_GetBackHandler = new SceneHandlers::GetBackHandler((CommonCore::RaiseEventInterface*) this);
	//Aggiunge la description di comando per usare il getback handler
	_GetBackHandler->setJSCallToNode(_MainNode.get(), "Left Mouse Picking GetBack Handled!");
	
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

//	osg::ref_ptr<osgDB::ReaderWriter::Options> rwoptions = new osgDB::ReaderWriter::Options;
//	rwoptions->setObjectCacheHint(osgDB::ReaderWriter::Options::CACHE_ALL);
//	osgDB::Registry::instance()->setOptions(rwoptions.get());


	_Viewer->setSceneData(_MainNode.get()); 

	return true;
}

bool CoreSceneModifier::buildMainScene()
{
	osg::ref_ptr<osg::MatrixTransform> mtscale = new osg::MatrixTransform;
	osg::ref_ptr<osg::MatrixTransform> mtrotate = new osg::MatrixTransform;
	osg::ref_ptr<osg::MatrixTransform> mttranslate = new osg::MatrixTransform;
	osg::ref_ptr<osg::Switch> switchnode = new osg::Switch;

	switchnode->setName("SwitchNode");
	mtscale->setName("MTScale");
	mtrotate->setName("MTRotate");
	mttranslate->setName("MTTranslate");

	mtscale->setMatrix(osg::Matrix::scale(osg::Vec3(10.0, 10.0, 10.0)));
	mtrotate->setMatrix(osg::Matrix::rotate(osg::inDegrees(45.0), osg::Vec3(0.0, 1.0, 00)));
	mttranslate->setMatrix(osg::Matrix::translate(osg::Vec3(0.0, 0.0, 10.0)));

	_LocalSceneGraph->setName("LoadedNode");

	_MainNode->addChild(_LocalSceneGraph.get());
	_MainNode->addChild(switchnode.get());
	_MainNode->addChild(mtscale.get());
	_MainNode->addChild(mtrotate.get());
	_MainNode->addChild(mttranslate.get());

	return true;
}