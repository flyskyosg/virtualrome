
#include <CorePick.h>
#include <PickHandle.h>


#include <osg/Material>
#include <osg/Geode>
#include <osg/BlendFunc>
#include <osg/Depth>
#include <osg/Projection>
#include <osg/Camera>

#include <osgDB/ReadFile>

#include <osgUtil/Optimizer>

#include <osgViewer/ViewerEventHandlers>

using namespace OSG4WebCC;


//CorePick Costruttore
CorePick::CorePick(std::string corename) : CoreBase(corename)
{
	this->sendNotifyMessage("CorePick -> Costructing CoreExamplePick Instance.");
}

//CorePick Distruttore
CorePick::~CorePick()
{
	this->sendNotifyMessage("~CorePick -> Destructing CoreExamplePick Instance.");
}

//Ridefinizione dell'albero di scena
bool CorePick::initSceneData()
{
	osg::ref_ptr<osgText::Text> updateText = new osgText::Text;

    // add the HUD subgraph.
	osg::ref_ptr<osg::Group> parent = new osg::Group;

    parent->addChild(this->createHUD(updateText.get()));
	parent->addChild(_LocalSceneGraph.get());

    // add the handler for doing the picking
    _Viewer->addEventHandler(new PickHandler(updateText.get()));

	_Viewer->setSceneData(parent.get());

	return true;
}


osg::Node* CorePick::createHUD(osgText::Text* updateText)
{
	osg::ref_ptr<osg::Camera> hudCamera = new osg::Camera;
    hudCamera->setReferenceFrame(osg::Transform::ABSOLUTE_RF);
    hudCamera->setProjectionMatrixAsOrtho2D(0,800,0,600);
    hudCamera->setViewMatrix(osg::Matrix::identity());
    hudCamera->setRenderOrder(osg::Camera::POST_RENDER);
    hudCamera->setClearMask(GL_DEPTH_BUFFER_BIT);
    
    std::string timesFont("fonts/times.ttf");
    
    // turn lighting off for the text and disable depth test to ensure its always ontop.
    osg::Vec3 position(75.0f,550.0f,0.0f);
    osg::Vec3 delta(0.0f,-30.0f,0.0f);
    
    osg::ref_ptr<osg::Geode> geode = new osg::Geode();
	osg::ref_ptr<osg::StateSet> stateset = geode->getOrCreateStateSet();
    stateset->setMode(GL_LIGHTING,osg::StateAttribute::OFF);
	stateset->setMode(GL_DEPTH_TEST,osg::StateAttribute::OFF);
	geode->setName( "simple" );
	hudCamera->addChild( geode.get() );
        
	osg::ref_ptr<osgText::Text> text = new  osgText::Text;
	geode->addDrawable( text.get() );
        
	text->setFont(timesFont);
	text->setText("Picking in Head Up Displays is simple!");
	text->setPosition( position );
        
	position += delta;    
    
    for (int i=0; i<5; i++) 
	{
		osg::Vec3 dy(0.0f,-20.0f,0.0f);
		osg::Vec3 dx(70.0f,0.0f,0.0f);
		osg::ref_ptr<osg::Geode> geode = new osg::Geode();
		osg::ref_ptr<osg::StateSet> stateset = geode->getOrCreateStateSet();
		
		const char *opts[]={"One", "Two", "Three", "January", "Feb", "2003"};
		osg::ref_ptr<osg::Geometry> quad = new osg::Geometry;
		stateset->setMode( GL_LIGHTING, osg::StateAttribute::OFF );
		stateset->setMode( GL_DEPTH_TEST, osg::StateAttribute::OFF );
		std::string name = "subOption";
		name += " ";
		name += std::string(opts[i]);
        geode->setName(name);
		
		osg::ref_ptr<osg::Vec3Array> vertices = new osg::Vec3Array(4); // 1 quad
		osg::ref_ptr<osg::Vec4Array> colors = new osg::Vec4Array;
		colors = new osg::Vec4Array;
		colors->push_back(osg::Vec4(0.8-0.1*i,0.1*i,0.2*i, 1.0));
		quad->setColorArray(colors.get());
		quad->setColorBinding(osg::Geometry::BIND_PER_PRIMITIVE);

		(*vertices)[0]=position;
		(*vertices)[1]=position+dx;
		(*vertices)[2]=position+dx+dy;
		(*vertices)[3]=position+dy;

		quad->setVertexArray(vertices.get());
		quad->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::QUADS,0,4));
		geode->addDrawable(quad.get());
		hudCamera->addChild(geode.get());
        
		position += delta;
	}    

	position += delta;   

	{
		osg::ref_ptr<osg::Geode> geode = new osg::Geode();
		osg::ref_ptr<osg::StateSet> stateset = geode->getOrCreateStateSet();
		stateset->setMode(GL_LIGHTING,osg::StateAttribute::OFF);
		stateset->setMode(GL_DEPTH_TEST,osg::StateAttribute::OFF);
		geode->setName( "The text label" );
		geode->addDrawable( updateText );
		hudCamera->addChild( geode.get() );
        
		updateText->setCharacterSize(12.0f);
		updateText->setFont(timesFont);
		updateText->setColor(osg::Vec4(1.0f,1.0f,0.0f,1.0f));
		updateText->setText("");
		updateText->setPosition(position);
		updateText->setDataVariance(osg::Object::DYNAMIC);
	}

	return hudCamera.release();
}

 