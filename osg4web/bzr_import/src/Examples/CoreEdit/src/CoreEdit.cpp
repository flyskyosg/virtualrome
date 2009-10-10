
#include <CoreEdit.h>

#include <osg/Material>
#include <osg/PolygonOffset>
#include <osg/PolygonMode>

#include <osgDB/ReadFile>

#include <osgUtil/Optimizer>

using namespace OSG4WebCC;

//CoreEdit Costruttore
CoreEdit::CoreEdit(std::string corename) : CoreBase(corename),
	_mainterrain(NULL),
	_picker(new SceneHandlers::PickEditHandler(NULL))
{
	this->sendNotifyMessage("CoreEdit -> Costructing CoreEdit Instance.");
}

//CoreEdit Distruttore
CoreEdit::~CoreEdit()
{
	if(_mainterrain.valid())
		_mainterrain = NULL;

	if(_picker.valid())
		_picker = NULL;

	this->sendNotifyMessage("~CoreEdit -> Destructing CoreEdit Instance.");
}

// Ridefinisco i comandi accettati dalla DoCommand per il CommandSchedule "this"
void CoreEdit::AddStartOptions(std::string str, bool erase)
{
	this->sendNotifyMessage("AddStartOptions -> Adding Starting Options.");

	//Pulisco il registry dei comandi per togliere commandbase
	this->clearCommandRegistry();
	this->clearCommandActions();

	this->setCommandScheduleName( "COREEDIT" );

	//Using: CEM_LOAD_TERRAIN fileaddress
	this->setCommandAction("LOAD_TERRAIN");
	
	this->addCommandSchedule((CommandSchedule*) this);
	this->addCommandSchedule((CommandSchedule*) _picker.get());
	
	this->DoCommand(str); //Passo la stringa di loading del terreno a DoCommand
}

//Ridefinizione dell'albero di scena
bool CoreEdit::initSceneData()
{
    osg::ref_ptr<osg::Group> root = new osg::Group;
	root->setName("root");
	
	osg::ref_ptr<osg::Group> terreni = new osg::Group;
	terreni->setName("terreni");

	_mainterrain = new osg::Group;
	_mainterrain->setName("Main Terrain Node");
	terreni->addChild( _mainterrain.get() );

	osg::ref_ptr<osg::Geode> geode = new osg::Geode;
	osg::ref_ptr<osg::Geometry> geombox = new osg::Geometry;
	osg::ref_ptr<osg::Vec3Array> vertices = new osg::Vec3Array;
	osg::ref_ptr<osg::Vec3Array> normals = new osg::Vec3Array;
	osg::ref_ptr<osg::Vec4Array> colors = new osg::Vec4Array;

	vertices->push_back(osg::Vec3(- 100.0f, - 100.0f, -10.0f));
	vertices->push_back(osg::Vec3(- 100.0f, 100.0f, -10.0f));
	vertices->push_back(osg::Vec3( 100.0f, 100.0f, -10.0f));
	vertices->push_back(osg::Vec3( 100.0f, - 100.0f, -10.0f));
	geombox->setVertexArray(vertices.get()); 

	//Associo Normali
	normals->push_back(osg::Vec3(0.0f,0.0f,1.0f));
	geombox->setNormalArray(normals.get());
	geombox->setNormalBinding(osg::Geometry::BIND_OVERALL);

	//Associo Colore
	colors->push_back(osg::Vec4(1.0f,1.0,0.8f,0.2f)); 
	geombox->setColorArray(colors.get());
	geombox->setColorBinding(osg::Geometry::BIND_OVERALL);

	//Definisco la primitiva per la geometria
	geombox->addPrimitiveSet(new osg::DrawArrays(GL_QUADS,0,4));
	geode->addDrawable(geombox.get());

	_mainterrain->addChild( geode.get() );

	osg::ref_ptr<osg::Group> added_models = new osg::Group;
	added_models->dirtyBound();
	added_models->setName("added_models");

	root->addChild( terreni.get() );
	terreni->addChild( added_models.get() );
	root->addChild( added_models.get() );
	
	//osg::ref_ptr<osg::Group> hud = createHUD();
	//root->addChild( hud.get() );
	
	//Setto il _picker
	_picker->setAddedModelGroup(added_models.get());
	//_picker->storeHUDPointer( hud.get() );	
	//_picker->defaultText();

	// add the handler for doing the picking
    _Viewer->addEventHandler( _picker.get() );

	_Viewer->setSceneData(root.get());

	return true;
}



// Ridefinisco la funzione che gestisce i comandi
std::string CoreEdit::handleAction(std::string argument)
{
	std::string retstr = "CORE_DONE";

	//this->sendNotifyMessage("handleAction -> Command Found");

	switch(this->getCommandActionIndex(argument))
	{
	case 0: //LOAD_TERRAIN
		{
			osg::ref_ptr<osg::Node> terreno = osgDB::readNodeFile( argument ); 
	
			if( terreno.valid() )
			{
				//Pulisco i nodi precedenti
				_mainterrain->removeChildren( 0, _mainterrain->getNumChildren() );

				//FIXME: aggiungere Optimizer

				_mainterrain->addChild( terreno.get() );

				_Viewer->home();
			}
		}
		break;
	default:
		break;
	}

	return retstr;
}

//Gestione eventi di pre rendering del pickEditHandler
void CoreEdit::preFrameUpdate()
{
	_picker->handleSceneGraph();
}


//Creating HUD 
osg::Group* CoreEdit::createHUD()
{
	osg::ref_ptr<osg::Geode> geode = new osg::Geode();
    
    //std::string timesFont("fonts/arial.ttf");

    osg::StateSet* stateset = geode->getOrCreateStateSet();
    stateset->setMode(GL_LIGHTING,osg::StateAttribute::OFF);

	osg::ref_ptr<osgText::Text> textMode = new osgText::Text;
	osg::ref_ptr<osgText::Text> textDragger = new  osgText::Text;
	osg::ref_ptr<osgText::Text> textModel = new  osgText::Text;
	osg::ref_ptr<osgText::Text> textInstructions = new  osgText::Text;
    
	geode->addDrawable( textMode.get() );
	geode->addDrawable( textDragger.get() );
	geode->addDrawable( textModel.get() );
	geode->addDrawable( textInstructions.get() );

    osg::Vec3 position1(50.0f,90.0f,0.0f);
    osg::Vec3 position2(50.0f,70.0f,0.0f);
	osg::Vec3 position3(50.0f,50.0f,0.0f);
	osg::Vec3 position4(50.0f,20.0f,0.0f);
		
	textMode->setPosition(position1);
	textMode->setCharacterSize(14,1);
    //textMode->setText("Riga 1.");

	textDragger->setPosition(position2);
	textDragger->setCharacterSize(14,1);
    //textDragger->setText("Riga 2.");

	textModel->setPosition(position3);
	textModel->setCharacterSize(14,1);
    //textModel->setText("Riga 3.");

	textInstructions->setPosition(position4);
	textInstructions->setCharacterSize(14,1);
	textInstructions->setText("TAB cambia modalita', ALT-SX cambia modello, CTRL-SX cambia dragger, SHIFT-SX conferma modifica");

	//text->setFont(timesFont);


	//disegno uno sfondo semitrasparente
	osg::ref_ptr<osg::Geometry> geom = new osg::Geometry;

	osg::ref_ptr<osg::Vec3Array> vertices = new osg::Vec3Array;
	vertices->push_back(osg::Vec3(20.0f,110.0f,0.0f));
	vertices->push_back(osg::Vec3(20.0f,10.0f,0.0f));
	vertices->push_back(osg::Vec3(1000.0f,10.0f,0.0f));
	vertices->push_back(osg::Vec3(1000.0f,110.0f,0.0f));

	geom->setVertexArray( vertices.get() );

	osg::ref_ptr<osg::Vec3Array> normals = new osg::Vec3Array;
	normals->push_back(osg::Vec3(0.0f,0.0f,1.0f));
	geom->setNormalArray( normals.get() );
	geom->setNormalBinding(osg::Geometry::BIND_OVERALL);

	osg::ref_ptr<osg::Vec4Array> colors = new osg::Vec4Array;
	colors->push_back(osg::Vec4(1.0f,1.0f,0.8f,0.1f));
	geom->setColorArray( colors.get() );
	geom->setColorBinding(osg::Geometry::BIND_OVERALL);

	geom->addPrimitiveSet(new osg::DrawArrays(GL_QUADS,0,4));

	osg::StateSet* stateset2 = geom->getOrCreateStateSet();
	stateset2->setMode(GL_BLEND, osg::StateAttribute::ON);

	stateset2->setRenderingHint(osg::StateSet::TRANSPARENT_BIN);

	geode->addDrawable( geom.get() );


	osg::ref_ptr<osg::Camera> camera = new osg::Camera;

    // set the projection matrix
    camera->setProjectionMatrix(osg::Matrix::ortho2D(0,1280,0,1024));

    // set the view matrix    
    camera->setReferenceFrame(osg::Transform::ABSOLUTE_RF);
    camera->setViewMatrix(osg::Matrix::identity());

    // only clear the depth buffer
    camera->setClearMask(GL_DEPTH_BUFFER_BIT);

    // draw subgraph after main camera view.
    camera->setRenderOrder(osg::Camera::POST_RENDER);

    camera->addChild(geode.get());
    
    return camera.release();
}
