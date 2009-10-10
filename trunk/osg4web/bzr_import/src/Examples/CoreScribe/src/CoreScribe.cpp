
#include <CoreScribe.h>


#include <osg/Material>
#include <osg/PolygonOffset>
#include <osg/PolygonMode>

#include <osgDB/ReadFile>

#include <osgUtil/Optimizer>

using namespace CommonCore;

//CoreScribe Costruttore
CoreScribe::CoreScribe(std::string corename) : CoreBase(corename)
{
	this->sendNotifyMessage("CoreScribe -> Costructing CoreScribe Instance.");
}

//CoreScribe Distruttore
CoreScribe::~CoreScribe()
{
	this->sendNotifyMessage("~CoreScribe -> Destructing CoreScribe Instance.");
}

//Carica un modello nella scena e aggiunge lo scribe sul modello. 
bool CoreScribe::loadModel(std::string nodename, bool erase) //Erase non usato
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

		osg::ref_ptr<osg::Group> decorator = new osg::Group;
    
		_LocalSceneGraph->addChild(lnode.get());
    
		_LocalSceneGraph->addChild(decorator.get());
    
		decorator->addChild(lnode.get());

		this->sendNotifyMessage("loadModel -> Adding Scribe decorator");

		osg::ref_ptr<osg::StateSet> stateset = new osg::StateSet;
		osg::ref_ptr<osg::PolygonOffset> polyoffset = new osg::PolygonOffset;
		polyoffset->setFactor(-1.0f);
		polyoffset->setUnits(-1.0f);
		osg::ref_ptr<osg::PolygonMode> polymode = new osg::PolygonMode;
		polymode->setMode(osg::PolygonMode::FRONT_AND_BACK,osg::PolygonMode::LINE);
		stateset->setAttributeAndModes(polyoffset.get(),osg::StateAttribute::OVERRIDE|osg::StateAttribute::ON);
		stateset->setAttributeAndModes(polymode.get(),osg::StateAttribute::OVERRIDE|osg::StateAttribute::ON);

		osg::ref_ptr<osg::Material> material = new osg::Material;
		stateset->setAttributeAndModes(material.get(),osg::StateAttribute::OVERRIDE|osg::StateAttribute::ON);
		stateset->setMode(GL_LIGHTING,osg::StateAttribute::OVERRIDE|osg::StateAttribute::OFF);

		stateset->setTextureMode(0,GL_TEXTURE_2D,osg::StateAttribute::OVERRIDE|osg::StateAttribute::OFF);
    
		decorator->setStateSet(stateset.get());
		
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

 