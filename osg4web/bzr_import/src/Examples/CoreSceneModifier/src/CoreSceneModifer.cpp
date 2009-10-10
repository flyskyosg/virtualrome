
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