
#include <LoadCore/LoadCore.h>


#include <osg/MatrixTransform>

#include <osgDB/ReadFile>

#include <osgUtil/Optimizer>

using namespace OSG4WebCC;


//LoadCore Costruttore
LoadCore::LoadCore(std::string corename) : CoreBase(corename),
	_NullManipulator( new Manipulators::NullManipulator() ),
	_TextMessage(new osgText::Text),
	_MessageTimer(NULL),
	_SwitchMessage(new osg::Switch),
	_LoadedNode(NULL),
	_CapsuleBarSD(new osg::ShapeDrawable),
	_CapsuleBarSwitch(new osg::Switch)
{
	this->sendNotifyMessage("LoadCore -> Costructing LoadCore Instance.");

	//Ridefinisco il nome dello Schedule Command
	this->setCommandScheduleName("LOADCORE");

	this->setCommandAction("LOAD_MODEL");
	this->setCommandAction("STATUSBAR_VALUE");
	this->setCommandAction("STATUSBAR_COLOR");
	this->setCommandAction("STATUSBAR_VISIBILITY");
	this->setCommandAction("SETMESSAGE");
	this->setCommandAction("SETMESSAGE_COLOR");

	this->addCommandSchedule((CommandSchedule*) this);
}

//LoadCore Distruttore
LoadCore::~LoadCore()
{
	if(_LoadedNode.valid())
		_LoadedNode = NULL;

	if(_SwitchMessage.valid())
		_SwitchMessage = NULL;

	if(_TextMessage.valid())
		_TextMessage = NULL;

	if(_CapsuleBarSD.valid())
		_CapsuleBarSD = NULL;

	if(_CapsuleBarSwitch.valid())
		_CapsuleBarSwitch = NULL;

	if(_NullManipulator.valid())
		_NullManipulator = NULL;

	if(_MessageTimer)
	{
		delete _MessageTimer;
		_MessageTimer = NULL;
	}

	this->sendNotifyMessage("~LoadCore -> Destructing LoadCore Instance.");
}

bool LoadCore::initSceneData()
{
	this->sendNotifyMessage("initSceneData -> Building the SceneGraph.");

	bool ret = this->createOSG4WebLogo();

	_Viewer->setSceneData(_LocalSceneGraph.get());

	return ret;
}

bool LoadCore::initManipulators()
{
	this->sendNotifyMessage("initManipulators -> Initializing Manipulators.");
	
	_NullManipulator->setNode( _LocalSceneGraph.get() );

	_KeySwitchManipulator->addMatrixManipulator( '1', "NullManipulator", _NullManipulator.get());
	_KeySwitchManipulator->addMatrixManipulator( '2', "TrackballManipulator", _TrackballManipulator.get());
    _KeySwitchManipulator->selectMatrixManipulator(0);
  
	return true;
}

bool LoadCore::loadModel(std::string nodename, bool erase)
{
	bool ret = false;

	this->sendNotifyMessage("loadModel -> Loading Model: " + nodename);

	_LoadedNode = osgDB::readNodeFile(nodename);

	if(_LoadedNode.valid())
	{
		osgUtil::Optimizer optimizer;
		optimizer.optimize(_LoadedNode.get());
		optimizer.reset();

		if(erase)
			_LocalSceneGraph->removeChildren(0, _LocalSceneGraph->getNumChildren());
		
		_LocalSceneGraph->addChild(_LoadedNode.get());

		//Attivo il trackball
		_KeySwitchManipulator->selectMatrixManipulator(1);
		
		_Viewer->home();

		this->sendNotifyMessage("loadModel -> Model Loaded Correctly");

		ret = true;
	}
	else
	{
		this->sendWarnMessage("loadModel -> Loading Model Failed. Model Name: " + nodename);
		ret = false; 
	}

	return ret;
}

bool LoadCore::refreshStatusBarValue(std::string value)
{
	bool ret = false;
	double newvalue;

	std::istringstream iss(value);
	ret = !(iss >> newvalue >> std::dec).fail();

	if(ret)
	{
		if(_CapsuleBarSD.valid())
		{
			osg::ref_ptr<osg::Capsule> capsule = dynamic_cast<osg::Capsule*>( _CapsuleBarSD->getShape() );
	
			if(capsule.valid())
			{
				capsule->setHeight( newvalue );
				_CapsuleBarSD->dirtyDisplayList();
				_CapsuleBarSD->dirtyBound();
			}
			else
				this->sendWarnMessage("refreshStatusBarValue -> _CapsuleBar getShape() failed");
		}
		else
			this->sendWarnMessage("refreshStatusBarValue -> _CapsuleBar not initialized");

	}
	else
		this->sendWarnMessage("refreshStatusBarValue -> Error string conversion in decimal");
	
	return ret;
}

bool LoadCore::setStatusBarColor(std::string colorname)
{
	bool ret = true;

	this->sendNotifyMessage("setStatusBarColor -> Changing status bar color");

	if(colorname == "OSG_GREEN")
		_CapsuleBarSD->setColor(OSG4WEB_COLORVEC_GREEN);
	else if(colorname == "OSG_BLUE")
		_CapsuleBarSD->setColor(OSG4WEB_COLORVEC_BLUE);
	else //(colorname == "OSG_RED")
		_CapsuleBarSD->setColor(OSG4WEB_COLORVEC_RED);

	return ret;
}

bool LoadCore::setStatusBarVisibility(std::string value)
{
	this->sendNotifyMessage("setStatusBarVisibility -> set visibility to " + value);

	if(value == "TRUE")
		_CapsuleBarSwitch->setValue(0, !_CapsuleBarSwitch->getValue(0)); //Uso la posizione precedente
	else //value == "FALSE"
		_CapsuleBarSwitch->setAllChildrenOff();

	return true;
}

bool LoadCore::createOSG4WebLogo()
{
	float radius = 6.5f;
    float height = 0.0f;

	double capsulerotateangle = 90.0;
	osg::Vec3 capsulerotateaxis(0.0, 1.0, 0.0);
	osg::Vec3 capsuleposition(50.0, 0.0, -120.0);

	std::string osglogo(this->getInstallationDirectory() + "/" + OSG4WEB_LOADCORE_LOGO);

	osg::ref_ptr<osg::Group> scene = new osg::Group;
	osg::ref_ptr<osg::Group> delight = new osg::Group;
	osg::ref_ptr<osg::MatrixTransform> capsulebarmt = new osg::MatrixTransform;
	osg::ref_ptr<osg::Geode> capsulegeode = new osg::Geode;
	osg::ref_ptr<osg::Capsule> capsulebar = new osg::Capsule;

	this->sendNotifyMessage("createOSG4WebLogo -> Creating OSG4Web Main Logo");
	
	capsulebarmt->setMatrix(
		osg::Matrix::rotate( 
			osg::inDegrees(capsulerotateangle), capsulerotateaxis) *
                 osg::Matrix::translate(capsuleposition)
				 );

	capsulebar->setName("Capsule Status Bar");
	capsulebar->setHeight(height);
	capsulebar->setCenter(osg::Vec3(0.0f,0.0f,0.0f));
	capsulebar->setRadius(radius);

	osg::ref_ptr<osg::TessellationHints> hints = new osg::TessellationHints;
    hints->setDetailRatio(0.5f);

	_CapsuleBarSD->setShape(capsulebar.get());
	_CapsuleBarSD->setTessellationHints(hints.get());
	
	_CapsuleBarSD->setColor(osg::Vec4(1.0f,0.12f,0.06f,1.0f));
	
	capsulegeode->addDrawable(_CapsuleBarSD.get());
	
	//Attacco CapsuleStatusbar alla MT
	capsulebarmt->addChild(capsulegeode.get());

	//Attacco CapsuleMT a Switch e metto off
	_CapsuleBarSwitch->addChild(capsulebarmt.get(), false);

	osg::ref_ptr<osg::StateSet> stateset = delight->getOrCreateStateSet();
    stateset->setMode(GL_LIGHTING,osg::StateAttribute::OVERRIDE|osg::StateAttribute::OFF);

	osg::ref_ptr<osg::ClearNode> backdrop = new osg::ClearNode;
    backdrop->setClearColor(osg::Vec4(1.0f,1.0f,1.0f,0.0f));
	
	//Creo il testo
	delight->addChild(createOSG4WebLogoText());
	
	//Creo lo spazio messaggi
	delight->addChild(createOSG4WebMessages());

	//Carico il modello World
  	_LoadedNode = osgDB::readNodeFile(osglogo);

	if(_LoadedNode.valid())
		delight->addChild(_LoadedNode.get());
	else
		this->sendWarnMessage("createOSG4WebLogo -> Error loading logo model: " + osglogo);

	scene->addChild(delight.get());
	scene->addChild(backdrop.get());
	scene->addChild(_CapsuleBarSwitch.get());

	_LocalSceneGraph->addChild(scene.get());

	// Optimize the model
	osgUtil::Optimizer optimizer;
	optimizer.optimize(_LocalSceneGraph.get());
	optimizer.reset();

	return true;
}

osg::Node* LoadCore::createOSG4WebMessages()
{
	osg::Camera *camera = new osg::Camera;

	osg::ref_ptr<osg::Geode> tgeode = new osg::Geode;

	this->sendNotifyMessage("createOSG4WebMessages -> Creating OSG4Web Status Messages");

	camera->setReferenceFrame(osg::Transform::ABSOLUTE_RF);
	camera->setProjectionMatrixAsOrtho2D(0, 1000, 0, 1000); 
	camera->setViewMatrix(osg::Matrix::identity());
	camera->setClearMask(GL_DEPTH_BUFFER_BIT);
	camera->addChild(_SwitchMessage.get());
	camera->getOrCreateStateSet()->setMode(GL_LIGHTING,osg::StateAttribute::OFF);

	_TextMessage->setFont(this->getInstallationDirectory() + "/" + OSG4WEB_LOADCORE_FONT);
	_TextMessage->setColor(OSG4WEB_COLORVEC_RED);
	_TextMessage->setCharacterSize(35.0f);
	_TextMessage->setPosition(osg::Vec3(1000.0 / 2.0, 1000.0 / 3.3, 0.0));
        
	_TextMessage->setAlignment(osgText::Text::CENTER_BASE_LINE);
	_TextMessage->setFontResolution(32,32); 
        
	_TextMessage->setText("Initializing OSG4Web...");

	_TextMessage->setBackdropType(osgText::Text::OUTLINE);
    _TextMessage->setBackdropImplementation(osgText::Text::POLYGON_OFFSET);
    _TextMessage->setBackdropOffset(0.05f);
    _TextMessage->setBackdropColor(osg::Vec4(0.1f, 0.1f, 0.1f, 1.0f));

	tgeode->addDrawable(_TextMessage.get());

	_SwitchMessage->addChild(tgeode.get(), false);

	return camera;
}

osg::Node* LoadCore::createOSG4WebLogoText()
{
	this->sendNotifyMessage("createOSG4WebLogoText -> Creating OSG4Web Logo Text");

	osg::Geode* geode = new osg::Geode();
	osg::BoundingBox bb(osg::Vec3(0.0f,0.0f,0.0f),osg::Vec3(100.0f,100.0f,100.0f));

	osg::ref_ptr<osg::StateSet> stateset = geode->getOrCreateStateSet();
    stateset->setMode(GL_LIGHTING,osg::StateAttribute::OFF);

    std::string font(this->getInstallationDirectory() + "/" + OSG4WEB_LOADCORE_FONT);
	std::string label("OSG4Web");

	osg::ref_ptr<osgText::Text> text = new  osgText::Text;
 
    text->setFont(font);
	text->setFontResolution(64,64);
    text->setAlignment(osgText::Text::CENTER_CENTER);
    text->setAxisAlignment(osgText::Text::XZ_PLANE);
    text->setCharacterSize((bb.zMax()-bb.zMin())*0.65f);
    text->setPosition(bb.center()-osg::Vec3(0.0f,0.0f,(1.2f)*(bb.zMax()-bb.zMin())));
    text->setColor(osg::Vec4(0.20f,0.45f,0.60f,1.0f));
    text->setText(label);

    text->setBackdropType(osgText::Text::OUTLINE);
    text->setBackdropImplementation(osgText::Text::POLYGON_OFFSET);
    text->setBackdropOffset(0.05f);
    text->setBackdropColor(osg::Vec4(0.0f, 0.0f, 0.5f, 1.0f));

	text->setColorGradientMode(osgText::Text::OVERALL);
    osg::Vec4 lightblue(0.30f,0.6f,0.90f,1.0f);
    osg::Vec4 blue(0.10f,0.30f,0.40f,1.0f);
    text->setColorGradientCorners(lightblue, blue, blue, lightblue);

    geode->addDrawable( text.get() );
        
    return geode;
}

bool LoadCore::setHUDMessage(std::string msg)
{
	if(_MessageTimer) //Forzo la chiusura del timer precedente
	{
		delete _MessageTimer;
		_MessageTimer = NULL;
	}

	if(msg.empty())
	{
		this->sendNotifyMessage("setHUDMessage -> Switching Off Status Messages");
		_SwitchMessage->setAllChildrenOff();
	}
	else
	{
		this->sendNotifyMessage("setHUDMessage -> Switching On new Status Messages: " + msg);
		_TextMessage->setText(msg);
	
		_MessageTimer = new osg::Timer;
		_SwitchMessage->setAllChildrenOn();
	}
	
	return true;
}

bool LoadCore::setHUDMessageColor(std::string colorname)
{
	bool ret = true;

	this->sendNotifyMessage("setHUDMessageColor -> Setting Status Message Color: " + colorname);

	if(colorname == "LC_OSG_GREEN")
		_TextMessage->setColor(OSG4WEB_COLORVEC_GREEN);
	else if(colorname == "LC_OSG_BLUE")
		_TextMessage->setColor(OSG4WEB_COLORVEC_BLUE);
	else //(colorname == "LC_OSG_RED")
		_TextMessage->setColor(OSG4WEB_COLORVEC_RED);

	return ret;
}

void LoadCore::preFrameUpdate() 
{
	//Controllo se c'è un Timer Messaggio 
	if(_MessageTimer)
	{
		if(_MessageTimer->time_s() >= OSG4WEB_MESSAGE_TIMER_ADDER)
		{
			_SwitchMessage->setAllChildrenOff();
			
			 //Spengo il timer
			delete _MessageTimer;
			_MessageTimer = NULL;
		}
	}
}


/** Ridefinizione della funzione di Gestione Comandi per CommandSchedule "this" */
std::string LoadCore::handleAction(std::string argument)
{
	std::string retstr = "CORE_DONE";
	std::string lcommand, rcommand;

	this->splitActionCommand(argument, lcommand, rcommand);
	this->sendNotifyMessage("handleAction -> Command Found");

	switch(this->getCommandActionIndex( lcommand ))
	{
	case LOAD_MODEL: //LOAD_MODEL
		if( !this->loadModel( rcommand, true ) )
			retstr = "CORE_FAILED";
		break;
	case STATUSBAR_VALUE: //STATUSBAR_VALUE
		if( !this->refreshStatusBarValue( rcommand ) )
			retstr = "CORE_FAILED";
		break;
	case STATUSBAR_COLOR: //STATUSBAR_COLOR
		if( !this->setStatusBarColor( rcommand ) )
			retstr = "CORE_FAILED";
		break;
	case STATUSBAR_VISIBILITY: //STATUSBAR_VISIBILITY
		if( !this->setStatusBarVisibility( rcommand ) )
			retstr = "CORE_FAILED";
		break;
	case SETMESSAGE: //SETMESSAGE
		if( !this->setHUDMessage( rcommand ) )
			retstr = "CORE_FAILED";
		break;
	case SETMESSAGE_COLOR: //SETMESSAGE_COLOR
		if( !this->setHUDMessageColor( rcommand ) )
			retstr = "CORE_FAILED";
		break;
	default: //UNKNOWN_CORE_COMMAND
		retstr = "UNKNOWN_CORE_COMMAND";
		break;
	}
	
	return retstr;
}

/*
std::string LoadCore::DoCommand(std::string command)
{
	this->sendNotifyMessage("DoCommand -> Command string: " + command);

	std::string retstr( "CORE_BADCOMMAND" );
	std::string::size_type pos = command.find( " " );
	std::string lcommand, rcommand;

	if( pos != std::string::npos )
	{
		lcommand = command.substr(0, pos);
		rcommand = command.substr(pos + 1, command.size() -1);
	}
	else
	{
		lcommand = command;
		rcommand.clear();
	}

	if(lcommand == "LC_LOAD_MODEL") //Only for testing core
	{
		if( !this->loadModel(rcommand) )
			retstr = "CORE_FAILED";
		else
			retstr = "CORE_DONE";
	}
	else if(lcommand == "LC_STATUSBAR_VALUE")
	{
		if( !this->refreshStatusBarValue(rcommand) )
			retstr = "CORE_FAILED";
		else
			retstr = "CORE_DONE";
	}
	else if(lcommand == "LC_STATUSBAR_COLOR")
	{
		if( !this->setStatusBarColor(rcommand) )
			retstr = "CORE_FAILED";
		else
			retstr = "CORE_DONE";
	}
	else if(lcommand == "LC_STATUSBAR_VISIBILITY")
	{
		if( !this->setStatusBarVisibility(rcommand) )
			retstr = "CORE_FAILED";
		else
			retstr = "CORE_DONE";
	}
	else if(lcommand == "LC_SETMESSAGE")
	{
		if( !this->setHUDMessage(rcommand) )
			retstr = "CORE_FAILED";
		else
			retstr = "CORE_DONE";
	}
	else if(lcommand == "LC_SETMESSAGE_COLOR")
	{
		if( !this->setHUDMessageColor(rcommand) )
			retstr = "CORE_FAILED";
		else
			retstr = "CORE_DONE";
	}
	else
	{
		this->sendWarnMessage("DoCommand -> Bad Command: " + command);
		retstr = "CORE_BADCOMMAND";
	}
	
	return retstr;
}

 */