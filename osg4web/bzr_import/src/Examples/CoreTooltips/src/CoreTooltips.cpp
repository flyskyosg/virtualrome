
#include <CoreTooltips.h>

#include <osg/Geode>
#include <osg/ShapeDrawable>
#include <osg/BlendFunc>

#include <osgDB/ReadFile>

#include <osgUtil/Optimizer>

#include <osgViewer/ViewerEventHandlers>

using namespace CommonCore;


/********************************************************************
 *
 *  Metodi Classe Core Esempio Tooltips
 *
 ********************************************************************/


/** CoreTooltips Costruttore */

CoreTooltips::CoreTooltips(std::string corename) : CoreBase(corename),
	_TooltipsParserHandler(NULL),
	_TooltipsSceneModifier(NULL)
{
	this->sendNotifyMessage("CoreTooltips -> Costructing CoreTooltips Instance."); //Log di Notify
}


/** CoreTooltips Distruttore */

CoreTooltips::~CoreTooltips()
{
	this->sendNotifyMessage("~CoreTooltips -> Destructing CoreTooltips Instance."); //Log di Notify

	if(_TooltipsParserHandler.valid())
		_TooltipsParserHandler = NULL;

	if(_TooltipsSceneModifier.valid())
		_TooltipsSceneModifier = NULL;
}


/** Ridefinisco l'albero di scene e attacco l'handler di Tooltip */

bool CoreTooltips::initSceneData()
{
	this->sendNotifyMessage("initSceneData -> Building the SceneGraph.");

	/***************************************************************
	 *
	 * CREO IL TOOLTIPSCENEMODIFIER
	 */

	//Creo il Tooltip Scene Modifier
	_TooltipsSceneModifier = new SceneHandlers::TooltipsSceneModifier();

	// add the HUD subgraph.
	osg::ref_ptr<osg::Group> parent = new osg::Group;
	osg::ref_ptr<osg::Node> tooltipnode = _TooltipsSceneModifier->createTooltipHUD();

	parent->setName("Root_SceneGraph_Node");
	parent->addChild(_LocalSceneGraph.get());
	parent->addChild(tooltipnode.get());

	//Creo i modelli geometrici della scena
	_LocalSceneGraph->setName("SubRoot_Models_SceneGraph");
	_LocalSceneGraph->addChild( (osg::Group*) createScene() );

    /***************************************************************
	 *
	 * CREO IL TOOLIPS HANDLER
	 */

	//Creo il Tooltip Handler
	_TooltipsParserHandler = new SceneHandlers::NodeParserHandler;
	//Setto la maschera di attraversamento dei Tooltips
	_TooltipsParserHandler->setTraversalNodeMask( _TooltipsSceneModifier->getAllowNodeMask() );

		
	/***************************************************************
	 *
	 * ATTACCO IL TOOLTIP ALLA SCENA
	 */
	
	_TooltipsParserHandler->addCommand( osgGA::GUIEventAdapter::FRAME, _TooltipsSceneModifier.get()); //FIXME: controllare se usare MOUSEMOVE
	_Viewer->addEventHandler(_TooltipsParserHandler.get());
	_Viewer->setSceneData(parent.get());

	//Azzero alla posizione iniziale
	_Viewer->home();

	return true;
}

/** Crea i modelli geometrici e aggiunge i comandi per i tooltip*/

osg::Node* CoreTooltips::createScene()
{
	osg::ref_ptr<osg::Group> grp = new osg::Group;

	osg::ref_ptr<osg::Geode> geodesphere = new osg::Geode();
	osg::ref_ptr<osg::Geode> geodebox = new osg::Geode();
	osg::ref_ptr<osg::Geode> geodecone = new osg::Geode();
	osg::ref_ptr<osg::Geode> geodecyl = new osg::Geode();
	osg::ref_ptr<osg::Geode> geodecapsule = new osg::Geode();

	osg::ref_ptr<osg::StateSet> stateset = new osg::StateSet();

	geodesphere->setStateSet( stateset.get() );
	geodebox->setStateSet( stateset.get() );
	geodecone->setStateSet( stateset.get() );
	geodecyl->setStateSet( stateset.get() );
	geodecapsule->setStateSet( stateset.get() );
    
	float radius = 0.8f;
	float height = 1.0f;
    
    osg::TessellationHints* hints = new osg::TessellationHints;
    hints->setDetailRatio(0.5f);

	//Creo i modelli da attaccare alla scena
	osg::ref_ptr<osg::ShapeDrawable> shpdrwsphere = new osg::ShapeDrawable( new osg::Sphere( osg::Vec3(0.0f,0.0f,0.0f), radius), hints);
	osg::ref_ptr<osg::ShapeDrawable> shpdrwbox = new osg::ShapeDrawable( new osg::Box( osg::Vec3(2.0f,0.0f,0.0f), 2*radius), hints);
	osg::ref_ptr<osg::ShapeDrawable> shpdrwcone = new osg::ShapeDrawable( new osg::Cone( osg::Vec3(4.0f,0.0f,0.0f), radius, height), hints);
	osg::ref_ptr<osg::ShapeDrawable> shpdrwcyl = new osg::ShapeDrawable( new osg::Cylinder( osg::Vec3(6.0f,0.0f,0.0f), radius, height), hints);
	osg::ref_ptr<osg::ShapeDrawable> shpdrwcapsule = new osg::ShapeDrawable( new osg::Cylinder( osg::Vec3(8.5f,0.0f,0.0f), radius * 1.5, height * 1.5), hints); //new osg::Capsule( osg::Vec3(8.0f,0.0f,0.0f), radius, height), hints);

	shpdrwsphere->setColor(osg::Vec4(1.0f,0.12f,0.06f,1.0f));
	shpdrwbox->setColor(osg::Vec4(1.0f,0.12f * 2,0.06f,1.0f));
	shpdrwcone->setColor(osg::Vec4(1.0f,0.12f * 3,0.06f,1.0f));
	shpdrwcyl->setColor(osg::Vec4(1.0f,0.12f * 4,0.06f,1.0f));
	shpdrwcapsule->setColor(osg::Vec4(1.0f,0.12f * 5,0.06f,1.0f));
    
	geodesphere->setName("Geode_Sphere");
    geodesphere->addDrawable( shpdrwsphere.get() );
	geodebox->setName("Geode_Box");
    geodebox->addDrawable( shpdrwbox.get() );
	geodecone->setName("Geode_Cone");
    geodecone->addDrawable( shpdrwcone.get() );
	geodecyl->setName("Geode_Cylinder");
    geodecyl->addDrawable( shpdrwcyl.get() );
	geodecapsule->setName("Geode_Big_Cylinder");
    geodecapsule->addDrawable( shpdrwcapsule.get() );

	grp->setName("Models_Group");

	/***********************************************************************************
	 * Inizializzo i nodi per essere utilizzati dal Tooltip Handler
	 * - Utilizzo il TooltipsHandler per creare i nodi necessari
	 * - Ritorna un nuovo nodo da attaccare alla scena
	 */
	
	grp->addChild( _TooltipsSceneModifier->setTooltipsProperties(geodesphere.get(), "I'm a Sphere") );
	grp->addChild( _TooltipsSceneModifier->setTooltipsProperties(geodebox.get(), "I'm a Box") );
	grp->addChild( _TooltipsSceneModifier->setTooltipsProperties(geodecone.get(), "I'm a Cone") );
	grp->addChild( _TooltipsSceneModifier->setTooltipsProperties(geodecyl.get(), "I'm a Cylinder") );
	grp->addChild( _TooltipsSceneModifier->setTooltipsProperties(geodecapsule.get(), "I'm a big Cylinder") );
 
	return grp.release();
}

void CoreTooltips::preFrameUpdate()
{
	if(_TooltipsSceneModifier.valid())
		_TooltipsSceneModifier->checkTooltipsTiming();
}
 