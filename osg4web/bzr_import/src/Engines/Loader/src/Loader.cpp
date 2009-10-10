
#include <Loader/Loader.h>


#include <osg/MatrixTransform>

#include <osgDB/ReadFile>

#include <osgUtil/Optimizer>

using namespace CommonCore;


//Loader Costruttore
Loader::Loader(std::string corename) : CoreBase(corename),
	_NullManipulator( new Manipulators::NullManipulator() ),
	_TextMessage(new osgText::Text),
	_MessageTimer(NULL),
	_SwitchMessage(new osg::Switch),
	_LoadedNode(NULL),
	_CameraNode(new osg::Camera),
	_CapsuleBarSD(new osg::ShapeDrawable),
	_CapsuleBarSwitch(new osg::Switch)
{
	this->sendNotifyMessage("Loader -> Costructing Loader Core Instance.");

	//Ridefinisco il nome dello Schedule Command
	this->setCommandScheduleName("LOADER");

	//Default value
	this->setCommandAction("UNKNOWN_ACTION");
	this->setCommandAction("LOAD_MODEL");
	this->setCommandAction("STATUSBAR_VALUE");
	this->setCommandAction("STATUSBAR_COLOR");
	this->setCommandAction("STATUSBAR_VISIBILITY");
	this->setCommandAction("SETMESSAGE");
	this->setCommandAction("SETMESSAGE_COLOR");

	this->addCommandSchedule((CommandSchedule*) this);
}

//Loader Distruttore
Loader::~Loader()
{
	if(_LoadedNode.valid())
		_LoadedNode = NULL;

	if(_CameraNode.valid())
		_CameraNode = NULL;

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

	this->sendNotifyMessage("~Loader -> Destructing Loader Core Instance.");
}

bool Loader::initSceneData()
{
	this->sendNotifyMessage("initSceneData -> Building the SceneGraph.");

	bool ret = this->createOSG4WebLogo();

	_Viewer->setSceneData(_LocalSceneGraph.get());

	return ret;
}

bool Loader::initManipulators()
{
	this->sendNotifyMessage("initManipulators -> Initializing Manipulators.");
	
	_NullManipulator->setNode( _LocalSceneGraph.get() );

	_KeySwitchManipulator->addMatrixManipulator( '1', "NullManipulator", _NullManipulator.get());
	_KeySwitchManipulator->addMatrixManipulator( '2', "TrackballManipulator", _TrackballManipulator.get());
    _KeySwitchManipulator->selectMatrixManipulator(0);
  
	return true;
}

bool Loader::loadModel(std::string nodename, bool erase)
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

bool Loader::refreshStatusBarValue(std::string value)
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
				capsule->setHeight( newvalue * OSG4WEB_STATUS_BAR_MULTIPLIER * 100.0 ); //riporto in %
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

bool Loader::setStatusBarColor(std::string colorname)
{
	bool ret = true;

	this->sendNotifyMessage("setStatusBarColor -> Changing status bar color");

	if(colorname == "LC_OSG_GREEN")
		_CapsuleBarSD->setColor(OSG4WEB_COLORVEC_GREEN);
	else if(colorname == "LC_OSG_BLUE")
		_CapsuleBarSD->setColor(OSG4WEB_COLORVEC_BLUE);
	else //(colorname == "LC_OSG_RED")
		_CapsuleBarSD->setColor(OSG4WEB_COLORVEC_RED);

	return ret;
}

bool Loader::setStatusBarVisibility(std::string value)
{
	this->sendNotifyMessage("setStatusBarVisibility -> set visibility to " + value);

	if(value == "TRUE")
		_CapsuleBarSwitch->setValue(0, !_CapsuleBarSwitch->getValue(0)); //Uso la posizione precedente
	else //value == "FALSE"
		_CapsuleBarSwitch->setAllChildrenOff();

	return true;
}

bool Loader::createOSG4WebLogo()
{
	float radius = 6.5f;
    float height = 0.0f;

	double capsulerotateangle = 90.0;
	osg::Vec3 capsulerotateaxis(0.0, 1.0, 0.0);
	osg::Vec3 capsuleposition(50.0, 0.0, -120.0);

	std::string osglogo(this->getInstallationDirectory() + "/" + OSG4WEB_LOADER_LOGO);

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
    backdrop->setClearColor(OSG4WEB_CLEARCOLOR_BACKGROUND);
	
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

osg::Node* Loader::createOSG4WebMessages()
{
	osg::ref_ptr<osg::Geode> tgeode = new osg::Geode;

	this->sendNotifyMessage("createOSG4WebMessages -> Creating OSG4Web Status Messages");

	_CameraNode->setReferenceFrame(osg::Transform::ABSOLUTE_RF);
	_CameraNode->setProjectionMatrixAsOrtho2D(0, 1280, 0, 1024); 
	_CameraNode->setViewMatrix(osg::Matrix::identity());
	_CameraNode->setClearMask(GL_DEPTH_BUFFER_BIT);
	_CameraNode->addChild(_SwitchMessage.get());
	_CameraNode->getOrCreateStateSet()->setMode(GL_LIGHTING,osg::StateAttribute::OFF);

	_TextMessage->setFont(this->getInstallationDirectory() + "/" + OSG4WEB_LOADER_FONT);
	_TextMessage->setColor(OSG4WEB_COLORVEC_RED);
	_TextMessage->setCharacterSize(OSG4WEB_LOADER_FONT_SIZE);
	_TextMessage->setPosition(osg::Vec3(1280.0 / 2.0, 1024.0 / 3.3, 0.0));
        
	_TextMessage->setAlignment(osgText::Text::CENTER_BASE_LINE);
	_TextMessage->setFontResolution(OSG4WEB_LOADER_FONT_RES, OSG4WEB_LOADER_FONT_RES); 
        
	_TextMessage->setText("Initializing OSG4Web...");

	_TextMessage->setBackdropType(osgText::Text::OUTLINE);
    _TextMessage->setBackdropImplementation(osgText::Text::POLYGON_OFFSET);
    _TextMessage->setBackdropOffset(0.05f);
    _TextMessage->setBackdropColor(osg::Vec4(0.1f, 0.1f, 0.1f, 1.0f));

	tgeode->addDrawable(_TextMessage.get());

	_SwitchMessage->addChild(tgeode.get(), false);

	return _CameraNode.get();
}

osg::Node* Loader::createOSG4WebLogoText()
{
	this->sendNotifyMessage("createOSG4WebLogoText -> Creating OSG4Web Logo Text");

	osg::Geode* geode = new osg::Geode();
	osg::BoundingBox bb(osg::Vec3(0.0f,0.0f,0.0f),osg::Vec3(100.0f,100.0f,100.0f));

	osg::ref_ptr<osg::StateSet> stateset = geode->getOrCreateStateSet();
    stateset->setMode(GL_LIGHTING,osg::StateAttribute::OFF);

    std::string font(this->getInstallationDirectory() + "/" + OSG4WEB_LOADER_FONT);
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

bool Loader::setHUDMessage(std::string msg)
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

bool Loader::setHUDMessageColor(std::string colorname)
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

void Loader::preFrameUpdate() 
{
	osgViewer::Viewer::Windows windows;
	_Viewer->getWindows(windows);
	for(osgViewer::Viewer::Windows::iterator itr = windows.begin(); itr != windows.end(); ++itr)
	{
		int x, y, width, height;
		(*itr)->getWindowRectangle(x, y, width, height);

		if(width != _CurrWidth || height != _CurrHeight)
		{
			_CurrWidth = width;
			_CurrHeight = height;

			if(_CameraNode.valid())
				_CameraNode->setProjectionMatrixAsOrtho2D(0, _CurrWidth, 0, _CurrHeight);

			if(_TextMessage.valid())
				_TextMessage->setPosition(osg::Vec3(_CurrWidth / 2.0, _CurrHeight / 3.3, 0.0));
		}
	}
	
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
std::string Loader::handleAction(std::string argument)
{
	this->sendNotifyMessage("handleAction -> serving command: " + argument);

	std::string retstr = "CORE_DONE";
	std::string lcommand, rcommand;

	this->splitActionCommand(argument, lcommand, rcommand);
	
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
	default: //UNKNOWN_ACTION
		retstr = "UNKNOWN_ACTION";
		break;
	}
	
	return retstr;
}