#include <string>
#include <iostream>

#include <CoreBase/CoreBase.h>

#include <osg/Node>
#include <osgDB/ReadFile>
#include <osgGA/GUIEventAdapter>
#include <osgUtil/Optimizer>




using namespace OSG4WebCC;


//CoreBase Costruttore
CoreBase::CoreBase(std::string corename) : CoreInterface(corename), CommandSchedule("COREBASE"),
		_Viewer(new osgViewer::Viewer),
		_Traits(new osg::GraphicsContext::Traits),
		_MainCamera(new osg::Camera),
		_LocalSceneGraph(new osg::Group),
		_TrackballManipulator(new osgGA::TrackballManipulator),
		_KeySwitchManipulator(new osgGA::KeySwitchMatrixManipulator),
		_WinData(NULL)
{
#if defined(_DEBUG)
	//Inizializzo il Debug
	if(! this->initializeLogMessages() )
		this->sendWarnMessage("CoreBase -> Error redirecting messages."); 

	this->sendNotifyMessage("CoreBase -> Starting Log Redirection.");
#endif

	this->sendNotifyMessage("CoreBase -> Costructing CoreBase Instance.");
}

//CoreBase Distruttore
CoreBase::~CoreBase()
{
	this->sendNotifyMessage("~CoreBase -> Destructing CoreBase Instance.");

	this->clearCommandRegistry();

	if(_Viewer.valid())
		_Viewer = NULL;

	if(_Traits.valid())
		_Traits = NULL;

	if(_MainCamera.valid())
		_MainCamera = NULL;

	if(_LocalSceneGraph.valid())
		_LocalSceneGraph = NULL;
	
	if(_TrackballManipulator.valid())
		_TrackballManipulator = NULL;

	if(_KeySwitchManipulator.valid())
		_KeySwitchManipulator = NULL;

	/*
#if defined(WIN32) //FIXME: cerca di risolvere il BUG dovuto all'utilizzo consequenziale dello stesso WinID
	osgViewer::GraphicsWindowWin32::WindowData *windowData = _WinData.get() ? dynamic_cast<osgViewer::GraphicsWindowWin32::WindowData*>(_WinData.get()) : 0;
    HWND hwnd = windowData ? windowData->_hwnd : 0;
	HDC hdc = ::GetDC( hwnd );

	::ReleaseDC(hwnd, hdc);
#else
	//TODO: Linux
#endif 
	*/

	if(_WinData.valid())
		_WinData = NULL;

#if defined(_DEBUG)
	this->sendNotifyMessage("~CoreBase -> Stopping Log Redirection.");

	if(! this->restoreLogMessages() )
		this->sendWarnMessage("~CoreBase -> Error closing messages redirection."); 
#endif
		
	//FIXME: Controllare il delete... aggiunto solo per fare testing
	//Killing Registry instance (ovviamente mi si inchioda tutto!!!)
	//osgDB::Registry::instance()->closeAllLibraries();
	//osgDB::Registry::instance(true);
}

void CoreBase::clearCommandRegistry() 
{ 
	_CommandRegistry.clear(); 
}

void CoreBase::addCommandSchedule(CommandSchedule* cschedule) 
{ 
	_CommandRegistry[cschedule->getCommandScheduleName()] = cschedule;
}


#if defined(WIN32)
bool CoreBase::InitCore(WINDOWIDTYPE mhWnd, std::string instdir, std::string options) 
{
	_InstDir = instdir; //Directory di installazione del Core attuale

	this->sendNotifyMessage("InitCore -> Starting Core Initialization.");

	//Descrivo il Pixel Format
	PIXELFORMATDESCRIPTOR pixelFormat =
	{
	    sizeof(PIXELFORMATDESCRIPTOR),
		1,
		PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER, 
		PFD_TYPE_RGBA,    
		24,
		0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0,
		24,
		0,
		0,
		PFD_MAIN_PLANE,
		0,
		0, 0, 0
	};

	//Retrieve DC Context
	HDC hdc = ::GetDC(mhWnd);
    if (hdc==0)
	{
		this->sendWarnMessage("InitCore -> GetDC Error.");
	    return false;
	}

	int pixelFormatIndex = ::ChoosePixelFormat(hdc, &pixelFormat);
	if (pixelFormatIndex==0)
	{
		this->sendWarnMessage("InitCore -> ChoosePixelFormat Error.");
	    ::ReleaseDC(mhWnd, hdc);
	    return false;
	}

	//Sets PixelFormat in Context
	if (!::SetPixelFormat(hdc, pixelFormatIndex, &pixelFormat))
	{
		this->sendWarnMessage("InitCore -> SetPixelFormat Error.");
	    ::ReleaseDC(mhWnd, hdc);
	    return false;
	}

	// Local Variable to hold window size data
	RECT rect;
	// Get the current window size
	::GetWindowRect(mhWnd, &rect);
	
	_WinRect[0] = rect.left;
	_WinRect[1] = rect.right;
	_WinRect[2] = rect.bottom;
	_WinRect[3] = rect.top;

	//Setto le proprietà di finestra
	if(!this->setupWindow(new osgViewer::GraphicsWindowWin32::WindowData(mhWnd)))
		return false;

	//Inizializzo le opzioni del core
	if(!this->initSetupOptions(options))
		return false;

	//Inizializzo i manipolatori
	if(!this->initManipulators())
		return false;

	//Configuro la camera
	if(!this->initCameraConfig())
		return false;

	//Creo e attacco lo scenegraph alla scena
	if(!this->initSceneData())
		return false;
	
	//Blocco il tasto Escape
	_Viewer->setKeyEventSetsDone(0);

	//Setto il resizer per il Viewer
	_Viewer->addEventHandler(new osgViewer::WindowSizeHandler);

	//OSG Realize
	_Viewer->realize();

	//Realize Test
	if(_Viewer->isRealized())
	{
		this->sendNotifyMessage("InitCore -> OSG Scene Realized.");
		return true;
	}
	else
	{
		this->sendWarnMessage("InitCore -> Realizing OSG Scene Failed.");
		return false;
	}
}
#else //Linux... TODO:
bool CoreBase::InitCore(Display*, WINDOWIDTYPE, std::string, std::string)
{
	//TODO: implement me!!!!
}
#endif

//Setta le opzioni del core successivamente alla inizializzazione
void CoreBase::AddStartOptions(std::string str, bool erase)
{
	this->sendNotifyMessage("AddStartOptions -> Adding Starting Options.");

	//Using: ADD_MODEL fileaddress
	this->setCommandAction("ADD_MODEL");
	//Using: ADDCLEAN_MODEL fileaddress
	this->setCommandAction("ADDCLEAN_MODEL");
	//Using: ADDSINGLE_MODEL fileaddress
	this->setCommandAction("ADDSINGLE_MODEL");
	//Using: VIEW_HOME
	this->setCommandAction("VIEW_HOME");

	this->addCommandSchedule((CommandSchedule*) this);
	
	this->DoCommand(str); //Passo la stringa a DoCommand che la gestisce
}

//Passata di Rendering
bool CoreBase::RenderScene()
{
	if(!_Viewer->done() && !this->isDone())
	{
		this->preFrameUpdate();
		_Viewer->frame();
		this->postFrameUpdate();
	}
	else
	{
		this->sendWarnMessage("RenderScene -> Core not initialized, Rendering skipped!");
		return false;
	}

	return true;
}

//Inizializza il manipolatore di scena
bool CoreBase::initManipulators()
{
	this->sendNotifyMessage("initManipulators -> Initializing Manipulators.");

	_KeySwitchManipulator->addMatrixManipulator( '1', "TrackballManipulator", _TrackballManipulator.get());
    _KeySwitchManipulator->selectMatrixManipulator(0);

	return true;
}

//Inizializzo la camera
bool CoreBase::initCameraConfig()
{
	this->sendNotifyMessage("initCameraConfig -> Initializing Camera Settings.");

	osg::DisplaySettings::instance()->setMinimumNumAlphaBits(8);
	osg::ref_ptr<osg::GraphicsContext> gc = osg::GraphicsContext::createGraphicsContext(_Traits.get());

	_MainCamera->setGraphicsContext(gc.get());
	_MainCamera->setViewport( new osg::Viewport( _Traits->x, _Traits->y, _Traits->width, _Traits->height ) );

	_Viewer->addEventHandler(new osgViewer::StatsHandler);
	_Viewer->addSlave(_MainCamera.get());
	_Viewer->setCameraManipulator(_KeySwitchManipulator.get());

	return true;
}

//Attacco l'albero di scena
bool CoreBase::initSceneData()
{
	this->sendNotifyMessage("initSceneData -> Building the SceneGraph.");

	_Viewer->setSceneData(_LocalSceneGraph.get());

	return true;
}

//Inizializzo le opzioni del core
bool CoreBase::initSetupOptions(std::string options, bool erase) //FIXME: finire le options... (Provare ad usare readNodeFunc in modo da non passare dal Registry)
{
	this->sendNotifyMessage("initSetupOptions -> setting init OSG Registry Options " + options);

	//std::string options("useOriginalExternalReferences noLoadExternalReferenceFiles");

	/*
	osg::ref_ptr<osgDB::ReaderWriter::Options> opt = osgDB::Registry::instance()->getOptions();
	if(!opt.valid())
	{
		opt = new osgDB::ReaderWriter::Options;
	}

	opt->setName("Global Loading Option");

	std::string option = opt->getOptionString();

	if(erase)
		opt->setOptionString(options);
	else
	{
		if(!option.empty())
			option += " ";

		option += options;
		opt->setOptionString(option);
	}
	
	osgDB::Registry::instance()->setOptions(opt.get()); 
	*/

	return true;
}

//Carica un modello nella scena. 
bool CoreBase::loadModel(std::string nodename, bool erase) //Erase non usato
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

//Setta le proprietà di finestra
bool CoreBase::setupWindow(osg::Referenced* wd)
{
	_WinData = wd;

	this->sendNotifyMessage("setupWindow -> Setting Window Properties.");

	// Setup the traits parameters
	_Traits->x = 0;
	_Traits->y = 0;

	//FIXME: così sarebbe corretto. Mi sfancula le proporzioni... sistemare
	
	_Traits->width = _WinRect[1] - _WinRect[0];
	_Traits->height = _WinRect[2] - _WinRect[3];
	

	//_Traits->width = 800;
	//_Traits->height = 600;

	_Traits->windowDecoration = false;
	_Traits->doubleBuffer = true;
	_Traits->vsync = true;
	_Traits->sharedContext = 0;
	_Traits->setInheritedWindowPixelFormat = true;
	_Traits->inheritedWindowData = _WinData;

	return true;
}

//Gestisce i comandi provenineti dalla Shell e da JavaScript 
std::string CoreBase::DoCommand(std::string command)
{
	std::string lcommand, rcommand;

	this->sendNotifyMessage("DoCommand -> Core Command: " + command);
	this->splitActionCommand(command, lcommand, rcommand);

	CommandSchedule* cs = _CommandRegistry[lcommand];
	
	if(cs)
		return cs->handleAction(rcommand);
	
	this->sendWarnMessage("DoCommand -> Command not Handled: " + lcommand + " Argument: " + rcommand);
	return "CORE_BADCOMMAND";
}

std::string CoreBase::handleAction(std::string argument)
{
	std::string retstr = "CORE_DONE";
	std::string lcommand, rcommand;

	this->splitActionCommand(argument, lcommand, rcommand);
	this->sendNotifyMessage("handleAction -> Command Found");

	switch(this->getCommandActionIndex(lcommand))
	{
	case 0: //ADD_MODEL fileaddress
		if( !this->loadModel(rcommand, false) )
			retstr = "CORE_FAILED";
		break;
	case 1: //ADDCLEAN_MODEL fileaddress
		if( !this->loadModel(rcommand, true) )
			retstr = "CORE_FAILED";
		break;
	case 2: //ADDSINGLE_MODEL fileaddress
		if( !this->loadModel(rcommand) )
			retstr = "CORE_FAILED";
		break;
	case 3: //VIEW_HOME
			_Viewer->home();
		break;
	default: //UNKNOWN_CORE_COMMAND
		retstr = "UNKNOWN_CORE_COMMAND";
		break;
	}
	
	return retstr;
}

//setta osgViewer a Done
void CoreBase::setDone() 
{ 
	if(_Viewer.valid())
		_Viewer->setDone(true); 
}

//Ritorna lo stato di osgViewer
bool CoreBase::isDone() 
{ 
	if(_Viewer.valid())
		return _Viewer->done(); 

	return false;
}

