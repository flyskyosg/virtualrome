#include <string>
#include <iostream>

#include <CoreBase/CoreBase.h>

#include <osg/Node>
#include <osgDB/ReadFile>
#include <osgGA/GUIEventAdapter>
#include <osgGA/StateSetManipulator>
#include <osgGA/AnimationPathManipulator>
#include <osgUtil/Optimizer>




using namespace CommonCore;


//CoreBase Costruttore
CoreBase::CoreBase(std::string corename) : CoreInterface(corename), CommandSchedule("COREBASE"),
		_Viewer(new osgViewer::Viewer),
		_Traits(new osg::GraphicsContext::Traits),
		_MainCamera(new osg::Camera),
		_LocalSceneGraph(new osg::Group),
		_TrackballManipulator(new osgGA::TrackballManipulator),
		_KeySwitchManipulator(new osgGA::KeySwitchMatrixManipulator),
		_WinData(NULL),
		_CurrWidth(550), 
		_CurrHeight(550)
{
#if defined(_DEBUG)
	//Inizializzo il Debug
	if(! this->initializeLogMessages() )
		this->sendWarnMessage("CoreBase -> Error redirecting messages."); 

	this->sendNotifyMessage("CoreBase -> Starting Log Redirection.");
#endif

	this->sendNotifyMessage("CoreBase -> Costructing CoreBase Instance.");

	_InstDir.clear();
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

#if defined(WIN32)

#else
	//TODO: Linux
#endif

	if(_WinData.valid())
		_WinData = NULL;

	this->sendNotifyMessage("~CoreBase -> Stopping Log Redirection if Present.");

	if(this->isLogMessagesInitialized())
		if(! this->restoreLogMessages() )
			this->sendWarnMessage("~CoreBase -> Error closing messages redirection."); 
		
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

	// Local Variable to hold window size data
	RECT rect;
	// Get the current window size
	::GetWindowRect(mhWnd, &rect);
	
	int ww, hh;
	ww = rect.right - rect.left; 
	hh = rect.bottom - rect.top;

	if(ww != 0)
		_CurrWidth = ww;

	if(hh != 0)
		_CurrHeight = hh;
	 
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

	//Inizializzo gli Handler globali
	if(!this->initSceneHandlers())
		return false;

	//Creo e attacco lo scenegraph alla scena
	if(!this->initSceneData())
		return false;
	
	//Blocco il tasto Escape
	_Viewer->setKeyEventSetsDone(0);

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

	//Default value
	this->setCommandAction("UNKNOWN_ACTION");
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
	
	double old_fov, old_ratio, old_zNear, y_zFar;

	osg::DisplaySettings::instance()->setMinimumNumAlphaBits(8);
	osg::ref_ptr<osg::GraphicsContext> gc = osg::GraphicsContext::createGraphicsContext(_Traits.get());

	_MainCamera->setGraphicsContext(gc.get());
	_MainCamera->setViewport( new osg::Viewport( _Traits->x, _Traits->y, _Traits->width, _Traits->height ) );

	_Viewer->setCameraManipulator(_KeySwitchManipulator.get());

	_Viewer->addSlave(_MainCamera.get()); //, osg::Matrix::scale(osg::Vec3(ratio, 1.0, 1.0)), osg::Matrix::identity()); //TIPS: altro modo per fare la reprojection di camera

	//TIPS: Lo accetta solo in questo modo. Se specifico la projection prima di fare slave non funziona
	_Viewer->getCamera()->getProjectionMatrixAsPerspective(old_fov, old_ratio, old_zNear, y_zFar);
	_Viewer->getCamera()->setProjectionMatrixAsPerspective(old_fov, ((double) _CurrWidth) / ((double) _CurrHeight), old_zNear, y_zFar);
        
    return true;
}

//Inizializzo gli Handler di Scena
bool CoreBase::initSceneHandlers()
{
	// add the stats handler
	_Viewer->addEventHandler(new osgViewer::StatsHandler);

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
bool CoreBase::initSetupOptions(std::string options, bool erase)
{
	this->sendNotifyMessage("initSetupOptions -> setting up init Core Option");

	if(!options.empty())
	{
		this->sendNotifyMessage("initSetupOptions -> setting init OSG Registry Options " + options);

		std::string left, var, args, right = options;

		do
		{
			//Split left right
			std::string::size_type pos = right.find( ' ' );

			if(pos == std::string::npos)
			{
				left = right;
				right.clear();
			}
			else
			{
				left = right.substr(0, pos);
				right = right.substr(pos + 1, right.size() - 1);
			}

			//Split var args
			pos = left.find( '=' );
			if(pos != std::string::npos)
			{
				var = left.substr(0, pos);
				args = left.substr(pos + 1, left.size() - 1);

				if(var == "PROXY_HOSTNAME")
				{
					this->sendNotifyMessage("initSetupOptions -> proxy hostname found!");

					if( !(putenv((char*) (std::string("OSG_PROXY_HOST=") + args).c_str())))
						this->sendWarnMessage("initSetupOptions -> Can't set OSG_PROXY_HOST env variable");
					}
				else if(var == "PROXY_PORT")
				{
					this->sendNotifyMessage("initSetupOptions -> proxy port found!");

					if( !(putenv((char*) (std::string("OSG_PROXY_PORT=") + args).c_str())))
						this->sendWarnMessage("initSetupOptions -> Can't set OSG_PROXY_PORT env variable ");
				}
			}
		} while( !right.empty() );
	}
	
	//TODO:

	return true;


	//FIXME: finire le options... (Provare ad usare readNodeFunc in modo da non passare dal Registry)
	/*
	osg::ref_ptr<osgDB::ReaderWriter::Options> opt = osgDB::Registry::instance()->getOptions();
	if(!opt.valid())
	{
		opt = new osgDB::ReaderWriter::Options;
		opt->ref(); //FIXME: FACCIO REF PER LA FAVA DELLA DEALLOCAZIONE... IN QUESTO MODO NON VENGONO PIU' UCCISE LE DLL
	}
	
	opt->setName("Global_Registry_Option");

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
	_Traits->width = _CurrWidth;
	_Traits->height = _CurrHeight;
	
	_Traits->windowDecoration = false;
	_Traits->doubleBuffer = true;
	_Traits->vsync = true;
	_Traits->sharedContext = 0;
	_Traits->setInheritedWindowPixelFormat = true;
	_Traits->inheritedWindowData = _WinData;
	//_Traits->pbuffer = true;

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
	case ADD_MODEL: //ADD_MODEL fileaddress
		if( !this->loadModel(rcommand, false) )
			retstr = "CORE_FAILED";
		break;
	case ADDCLEAN_MODEL: //ADDCLEAN_MODEL fileaddress
		if( !this->loadModel(rcommand, true) )
			retstr = "CORE_FAILED";
		break;
	case ADDSINGLE_MODEL: //ADDSINGLE_MODEL fileaddress
		if( !this->loadModel(rcommand) )
			retstr = "CORE_FAILED";
		break;
	case VIEW_HOME: //VIEW_HOME
			_Viewer->home();
		break;
	default: //UNKNOWN_ACTION
		retstr = "UNKNOWN_ACTION";
		break;
	}
	
	return retstr;
}

//setta osgViewer a Done
void CoreBase::setDone() 
{ 
	if(_Viewer.valid())
	{
		_Viewer->setDone(true); 
		Sleep(1000);
		_Viewer->stopThreading();
	}
}

//Ritorna lo stato di osgViewer
bool CoreBase::isDone() 
{ 
	if(_Viewer.valid())
		return _Viewer->done(); 

	return false;
}

