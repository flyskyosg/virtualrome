
#include <FunCore/FunCore.h>
#include <FunCore/EnvironmentMap.h>


#include <osg/MatrixTransform>
#include <osg/Fog>
#include <osg/LightSource>
#include <osg/Geode>

#include <osgDB/FileNameUtils>
#include <osgDB/Registry>
#include <osgDB/ReadFile>

#include <osgUtil/Optimizer>

#include <Utilities/StringUtils.h>

#include <CommonCore/Visitors/FindNodeVisitor.h>

using namespace CommonCore;


//FunCore Costruttore
FunCore::FunCore(std::string corename) : CoreBase(corename),
	_LoaderThreadHandler( NULL ),
	_SceneModifier( new SceneHandlers::SceneModifier ),
	_StandardNodeParserHandler( NULL ),
	_TooltipsParserHandler( NULL ),
	_TooltipsSceneModifier( NULL ),
	//_WalkManip( new Manipulators::walkManipulator ),
	_ViRoMan( new Manipulators::ViroManipulator ),
	_AnimateHandler( NULL ),
	_GetBackHandler( NULL ),
	_MainNode( new osg::Group ),
	_ModiSceneGraph( new osg::Group ),
	_SupportNode( new osg::Group ),
	_sendCEventToJSTime( osg::Timer::instance()->tick() ),
	_jsMapActive(false),
	_envdone(false),
	_maininit(false)
{
	this->sendNotifyMessage("FunCore -> Costructing FunCore Instance.");
	
	//_LoaderThreadHandler->setOptimization(true);

	this->clearCommandActions();
	this->clearCommandRegistry();

	this->setCommandScheduleName("FUNCORE");
	
	this->setCommandAction("UNKNOWN_ACTION");
	this->setCommandAction("SETMAINSCENEGRAPH");
	this->setCommandAction("ADDFILETONODE");
	this->setCommandAction("CHECKNODEPRESENCE");
	this->setCommandAction("SETOPTIMIZATION");
	this->setCommandAction("SWITCH_MANIPULATORS");
	this->setCommandAction("RESET_SCENE");
	this->setCommandAction("ACTIVATE_JSMAP_MESSAGES");
	this->setCommandAction("DEACTIVATE_JSMAP_MESSAGES");
	this->setCommandAction("GET_JSMAP_COORDINATES");
	this->setCommandAction("SETHOMEPOSITION");
	this->setCommandAction("SETBOUNDS");

	this->addCommandSchedule((CommandSchedule*) this);
}

//FunCore Distruttore
FunCore::~FunCore()
{
	if(_LoaderThreadHandler.valid())
		_LoaderThreadHandler = NULL;

	if(_WalkManip.valid())
		_WalkManip = NULL;

	if(_ViRoMan.valid())
		_ViRoMan = NULL;

	if(_SceneModifier.valid())
		_SceneModifier = NULL;

	if(_AnimateHandler.valid())
		_AnimateHandler = NULL;

	if(_TooltipsParserHandler.valid())
		_TooltipsParserHandler = NULL;

	if(_TooltipsSceneModifier.valid())
		_TooltipsSceneModifier = NULL;

	if(_GetBackHandler.valid())
		_GetBackHandler = NULL;

	if(_StandardNodeParserHandler.valid())
		_StandardNodeParserHandler = NULL;

	// HUD
	if ( _HUD.get()->getHUD() ) _HUD->getHUD()->setUpdateCallback(0);

	this->sendNotifyMessage("~FunCore -> Destructing FunCore Instance.");
}

//Setta le opzioni del core successivamente alla inizializzazione
void FunCore::AddStartOptions(std::string str, bool erase)
{
	this->sendNotifyMessage("AddStartOptions -> Adding Starting Options.");

	this->DoCommand(str); //Passo il main scene graph se è presente
}

//Ridefinizione dell'albero di scena
bool FunCore::initSceneData()
{
	this->buildMainScene();

	_SceneModifier->setSceneData(_ModiSceneGraph.get());
	_Viewer->setSceneData(_MainNode.get());

	return true;
}

//Attacco gli Handler di Scena
bool FunCore::initSceneHandlers()
{
	CoreBase::initSceneHandlers();

	_AnimateHandler = new SceneHandlers::AnimateViewHandler(_Viewer.get());

	_LoaderThreadHandler = new SceneHandlers::LoadThreadsHandler(_Traits->width, _Traits->height);

	_StandardNodeParserHandler = new SceneHandlers::NodeParserHandler( 0x00000001 ); //TODO: fare in modo che venga da GetBackHandler

	_GetBackHandler = new SceneHandlers::GetBackHandler((CommonCore::RaiseEventInterface*) this);
	_StandardNodeParserHandler->addCommand( osgGA::GUIEventAdapter::PUSH, _GetBackHandler.get());
	_StandardNodeParserHandler->setTraversalNodeMask(0x01);
	
	_TooltipsSceneModifier = new SceneHandlers::TooltipsSceneModifier(_Traits->width, _Traits->height);
	_TooltipsParserHandler = new SceneHandlers::NodeParserHandler;
	_TooltipsParserHandler->setTraversalNodeMask( _TooltipsSceneModifier->getAllowNodeMask() );
	_TooltipsParserHandler->addCommand( osgGA::GUIEventAdapter::FRAME, _TooltipsSceneModifier.get());
	
	this->addCommandSchedule((CommandSchedule*) _SceneModifier.get());
	this->addCommandSchedule((CommandSchedule*) _AnimateHandler.get());

	_Viewer->addEventHandler(_StandardNodeParserHandler.get());
	_Viewer->addEventHandler(_AnimateHandler.get());
	_Viewer->addEventHandler(_TooltipsParserHandler.get());
	_Viewer->addEventHandler(_LoaderThreadHandler.get());

	return true;
}

bool FunCore::initManipulators()
{
	this->sendNotifyMessage("initManipulators -> Initializing Manipulators.");

	//_WalkManip->setNode(_ModiSceneGraph.get());
	//_WalkManip->setViewer( _Viewer.get(), "", "" );

	_ViRoMan->setNode(_ModiSceneGraph.get());
	_ViRoMan->addPickHandler( _Viewer.get() );

	// Setup del ViRo
	// Opzioni iniziali da abilitare per VRome
	_ViRoMan->ClearOptions();
	_ViRoMan->Enable(
			_ViRoMan->Z_COLLIDE
			| _ViRoMan->COLLISIONS
			| _ViRoMan->PERIPHERAL_LOCK
			| _ViRoMan->GLASS_PRISON
			//| _ViRoMan->AVOIDANCE
			//| _ViRoMan->MULTISAMPLING
			//| _ViRoMan->SURFACE_ADAPTION
			| _ViRoMan->AUTO_SCALE_STEP
			| _ViRoMan->Z_LOCK
			);

	// distanza di impatto da superfici (es. muri)
	_ViRoMan->setImpactDistance( 0.2 );
	_ViRoMan->setAvoidanceDistance( 2.0 );

	// distanza di impatto da terra
	_ViRoMan->setGroundDistance( 2.0 );

	// Glass Prison
	osg::BoundingBox BB;									
	BB.set( osg::Vec3d(OSG4WEB_VROME_XMIN,OSG4WEB_VROME_YMIN,OSG4WEB_VROME_ZMIN), osg::Vec3d(OSG4WEB_VROME_XMAX,OSG4WEB_VROME_YMAX,OSG4WEB_VROME_ZMAX) );
	_ViRoMan->setGlassPrison( BB );

	//Attacca al Command Registry il Manipolator CommandSchedule
	//this->addCommandSchedule((CommandSchedule*) _WalkManip.get());
	this->addCommandSchedule((CommandSchedule*) _ViRoMan.get());

	_KeySwitchManipulator->addMatrixManipulator( '1', "ViRoManipulator", _ViRoMan.get());
	//_KeySwitchManipulator->addMatrixManipulator( '1', "WalkManipulator", _WalkManip.get());
	_KeySwitchManipulator->addMatrixManipulator( '2', "TrackBall", new osgGA::TrackballManipulator);
    _KeySwitchManipulator->selectMatrixManipulator(0);
  
	return true;
}


bool FunCore::buildMainScene()
{
	osg::ref_ptr<osg::Node> loadinghud = _LoaderThreadHandler->createLoadingHUD();
	osg::ref_ptr<osg::Node> tooltiphud = _TooltipsSceneModifier->createTooltipHUD();

	//Setto i nomi dei nodi
	_MainNode->setName("Super_Group_Node");
	_ModiSceneGraph->setName("Modi_Scene_Graph");

	//Attacco gli HUD 
	_MainNode->addChild(loadinghud.get());
	_MainNode->addChild(tooltiphud.get());
	
	// No Pick
	loadinghud.get()->setNodeMask( NOT_SOLID_MASK );
	tooltiphud.get()->setNodeMask( NOT_SOLID_MASK );

	//Attacco Nodi
	_MainNode->addChild( _ModiSceneGraph.get() );

	//Passo il nodo di scena al quale è possibile aggiungere figli
	_LoaderThreadHandler->setNode(_ModiSceneGraph.get());
	
	//TODO:

	return true;
}

/** Ridefinizione della funzione di Gestione Comandi per CommandSchedule "this" */
std::string FunCore::handleAction(std::string argument)
{
	std::string retstr = "COMMAND_OK";
	std::string lcommand, rcommand;

	this->sendNotifyMessage("handleAction -> Command Found");
	this->splitActionCommand(argument, lcommand, rcommand);

	switch(this->getCommandActionIndex(lcommand))
	{
	case SETMAINSCENEGRAPH:
		{
			if( !this->loadModel(rcommand, false) )
				retstr = "REQUEST_IN_QUEUE";
		}
		break;
	case ADDFILETONODE:
		{
			if( !this->loadModelToNode(rcommand) )
				retstr = "REQUEST_IN_QUEUE";
		}
		break;
	case CHECKNODEPRESENCE:
		{
			if( !_LoaderThreadHandler->checkNodePresenceByName(rcommand) )
				retstr = "NODE_NOT_FOUND";
			else
				retstr = "NODE_FOUND";
		}
		break;
	case SETOPTIMIZATION:
		{
			if(rcommand == "TRUE")
				_LoaderThreadHandler->setOptimization(true);
			else if(rcommand == "FALSE")
				_LoaderThreadHandler->setOptimization(true);
			else
				retstr = "BAD_COMMAND";
		}
		break;
	case SWITCH_MANIPULATORS:
		{
			//if(_KeySwitchManipulator->getCurrentMatrixManipulator() == _WalkManip.get())
			if(_KeySwitchManipulator->getCurrentMatrixManipulator() == _ViRoMan.get())
				_KeySwitchManipulator->selectMatrixManipulator(1);
			else
				_KeySwitchManipulator->selectMatrixManipulator(0);

		}
		break;
	case RESET_SCENE:
		{
			if( !this->loadModel(rcommand, true) )
				retstr = "REQUEST_IN_QUEUE";
		}
		break;
	case ACTIVATE_JSMAP_MESSAGES:
		{
			_jsMapActive = true;
		}
		break;
	case DEACTIVATE_JSMAP_MESSAGES:
		{
			_jsMapActive = false;	
		}
		break;
	case GET_JSMAP_COORDINATES:
		{
			retstr = this->getJSMapCoordinates();
		}
		break;

	case SETHOMEPOSITION:
		{
			if ( this->setHomePosition( rcommand ) ) retstr = "HOME_SET";
			else retstr = "HOME_NOT_SET";
		}
		break;
		
	case SETBOUNDS:
		{
			if ( this->setBounds( rcommand ) ) retstr = "BOUNDS_SET";
			else retstr = "INVALID_BOUNDS";
		}
		break;

	default: //UNKNOWN_ACTION
		retstr = "UNKNOWN_ACTION";
		break;
	}
	
	return retstr;
}

bool FunCore::loadModel(std::string nodename, bool erase)
{
	if(erase)
	{
		osgDB::Registry::instance()->getOrCreateDatabasePager()->clear();
		osgDB::Registry::instance()->getOrCreateDatabasePager()->resetStats();
		_LoaderThreadHandler->clear();
		_ModiSceneGraph->removeChildren(0, _ModiSceneGraph->getNumChildren()); //Clear di tutta la parte di scenegraph modelli
		_maininit = false;
	}

	std::string address, filename;

	int pos = nodename.rfind("/");

	if(pos != std::string::npos)
	{
		address = nodename.substr(0, pos);
		filename = nodename.substr(pos + 1, nodename.size());

		_LoaderThreadHandler->setServerPrefix(address + "/");
	}
	
	return _LoaderThreadHandler->requestLoading(nodename);
}

bool FunCore::loadModelToNode(std::string arguments)
{
	std::string nodename, newargs;
	this->splitActionCommand(arguments, nodename, newargs);
	
	return _LoaderThreadHandler->requestLoading(nodename, newargs); //FIXME: ritorna false se il nodo è già in coda di caricamento
}

void FunCore::preFrameUpdate()
{
	this->handleLoadingThreads();

	if(!_envdone)
		this->handleEnvironment();

	this->handleTooltips();

	this->sendLookAtToJS();
}

void FunCore::sendLookAtToJS()
{
	if(_jsMapActive)
	{
		if( osg::Timer::instance()->delta_m( _sendCEventToJSTime, osg::Timer::instance()->tick() ) > CYCLICIC_RAISE_EVENT_TO_JS_TIMER )
		{
			this->raiseCommand( this->getJSMapCoordinates() );
		
			_sendCEventToJSTime = osg::Timer::instance()->tick();
		}
	}
}

std::string FunCore::getJSMapCoordinates()
{
	osg::Vec3 eye, center, up;
	_MainCamera->getViewMatrixAsLookAt(eye, center, up);

	std::string jsmapcoordstring("JSMAP");
	
	jsmapcoordstring += 
		" EYE " + Utilities::StringUtils::numToString(eye.x()) + " " + Utilities::StringUtils::numToString(eye.y()) + " " + Utilities::StringUtils::numToString(eye.z()) + 
		" CENTER " + Utilities::StringUtils::numToString(center.x()) + " " + Utilities::StringUtils::numToString(center.y()) + " " + Utilities::StringUtils::numToString(center.z()) +
		" UP " + Utilities::StringUtils::numToString(up.x()) + " " + Utilities::StringUtils::numToString(up.y()) + " " + Utilities::StringUtils::numToString(up.z());

	return jsmapcoordstring;
}

bool FunCore::setHomePosition( std::string arguments ){
	if (_Viewer.get() ){
		double x,y,z, tx,ty,tz;
		if (_Viewer.get()->getCameraManipulator() && ( sscanf(arguments.c_str(), "%lf %lf %lf %lf %lf %lf", &x,&y,&z,&tx,&ty,&tz) == 6) ){
			_Viewer.get()->getCameraManipulator()->setHomePosition( osg::Vec3d(x,y,z), osg::Vec3d(tx,ty,tz), osg::Z_AXIS );
			return true;
			}
		}
	return false;
}

bool FunCore::setBounds( std::string arguments ){
	if ( _ViRoMan.get() ){
		double x,y,z, X,Y,Z;
		if ( sscanf(arguments.c_str(), "%lf %lf %lf %lf %lf %lf", &x,&y,&z,&X,&Y,&Z) == 6 ){
			if (x>=X || y>=Y || z>=Z) return false;

			// Glass Prison
			osg::BoundingBox BB;									
			BB.set( osg::Vec3d(x,y,z), osg::Vec3d(X,Y,Z) );
			_ViRoMan->setGlassPrison( BB );
			_ViRoMan->Enable( _ViRoMan->GLASS_PRISON );

			return true;
			}
		}
	return false;
}

void FunCore::handleTooltips()
{
	if(_TooltipsSceneModifier.valid())
		_TooltipsSceneModifier->checkTooltipsTiming();
}

void FunCore::handleEnvironment()
{
	Visitors::FindNodeVisitor fnvenvironment("Skybox_Geode_To_Attach");
	_ModiSceneGraph->accept(fnvenvironment);
	
	if( fnvenvironment.getNodeFoundSize() == 0)
		return;

	_envdone = true;

	unsigned int npgeosize = fnvenvironment.getNodeByIndex(0).size();

	if(npgeosize < 2)
		return;

	osg::ref_ptr<osg::Geode> skyboxgeode = dynamic_cast<osg::Geode*>(fnvenvironment.getNodeByIndex(0).at(npgeosize - 1));
	osg::ref_ptr<osg::Group> skyboxgroup = dynamic_cast<osg::Group*>(fnvenvironment.getNodeByIndex(0).at(npgeosize - 2));

	if( !skyboxgeode.valid() || !skyboxgroup.valid())
	{
		fnvenvironment.getNodeByIndex(0).at(npgeosize - 1)->setName("FAKE_Skybox_Group_To_Attach");
		return;
	}

	fnvenvironment.getNodeByIndex(0).at(npgeosize - 1)->setName("FAKE_Skybox_Group_To_Attach"); //Viene cambiato alla fine se va tutto bene

	skyboxgroup->removeChild(skyboxgeode.get());
	
	// No Picking
	skyboxgroup->setNodeMask( NOT_SOLID_MASK );
	skyboxgeode->setNodeMask( NOT_SOLID_MASK );
	
	Visitors::FindNodeVisitor fnvcleargroup("Default_Clear_Group");
	_ModiSceneGraph->accept(fnvcleargroup);

	if(fnvcleargroup.getNodeFoundSize() > 0)
	{
		unsigned int npclgsize = fnvcleargroup.getNodeByIndex(0).size();
		if(npclgsize < 2)
			return;
			
		//Rimuovo il Gruppo default di clear
		osg::ref_ptr<osg::Group> parent = (osg::Group*) fnvcleargroup.getNodeByIndex(0).at(npclgsize - 2);
		parent->removeChild(fnvcleargroup.getNodeByIndex(0).at(npclgsize - 2));
	}

	osg::ref_ptr<osg::ClearNode> clearNode = new osg::ClearNode;
	clearNode->setName("Environment_Map_Clear_Node");
	clearNode->setClearColor(osg::Vec4(54.0/255.0, 137.0/255.0, 152.0/255.0 , 1.0));
	clearNode->setNodeMask( NOT_SOLID_MASK );
			
	osg::ref_ptr<osg::Transform> transform = new MoveEarthySkyWithEyePointTransform;
	clearNode->setName("Environment_Map_Transform_Node");
	transform->setCullingActive(false);
	transform->addChild(clearNode.get());

	_ModiSceneGraph->addChild(transform.get());

	//--- Environment Fog and Light
//#define _NITE_
	
	osg::ref_ptr<osg::Group> EnvNode = new osg::Group;
	osg::ref_ptr<osg::Fog> EnvFog = new osg::Fog;
	osg::ref_ptr<osg::StateSet> EnvSS;
	osg::ref_ptr<osg::LightSource> EnvSun;
	osg::ref_ptr<osg::LightSource> EnvCL;

	EnvNode->setNodeMask( NOT_SOLID_MASK );
	EnvSS = EnvNode->getOrCreateStateSet();
	
	EnvFog->setMode(osg::Fog::EXP2);

#ifdef _NITE_
	EnvFog->setColor( osg::Vec4f(0.0,0.0,0.0, 1.0) );
#else
	EnvFog->setColor( osg::Vec4f(0.95,1.0,1.0, 1.0) );
#endif

	EnvSS->setAttributeAndModes( EnvFog.get() );
	//EnvSS->setMode(GL_CULL_FACE,osg::StateAttribute::ON);

	EnvSun = new osg::LightSource;
	EnvCL  = new osg::LightSource;
	
#define ENV_SUN_POSITION	osg::Vec4(0,0,20000, 0.0)

	EnvSun->getLight()->setPosition( ENV_SUN_POSITION );
	EnvSun->getLight()->setLightNum(0);
	
	EnvCL->getLight()->setPosition( -ENV_SUN_POSITION );
	EnvCL->getLight()->setLightNum(1);
	

#ifdef _NITE_
	EnvSun->getLight()->setAmbient( osg::Vec4(0.0,0.1,0.6, 1.0) );
	EnvCL->getLight()->setDiffuse( osg::Vec4(1.0,1.0,0.5, 1.0) );
#else
	EnvSun->getLight()->setAmbient( osg::Vec4(0.7,0.7,0.6, 1.0) );
	EnvCL->getLight()->setDiffuse( osg::Vec4(0.1,0.1,0.1, 1.0) );
#endif

	EnvNode->addChild( EnvSun.get() );
	EnvNode->addChild( EnvCL.get() );

	// Water
/*	
	#define WATER_H		30.0

	osg::Geometry* water = new osg::Geometry;
    osg::Vec3Array* coords = new osg::Vec3Array(4);
    (*coords)[0].set(OSG4WEB_VROME_XMIN,OSG4WEB_VROME_YMAX, WATER_H);
    (*coords)[1].set(OSG4WEB_VROME_XMIN,OSG4WEB_VROME_YMIN, WATER_H);
    (*coords)[2].set(OSG4WEB_VROME_XMAX,OSG4WEB_VROME_YMIN, WATER_H);
    (*coords)[3].set(OSG4WEB_VROME_XMAX,OSG4WEB_VROME_XMAX, WATER_H);
    water->setVertexArray(coords);

    osg::Vec3Array* norms = new osg::Vec3Array(1);
    (*norms)[0].set(0.0f,0.0f,1.0f);
    water->setNormalArray(norms);
    water->setNormalBinding(osg::Geometry::BIND_OVERALL);

	osg::Vec4Array* colours = new osg::Vec4Array(1);
    (*colours)[0].set(1.0f,1.0f,1.0, 1.0f);
    water->setColorArray(colours);
    water->setColorBinding(osg::Geometry::BIND_OVERALL);

    water->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::QUADS,0,4));
	osg::ref_ptr<osg::Geode> WaterNode = new osg::Geode;
	WaterNode.get()->addDrawable( water );
*/

	/////////////////////////////////////////////////////////////////////

	osg::ref_ptr<osg::StateSet> stateset = skyboxgeode->getStateSet();
	if(stateset.valid())
	{
		osg::ref_ptr<osg::TexMat> tm = dynamic_cast<osg::TexMat*>(stateset->getTextureAttribute(0, osg::StateAttribute::TEXMAT));

		if(tm.valid())
		{
			clearNode->setCullCallback(new TexMatCallback(*tm));
			clearNode->addChild(skyboxgeode.get());
			skyboxgeode->setName("ATTACHED_Skybox_Geode_To_Attach");
		}
	}

	if (_MainNode.get() ){
		//_MainNode->addChild( WaterNode.get() );
		_MainNode->addChild( EnvNode.get() );
		EnvFog->setDensity( 400.0 / _MainNode.get()->getBound().radius() );			// Formula da testare con differenti modelli
		_MainNode->getOrCreateStateSet()->setAttributeAndModes( EnvFog.get() );
		if ( _ViRoMan.get() ) _ViRoMan->setEnvFog( EnvFog.get() );
	}
}

void FunCore::handleLoadingThreads()
{
	//Handle dei loading
	osg::ref_ptr<osg::Node> lnode = _LoaderThreadHandler->handleLoading();

	//Se nodo valido lo attacco direttamente
	if( lnode.valid() )
	{
		if( !_maininit ) //Nodo attaccato diretto è solo il Master di Scena
		{
			_ModiSceneGraph->addChild(lnode.get());

			Visitors::FindNodeVisitor fnvconstraints("Contraints_Geo_Matrix");
			_ModiSceneGraph->accept(fnvconstraints);

			if( fnvconstraints.getNodeFoundSize() == 1)
			{
				osg::ref_ptr<osg::Node> node = fnvconstraints.getNodeByIndex(0).at(fnvconstraints.getNodeByIndex(0).size() -1);

				if(node.valid())
				{
					/*
					_WalkManip->searchPosUsingSubNode(node.get(), 0.75f);
					_WalkManip->setGroundCollisionOnOff(false);
					_WalkManip->setStepAmount( node->getBound().radius() / 80.0f, node->getBound().radius() / 80.0f);
					_WalkManip->setIntersectSegmenteMultiplier(1.5, 1.0);	
					*/
				}
				//else _WalkManip->searchDefaultPos();
			}
			//else _WalkManip->searchDefaultPos();

			osg::ref_ptr<osg::Group> clearGroup = new osg::Group;
			clearGroup->setName("Default_Clear_Group");

			osg::ref_ptr<osg::ClearNode> clearNode = new osg::ClearNode;
			clearNode->setName("Environment_Map_Clear_Node");
			//clearNode->setClearColor(osg::Vec4(54.0/255.0, 137.0/255.0, 152.0/255.0 , 1.0));
			clearNode->setClearColor(osg::Vec4(1,1,1,1));

			clearGroup->addChild(clearNode.get());
			_ModiSceneGraph->addChild(clearGroup.get());

			// Andrebbe in buildMainScene()
			//-----------------------------
			if ( _ViRoMan.get() ){
				// Crea HUD
				_HUD = new ViroHud( _ViRoMan.get() );
				//_HUD->setViewportSize(this->getCurrentWinWidth(),this->getCurrentWinHeight());
				_HUD->setViewportSize(1400,700);
				_HUD->Realize();

				_MainNode->addChild( _HUD.get()->getHUD() );

				// Crea Callback di aggiornamento HUD
				ViroHudUpdater* hupd = new ViroHudUpdater( _HUD.get() );
				_HUD->getHUD()->setUpdateCallback( hupd );

				// setup
				_HUD->setServerPrefix( _LoaderThreadHandler->getServerPrefix() );
				}

			_Viewer->home();
			_maininit = true;
		}
		else
		{
			_ModiSceneGraph->addChild(lnode.get());
			//this->raiseCommand("ALERT Scena già inizializzata");
		}
	}

	//Controllo errori di caricamento
	std::vector<std::string> failedln;
	if(_LoaderThreadHandler->handleLoadingErrors(failedln))
	{
		for(unsigned int i = 0; i < failedln.size(); i++)
			this->raiseCommand("ALERT Errore durante il caricamento del file: " + failedln.at(i));
	}
}