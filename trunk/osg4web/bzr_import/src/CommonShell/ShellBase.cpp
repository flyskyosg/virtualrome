#include <iostream>
#include <fstream>
#include <sstream>

#include <Utilities/StringUtils.h>
#include <Utilities/FileUtils.h>
#include <CommonCore/DynamicLoad.h>
#include <CommonCore/CoreInterface.h>
#include <CommonShell/ShellCommandParser.h>
#include <CommonShell/ShellBase.h>


/************************************************************************
 *
 * Files Functions
 *
 ************************************************************************/

ShellBase::ShellBase() : m_DynLoad(NULL),
	m_CoreInterface(NULL),
	m_hWnd(NULL),
	m_eventfuncptr(NULL),
	m_resetwclassptr(NULL),
	m_errorcode(0),
	m_doRender(false),
	m_coutbuf(NULL),
	m_cerrbuf(NULL),
	m_coreInit(false),
	m_fout(NULL),
	m_Downloader(NULL),
	m_Renderer(NULL)
{
	//Initialize Shell Downloader Thread
	m_Downloader = new ShellDownloader( this );
	//Set Cores Default Name
	m_generalOption.setShellOption(LOADER_CORENAME, LOADER_CORE_DEFAULT_NAME);
	m_generalOption.setShellOption(ADV_CORE_CORENAME, ADVANCED_CORE_DEFAULT_NAME);
	//Initialize Logs
	initializeErrorMessages();
}

ShellBase::~ShellBase()
{
	m_initOption.clearOption();
	m_generalOption.clearOption();
	m_objectOption.clearOption();

	if(m_Renderer)
	{
		delete m_Renderer;
		m_Renderer = NULL;
	}

	if(m_Downloader)
	{
		m_Downloader->closeDownloader();
		delete m_Downloader;
		m_Downloader = NULL;
	}

	if(this->isLogMessagesInitialized())
		if(! this->restoreLogMessages() )
			this->sendWarnMessage("ShellBase::~ShellBase -> Error closing messages redirection."); 
}


/************************************************************************
 *
 * Logs Functions
 *
 ************************************************************************/

void ShellBase::initializeLog(std::string logname)
{
#if defined(_DEBUG)
	if(! this->initializeLogMessages(logname) )
		this->sendWarnMessage("ShellBase::ShellBase -> Error redirecting messages."); 
#endif
}

void ShellBase::sendWarnMessage(std::string warnmessage)
{
	if(m_fout != NULL)
		std::cerr.rdbuf( m_fout->rdbuf() );
	
	std::cerr << "ERROR: "<< warnmessage << std::endl;
}

void ShellBase::sendNotifyMessage(std::string notifymessage)
{
	if(m_fout != NULL)
		std::cout.rdbuf( m_fout->rdbuf() );
	
	std::cout << "NOTIFY: " << notifymessage << std::endl;
}

bool ShellBase::initializeLogMessages(std::string logname)
{
	m_coutbuf = std::cout.rdbuf();
	m_cerrbuf = std::cerr.rdbuf();
	
	std::string fname(logname + "_log_");

	std::ostringstream convstream;
	convstream << this << std::flush;
	fname += convstream.str();

	fname += "_ID.txt";
    
	m_fout = new std::ofstream( fname.c_str() );
	
	if(m_fout->fail())
	{
		delete m_fout;
		m_fout = NULL;
		return false;
	}

	std::cout.rdbuf( m_fout->rdbuf() );
	std::cerr.rdbuf( m_fout->rdbuf() );

	return true;
}

bool ShellBase::restoreLogMessages()
{
	if(m_fout)
	{
		m_fout->close();
	
		delete m_fout;

		std::cout.rdbuf( m_coutbuf );
		std::cout.rdbuf( m_cerrbuf );

		return true;
	}

	return false;
}


/************************************************************************
 *
 * Exec Command Functions
 *
 ************************************************************************/

std::string ShellBase::execShellCommand(std::string str)
{
	std::string retstr;
	ShellCommandParser scommandp(str);

	if(!scommandp.isValidString())
		return "BAD_COMMAND";

	switch(scommandp.getShellCommand())
	{
	case ShellCommand::SC_RELOAD_LCORE:
		{
			//TODO:
			retstr = "IMPLEMENT ME";
		}
		break;
	case ShellCommand::SC_FORCE_COREADV_LOAD:
		{
			//TODO:
			retstr = "IMPLEMENT ME";
		}
		break;
	case ShellCommand::SC_SHELL_VERSION:
		{
			retstr = this->getShellVersion();
		}
		break;
	default:
		//FIXME: qua non ci deve finire
		retstr = "UNKNOWN_COMMAND";
		break;
	}

	return retstr;
}

std::string ShellBase::execCoreCommand(std::string str)
{
	std::string ret("CORENOTINITIALIZED");

	if(m_coreInit && m_Renderer)
	{
		//Requesting Mutual Exclusion
		m_Renderer->requestExclusion();
		ret = m_CoreInterface->DoCommand(str);
		m_Renderer->releaseExclusion();
	}
	else
	{
		this->setErrorCode( 10 );
		this->sendWarnMessage( "ShellBase::execCoreCommand -> " + this->getErrorString() );
	}

	return ret;
}


/************************************************************************
 *
 * Shell and Core Option Functions
 *
 ************************************************************************/

// Formattazione Stringa di Options al Loader:
// "Stringa arg di Object" "PROXY_HOSTNAME=url" <"PROXY_PORT=port">
std::string ShellBase::getInitLoaderOptions()
{
	std::string purl, pport;
	std::string retstr;

	m_objectOption.getShellOption(OBJECT_OPTION_LOADEROPT, retstr);

	if( m_initOption.getShellOption(INIT_OPTION_PROXYHNAME, purl) )
	{
		if(!retstr.empty())
		{
			retstr += " ";
		}

		retstr += "PROXY_HOSTNAME=";
		retstr += purl;
		
		if( m_initOption.getShellOption(INIT_OPTION_PROXYHPORT, pport) )
		{
			retstr += " PROXY_PORT=";
			retstr += pport;
		}
	}

	return retstr;
}

// Formattazione Stringa di Options a AdvancedCore:
// "Stringa arg di Object" "PROXY_HOSTNAME=url" <"PROXY_PORT=port">
std::string ShellBase::getInitAdvancedCoreOptions()
{
	std::string purl, pport;
	std::string retstr;

	m_objectOption.getShellOption(OBJECT_OPTION_ADVCINITOPT, retstr);

	//FIXME: rendere indipendente dal core
	if( m_initOption.getShellOption(INIT_OPTION_PROXYHNAME, purl) )
	{
		if(!retstr.empty())
		{
			retstr += " ";
		}

		retstr += "PROXY_HOSTNAME=";
		retstr += purl;
		
		if( m_initOption.getShellOption(INIT_OPTION_PROXYHPORT, pport) )
		{
			retstr += " PROXY_PORT=";
			retstr += pport;
		}
	}

	return retstr;
}

bool ShellBase::configuringInitialOptions()
{
	std::string instdir, locinstdir, tempdir, coredir;

	//Se sono qui sicuramente ci sono le dir. Test skipped
	m_initOption.getShellOption(INIT_OPTION_INSTALLDIR, instdir); 
	m_initOption.getShellOption(INIT_OPTION_LOCALINSTDIR, locinstdir);
	m_initOption.getShellOption(INIT_OPTION_TEMPDIR, tempdir);
	m_initOption.getShellOption(INIT_OPTION_COREINSTDIR, coredir);

	if( !getOrCreateDirectory(instdir) ||  !getOrCreateDirectory(locinstdir) || !getOrCreateDirectory(tempdir) || !getOrCreateDirectory(coredir))
	{
		this->setErrorCode( 1 );
		this->sendWarnMessage( "ShellBase::configuringInitialOptions -> " + this->getErrorString());
		return false;
	}

	return true;
}

bool ShellBase::configuringObjectOptions()
{
	//Sets 
	std::string temp;
	if(m_objectOption.getShellOption(OBJECT_OPTION_ENABLELOGS, temp)) //Se è presente l'indirizzo del core da caricare
	{
		if(!this->isLogMessagesInitialized())
			if(! this->initializeLogMessages(temp) )
				this->sendWarnMessage("ShellBase::ShellBase -> Error redirecting messages."); 
	}

	if(!m_objectOption.getShellOption(OBJECT_OPTION_ADVCORE).empty()) //Se è presente l'indirizzo del core da caricare
	{
		//Setto il nome del core advanced
		std::string fileName( Utilities::FileUtils::getSimpleFileName(m_objectOption.getShellOption(OBJECT_OPTION_ADVCORE) ) );

		//Trovo il nome della DLL da caricare
		std::string::size_type dotpos = fileName.find_last_of('.'); 
		if (dotpos == std::string::npos) 
			m_generalOption.setShellOption( ADV_CORE_CORENAME, fileName );
		else
		{
			std::string newname = std::string(fileName.begin(), fileName.begin() + dotpos);
			m_generalOption.setShellOption( ADV_CORE_CORENAME, newname);
		}

		//Setto la directory del core
		std::string coredir = m_initOption.getShellOption(INIT_OPTION_COREINSTDIR); //FIXME: controllare se va fatto test
		std::string newcoredir = Utilities::FileUtils::convertFileNameToNativeStyle( coredir + "/" + m_objectOption.getShellOption(OBJECT_OPTION_ADVCORESHA1));

		m_generalOption.setShellOption(ADV_CORE_MAINCOREDIR, newcoredir);
	
		if(!m_objectOption.getShellOption(OBJECT_OPTION_ADVCORESHA1, temp)) //Controllo la presenza anche dello SHA1 del pacchetto Core
		{
			this->sendWarnMessage( "ShellBase::configuringObjectOptions -> Advanced Core SHA1 Option not set. Check your web page!" );

			//Setting Error Message
			this->execCoreCommand("LOADER SETMESSAGE_COLOR LC_OSG_RED");
			this->execCoreCommand("LOADER SETMESSAGE Configuring Shell Options Failed!");

			return false;
		}

		if(m_objectOption.getShellOption(OBJECT_OPTION_ADVCOREDEP, temp)) //Se è presente il nome del pacchetto di dipendenze
		{
			//Setto la directory delle dipendenze
			std::string newdepcoredir = Utilities::FileUtils::convertFileNameToNativeStyle(	coredir + "/" + CORE_DEPENDANCIES_DIR + "/" + m_objectOption.getShellOption(OBJECT_OPTION_ADVCOREDEPSHA1));
			m_generalOption.setShellOption(ADV_CORE_DEPCOREDIR, newdepcoredir);

			if(!m_objectOption.getShellOption(OBJECT_OPTION_ADVCOREDEPSHA1, temp)) //Controllo se è stato settato lo sha1
			{
				this->sendWarnMessage( "ShellBase::configuringObjectOptions -> Advanced Core Dependancies SHA1 Option not set. Check your web page!" );

				//Setting Error Message
				this->execCoreCommand("LOADER SETMESSAGE_COLOR LC_OSG_RED");
				this->execCoreCommand("LOADER SETMESSAGE Configuring Shell Options Failed!");

				return false;
			}
		}
	}
	else //Carica solo il core di loading
	{
		//TODO:
	}


	/*  FIXME: controllo ridondante

	//Return if CoreString is inizialized and SHA-1 isn't
	std::string test;
	if(m_objectOption.getShellOption(OBJECT_OPTION_ADVCORE, test) && !m_objectOption.getShellOption(OBJECT_OPTION_ADVCORESHA1, test))
	{
		//Setting Error Message
		this->execCoreCommand("LOADER SETMESSAGE_COLOR LC_OSG_RED");
		this->execCoreCommand("LOADER SETMESSAGE Advanced Core SHA-1 HASH not set!");

		return false;
	}

	*/

	return true;
}


bool ShellBase::checkLoaderPresence()
{
	this->sendNotifyMessage( "ShellBase::checkLoaderPresence-> Checking loading core library");
	std::string coredirectory;

	m_initOption.getShellOption(INIT_OPTION_LOCALINSTDIR , coredirectory); //FIXME: controllare se fare i test
	coredirectory = Utilities::FileUtils::convertFileNameToNativeStyle(coredirectory);

	//Check library presence
	std::string s = Utilities::FileUtils::convertFileNameToNativeStyle(
						coredirectory + "/" + 
						m_generalOption.getShellOption(LOADER_CORENAME) + DEBUGAPPEND + 
						DynamicLoad::getLibraryExtension()
						);

	if(!Utilities::FileUtils::fileExists(s)) 
	{
		this->sendNotifyMessage( "ShellBase::checkLoaderPresence-> Loader Core library not found in local install dir. Serching in base install dir." );

		m_initOption.getShellOption(INIT_OPTION_INSTALLDIR , coredirectory); //FIXME: controllare se fare i test
		coredirectory = Utilities::FileUtils::convertFileNameToNativeStyle(coredirectory);

		//Check library presence
		s = Utilities::FileUtils::convertFileNameToNativeStyle(
						coredirectory + "/" + 
						m_generalOption.getShellOption(LOADER_CORENAME) + DEBUGAPPEND + 
						DynamicLoad::getLibraryExtension()
						);

		if(!Utilities::FileUtils::fileExists(s))
		{
			this->setErrorCode( 4 );
			this->sendWarnMessage( "ShellBase::checkLoaderPresence-> " + this->getErrorString() );
			return false;
		}
	}

	if(!m_Environm.addDirectoryToPath(coredirectory))
	{
		this->setErrorCode( 5 );
		this->sendWarnMessage( "ShellBase::checkLoaderPresence-> " + this->getErrorString() );
		return false;
	}

	m_generalOption.setShellOption(LOADER_COREDIR, coredirectory);

	return true;
}

bool ShellBase::checkAdvCoreDependaciesPresence()
{
	this->sendNotifyMessage( "ShellBase::checkAdvCoreDependaciesPresence-> Checking dependacies directory");		
	std::string dir = m_generalOption.getShellOption(ADV_CORE_DEPCOREDIR);

	if( !Utilities::FileUtils::fileExists(dir) )
	{
		dir = m_initOption.getShellOption(INIT_OPTION_LOCALINSTDIR);
		dir = Utilities::FileUtils::convertFileNameToNativeStyle(dir + "/" + CORE_DEPENDANCIES_DIR + "/" + m_objectOption.getShellOption(OBJECT_OPTION_ADVCOREDEPSHA1));
	
		if( !Utilities::FileUtils::fileExists(dir) )
		{
			this->sendNotifyMessage( "ShellBase::checkAdvCoreDependaciesPresence-> Dependancies directory not found");
			return false;
		}

		//Setting new directory found
		m_generalOption.setShellOption(ADV_CORE_DEPCOREDIR, dir);
	}

	return true;

}

bool ShellBase::checkAdvCorePresence()
{
	this->sendNotifyMessage( "ShellBase::checkAdvCorePresence-> Checking advanced core library");
	std::string dir = m_generalOption.getShellOption(ADV_CORE_MAINCOREDIR); //Prendo dir del core advanced

	if( !Utilities::FileUtils::fileExists(dir) )
	{
		if( !Utilities::FileUtils::makeDirectory(dir) )
		{
			this->setErrorCode( 30 ); //FIXME: messaggio di errore al core loader
			this->sendWarnMessage( "ShellBase::checkAdvCorePresence-> " + this->getErrorString() );
			return false;
		}
	}

	std::string libraryfullpath(
		Utilities::FileUtils::convertFileNameToNativeStyle(
			dir + 
			"/" + 
			m_generalOption.getShellOption(ADV_CORE_CORENAME) + 
			DEBUGAPPEND + 
			DynamicLoad::getLibraryExtension()
			));
	
	//Check library presence
	if(!Utilities::FileUtils::fileExists(libraryfullpath))
	{
		//Non c'è in User Space, cerco anche nell'installazione locale
		this->sendNotifyMessage( "ShellBase::checkAdvCorePresence-> Advanced library is not present in user osg4web dir. Serching in firefox local install dir." );

		std::string coredirectory = m_initOption.getShellOption(INIT_OPTION_LOCALINSTDIR);
		coredirectory = Utilities::FileUtils::convertFileNameToNativeStyle(coredirectory + "/" + m_objectOption.getShellOption(OBJECT_OPTION_ADVCORESHA1));

		//Check library presence
		std::string s = Utilities::FileUtils::convertFileNameToNativeStyle(
						coredirectory + 
						"/" + 
						m_generalOption.getShellOption(ADV_CORE_CORENAME) + 
						DEBUGAPPEND + 
						DynamicLoad::getLibraryExtension()
						);

		if(!Utilities::FileUtils::fileExists(s))
		{
			//Non c'è in Firefox Local, cerco nell'installazione principale
			this->sendNotifyMessage( "ShellBase::checkAdvCorePresence-> Advanced library is not present in local install dir. Serching in base install dir." );

			m_initOption.getShellOption(INIT_OPTION_INSTALLDIR , coredirectory); //FIXME: controllare se fare i test
			coredirectory = Utilities::FileUtils::convertFileNameToNativeStyle(coredirectory + "/" + m_objectOption.getShellOption(OBJECT_OPTION_ADVCORESHA1));

			//Check library presence
			s = Utilities::FileUtils::convertFileNameToNativeStyle(
							coredirectory + 
							"/" + 
							m_generalOption.getShellOption(ADV_CORE_CORENAME) + 
							DEBUGAPPEND + 
							DynamicLoad::getLibraryExtension()
							);

			if(!Utilities::FileUtils::fileExists(s))
			{
				this->sendNotifyMessage( "ShellBase::checkAdvCorePresence-> Advanced library not found!" );
				return false;
			}
		
		}

		//Setting new directory found
		m_generalOption.setShellOption(ADV_CORE_MAINCOREDIR, coredirectory);
	}
	
	return true;
}

bool ShellBase::initializeAdvancedCore()
{
	this->sendNotifyMessage("ShellBase::initializeAdvancedCore -> configuring Object Core Options.");
	if(!this->configuringObjectOptions())
		return false;

	if(!m_objectOption.getShellOption(OBJECT_OPTION_ADVCORE).empty())
	{
		ShellDownloader::RequestDownload request;
		std::string dlname, tempdir, unpackdirectory;

		tempdir = m_initOption.getShellOption( INIT_OPTION_TEMPDIR );

		//If core dependancies are present request is queued
		if(!m_objectOption.getShellOption(OBJECT_OPTION_ADVCOREDEP).empty())
		{
			this->sendNotifyMessage("ShellBase::initializeAdvancedCore -> advanced core dependacies needed.");

			dlname = Utilities::FileUtils::getSimpleFileName( m_objectOption.getShellOption(OBJECT_OPTION_ADVCOREDEP) );
			unpackdirectory = m_generalOption.getShellOption( ADV_CORE_DEPCOREDIR );

			request.setDownloadURL(m_objectOption.getShellOption(OBJECT_OPTION_ADVCOREDEP), ShellDownloader::RequestDownload::DEPCORE_FILE );
			request.setSecurityString(m_objectOption.getShellOption(OBJECT_OPTION_ADVCOREDEPSHA1));
			request.setDownloadTempDir( tempdir );
			request.setUnpackDirectory( unpackdirectory );
			
			this->sendNotifyMessage("ShellBase::initializeAdvancedCore -> requesting advanced core dependacies.");

			m_Downloader->addDownloadRequest(request);

			request.clear();
		}
	
		dlname = Utilities::FileUtils::getSimpleFileName( m_objectOption.getShellOption(OBJECT_OPTION_ADVCORE) );
		unpackdirectory = m_generalOption.getShellOption( ADV_CORE_MAINCOREDIR );

		request.setDownloadURL(m_objectOption.getShellOption(OBJECT_OPTION_ADVCORE), ShellDownloader::RequestDownload::CORE_FILE );
		request.setSecurityString(m_objectOption.getShellOption(OBJECT_OPTION_ADVCORESHA1));
		request.setDownloadTempDir( tempdir );
		request.setUnpackDirectory( unpackdirectory );

		this->sendNotifyMessage("ShellBase::initializeAdvancedCore -> requesting advanced core.");
		m_Downloader->addDownloadRequest(request);
	}
	else
	{
		this->sendNotifyMessage("ShellBase::initializeAdvancedCore -> advanced core not requested .");
		return true; //TODO: controllare se qua ci vanno messaggi di errore
	}

	return true;
}

bool ShellBase::startDownloadedCore(bool timing)
{
	//FIXME: trovare il modo di non usare la sleep direttamente
	if(timing)
		PR_Sleep(LOADING_CORE_MESSAGES_MIN);

	//Attivo status bar
	this->execCoreCommand("LOADER STATUSBAR_VISIBILITY FALSE");

	//Attivo il messaggio di Download del Core
	this->execCoreCommand("LOADER SETMESSAGE_COLOR LC_OSG_BLUE");
	this->execCoreCommand("LOADER SETMESSAGE Core Found");

	if(timing)
		PR_Sleep(LOADING_CORE_MESSAGES_MID);

	//Attivo il messaggio di Download del Core
	this->execCoreCommand("LOADER SETMESSAGE_COLOR LC_OSG_BLUE");
	this->execCoreCommand("LOADER SETMESSAGE Core Loading ...");

	if(timing)
		PR_Sleep(LOADING_CORE_MESSAGES_MID);

	//Loading Session
	this->sendNotifyMessage(std::string("ShellBase::startDownloadedCore -> Loading Unpacked Core"));

	if(!this->startLoadingAdvancedCore()) //Start Advanced Core Loading
	{
		this->sendWarnMessage(std::string("ShellBase::startDownloadedCore -> Loading advanced core failed! Starting Loading Core"));

		//Reload Loader
		if(!this->startLoadingBaseCore())
		{
			this->sendWarnMessage(std::string("ShellBase::startDownloadedCore -> Starting Loading Core failed... BYE BYE..."));
			return false;
		}

		//Setting Error Message
		this->execCoreCommand("LOADER SETMESSAGE_COLOR LC_OSG_RED");
		this->execCoreCommand("LOADER SETMESSAGE Loading Advanced Core Failed!");
	}
	else
		this->sendNotifyMessage(std::string("ShellBase::startDownloadedCore -> Advanced Core is up and running"));

	return true;
}





/************************************************************************
 *
 * Starting Downloaded Cores (Loader Core / AdvancedCore)
 *
 ************************************************************************/

bool ShellBase::startLoadingBaseCore()
{
	if(m_generalOption.getShellOption(LOADER_CORENAME).empty())
	{
		this->setErrorCode( 2 );
		this->sendWarnMessage( "ShellBase::startLoadingBaseCore -> " + this->getErrorString() );
		return false;
	}

	if(m_hWnd == NULL)
	{
		this->setErrorCode( 3 );
		this->sendWarnMessage( "ShellBase::startLoadingBaseCore-> " + this->getErrorString() );
		return false;
	}

	if(!this->checkLoaderPresence())
	{
		//Messaggi già impostati in checkload
		return false;
	}

	if(this->isRunning())
		if(!this->closeAllLibraries())
		{
			this->sendWarnMessage( "ShellBase::startLoadingBaseCore -> closing previous running core failed!" );
			return false;
		}

	if( !this->loadDynamicCore(m_generalOption.getShellOption(LOADER_CORENAME) + DEBUGAPPEND) )
	{
		this->sendWarnMessage( "ShellBase::startLoadingBaseCore -> runtime library loading failed!" );
		return false;
	}

	std::string logname;
	if(m_objectOption.getShellOption(OBJECT_OPTION_ENABLELOGS, logname)) //Se è presente l'indirizzo del core da caricare
		m_CoreInterface->forcingLogMessages();

	m_CoreInterface->setEventBridge(m_instanceclassptr, m_eventfuncptr, &ShellBase::requestFileDownload);

	m_coreInit = m_CoreInterface->InitCore(m_hWnd, m_generalOption.getShellOption(LOADER_COREDIR), this->getInitLoaderOptions());

	if(m_coreInit)
	{
		if(!startRendering())
		{
			this->sendWarnMessage( "ShellBase::startLoadingBaseCore -> startRendering failed!" );
			return false;
		}
	}
	else
	{
		this->setErrorCode( 10 );
		this->sendWarnMessage( "ShellBase::startLoadingBaseCore -> " + this->getErrorString() );
		return false;
	}

	return true;
}

bool ShellBase::startLoadingAdvancedCore()
{
	bool ret = false;

	std::string advcoredir = m_generalOption.getShellOption(ADV_CORE_MAINCOREDIR);
	std::string advcoredepdir = m_generalOption.getShellOption(ADV_CORE_DEPCOREDIR);

	std::string libraryfullpath(
		Utilities::FileUtils::convertFileNameToNativeStyle(
			advcoredir + 
			"/" + 
			m_generalOption.getShellOption(ADV_CORE_CORENAME) + 
			DEBUGAPPEND + 
			DynamicLoad::getLibraryExtension()
			));

	//Check Advanced Core library presence
	if(!Utilities::FileUtils::fileExists(libraryfullpath))
	{
		this->sendWarnMessage( "ShellBase::startLoadingAdvancedCore-> Advanced Core library seems to be not present" );
		return ret;
	}

	if(!m_Environm.restoreOriginalPath())
	{
		this->sendWarnMessage( "ShellBase::startLoadingAdvancedCore -> error setting initial path env var!" );
		return false;
	}

	if(!advcoredepdir.empty()) //Setting dependancies directory if presents
	{
		if(!m_Environm.addDirectoryToPath(advcoredepdir))
		{
			this->sendWarnMessage( "ShellBase::startLoadingAdvancedCore -> error setting path env var!" );
			return false;
		}
	}

	if(!m_Environm.addDirectoryToPath(advcoredir))
	{
		this->sendWarnMessage( "ShellBase::startLoadingAdvancedCore -> error setting path env var!" );
		return false;
	}

	if( !this->closeAllLibraries() )
	{
		this->sendWarnMessage( "ShellBase::startLoadingAdvancedCore -> closing runtime libraries failed!" );
		return false;
	}
	
	if( !this->loadDynamicCore(m_generalOption.getShellOption(ADV_CORE_CORENAME) + DEBUGAPPEND) )
	{
		this->sendWarnMessage( "ShellBase::startLoadingAdvancedCore -> runtime library loading failed!" );
		return false;
	}

	//TODO: reset di finestra

	std::string logname;
	if(m_objectOption.getShellOption(OBJECT_OPTION_ENABLELOGS, logname)) //Se è presente l'indirizzo del core da caricare
		m_CoreInterface->forcingLogMessages();

	m_CoreInterface->setEventBridge(m_instanceclassptr, m_eventfuncptr, &ShellBase::requestFileDownload);
	
	m_coreInit = m_CoreInterface->InitCore(m_hWnd, advcoredir, this->getInitAdvancedCoreOptions());

	if(m_coreInit)
	{
		if(!startRendering())
		{
			this->sendWarnMessage( "ShellBase::startLoadingAdvancedCore -> startRendering failed!" );
			return false;
		}
		else 
			ret = true;
	}
	else
	{
		this->setErrorCode( 10 );
		this->sendWarnMessage( "ShellBase::startLoadingAdvancedCore -> " + this->getErrorString() );
		return false;
	}

	//Applico la stringa di start se presente
	std::string startadvcoreopt = m_objectOption.getShellOption(OBJECT_OPTION_ADVCSTARTOPT);
	if(!startadvcoreopt.empty())
		m_CoreInterface->AddStartOptions(startadvcoreopt);

	return ret;
}


/************************************************************************
 *
 * Handle Dynamic Core Libraries
 *
 ************************************************************************/

bool ShellBase::loadDynamicCore(std::string core)
{
	CommonCore::createCoreInterfaceFunction* createClassInstance;

	std::string errmsg;
	m_DynLoad = DynamicLoad::loadLibrary(core, errmsg);

	if(m_DynLoad == NULL)
	{
		this->setErrorCode( 21 );
		this->sendWarnMessage( "ShellBase::loadDynamicCore -> " + this->getErrorString() + " Error: " + errmsg );
		return false;
	}

	createClassInstance = (CommonCore::createCoreInterfaceFunction*) m_DynLoad->getProcAddress("createClassInstance");
	
	if (createClassInstance == NULL) 
	{
		this->setErrorCode( 22 );
		this->sendWarnMessage( "ShellBase::loadDynamicCore -> " + this->getErrorString() );
		return false;
	}

	m_CoreInterface = createClassInstance();

	if(m_CoreInterface == NULL)
	{
		this->setErrorCode( 23 );
		this->sendWarnMessage( "ShellBase::loadDynamicCore -> " + this->getErrorString() );
		return false;
	}

	return true;
}

bool ShellBase::deleteCurrentCore()
{
	if(m_DynLoad == NULL)
	{
		this->setErrorCode( 24 );
		this->sendWarnMessage( "ShellBase::deleteCurrentCore -> " + this->getErrorString() );
		return false;
	}

	CommonCore::deleteCoreInterfaceFunction* deleteClassInstance;

	//Richiamo la funzione di distruzione della Classe Instanziata
	deleteClassInstance = (CommonCore::deleteCoreInterfaceFunction*) m_DynLoad->getProcAddress("deleteClassInstance");
	if (deleteClassInstance == NULL) 
	{
		this->setErrorCode( 25 );
		this->sendWarnMessage( "ShellBase::deleteCurrentCore -> " + this->getErrorString() );
		return false;
	}
	
	//Uccido la classe Core
	deleteClassInstance(m_CoreInterface);
	m_CoreInterface = NULL;

	//Deinizializzo la variabile di controllo
	m_coreInit = false;
	
	//Pulisco il LoaderDinamico
	delete m_DynLoad;

	return true;
}

bool ShellBase::closeAllLibraries()
{
	if(!this->stopRendering())
	{
		this->sendWarnMessage( "ShellBase::closeAllLibraries -> stoprendering failed!" );
		return false;
	}

	if(!this->deleteCurrentCore())
	{
		this->sendWarnMessage( "ShellBase::closeAllLibraries -> deletecurrentcore failed!" );
		return false;
	}

	return true;
}


/************************************************************************
 *
 * Files Functions
 *
 ************************************************************************/

bool ShellBase::getOrCreateDirectory(std::string dir)
{
	if(!Utilities::FileUtils::fileExists(dir))
		if(!Utilities::FileUtils::makeDirectory(dir))
			return false;

	return true;
}

bool ShellBase::checkFileExistance(std::string fname)
{
	return Utilities::FileUtils::fileExists(fname);
}

bool ShellBase::removingFile(std::string fname)
{
	return Utilities::FileUtils::fileDeletes(fname);
}


/************************************************************************
 *
 * Error Functions
 *
 ************************************************************************/

std::string ShellBase::getErrorString(unsigned int no)
{
	this->sendNotifyMessage("ShellBase::getErrorString -> getting error by index: " );

	if(no < m_errormessage.size())
	{
		if(!m_errormessage.at(no).empty())
			return m_errormessage.at(no);
	}

	this->sendWarnMessage("ShellBase::getErrorString -> error code not presents or messages not sets, sending default message" );
	
	return m_errormessage.at(0);
}

std::string ShellBase::getErrorString()
{
	this->sendNotifyMessage("ShellBase::getErrorString -> getting error by errorcode: " ); //TODO: aggiungere il codice d'errore

	return getErrorString(m_errorcode);
}

void ShellBase::initializeErrorMessages()
{
	this->sendNotifyMessage("ShellBase::initializeErrorMessages -> initializing error messages" );

	m_errormessage.push_back("Unknown error - Default Message"); //0
	m_errormessage.push_back("Making directory failed"); //1
	m_errormessage.push_back("Loader Core name not present"); //2
	m_errormessage.push_back("Window pointer not initialized"); //3
	m_errormessage.push_back("Loader Core library not present"); //4
	m_errormessage.push_back("Accessing environment vars failed"); //5
	m_errormessage.push_back(std::string()); //6
	m_errormessage.push_back(std::string()); //7
	m_errormessage.push_back(std::string()); //8
	m_errormessage.push_back(std::string()); //9
	m_errormessage.push_back("Core not initialized"); //10
	m_errormessage.push_back("Preparing core rendering process failed"); //11
	m_errormessage.push_back("Deinitialize core rendering process failed"); //12
	m_errormessage.push_back(std::string()); //13
	m_errormessage.push_back(std::string()); //14
	m_errormessage.push_back(std::string()); //15
	m_errormessage.push_back(std::string()); //16
	m_errormessage.push_back(std::string()); //17
	m_errormessage.push_back(std::string()); //18
	m_errormessage.push_back(std::string()); //19
	m_errormessage.push_back("Core Interface not initialized"); //20
	m_errormessage.push_back("Loading Run-Time library failed"); //21
	m_errormessage.push_back("Getting process library failed"); //22
	m_errormessage.push_back("Creating library class instance failed"); //23
	m_errormessage.push_back("Run-Time library not present"); //24
	m_errormessage.push_back("Deleting library class instance failed"); //25
	m_errormessage.push_back(std::string()); //26
	m_errormessage.push_back(std::string()); //27
	m_errormessage.push_back(std::string()); //28
	m_errormessage.push_back(std::string()); //29
	m_errormessage.push_back("Advanced Core not initialized"); //30
	m_errormessage.push_back("Request dowloading core message failed"); //31
	m_errormessage.push_back("Opening file failed"); //32
	m_errormessage.push_back("Validity check failed"); //33
	m_errormessage.push_back("Writing file failed"); //34
	m_errormessage.push_back("Rar open archive failed!"); //35
	m_errormessage.push_back(std::string()); //36
	m_errormessage.push_back(std::string()); //37
	m_errormessage.push_back(std::string()); //38
	m_errormessage.push_back(std::string()); //39
	m_errormessage.push_back("retrieve proxy info failed!"); //40
	m_errormessage.push_back("retrieve plugin directory failed!"); //41
	m_errormessage.push_back("retrieve local plugin directory failed!"); //42
	m_errormessage.push_back("retrieve temp directory failed!"); //43
	m_errormessage.push_back("retrieve core directory failed!"); //44
	m_errormessage.push_back("window pointer is not valid!"); //45
	m_errormessage.push_back("window pointer is not present!"); //46
	m_errormessage.push_back(std::string()); //47
	m_errormessage.push_back(std::string()); //48
	m_errormessage.push_back(std::string()); //49
}


/************************************************************************
 *
 * Rendering Functions
 *
 ************************************************************************/

void ShellBase::requestExplicitRendering()
{
	if(m_coreInit && m_Renderer)
	{
		//Requesting Mutual Exclusion
		m_Renderer->requestExclusion();
		if(m_coreInit)
		{
			if(!m_CoreInterface->isDone())
				m_CoreInterface->RenderScene();

		}
		m_Renderer->releaseExclusion();
	}
}

bool ShellBase::startRendering()
{
	if(m_CoreInterface == NULL)
	{
		this->setErrorCode( 20 );
		this->sendWarnMessage( "ShellBase::startRendering -> " + this->getErrorString() );
		return false;
	}

	if(m_coreInit)
	{
		m_Renderer = new ShellRenderer(this);
		m_doRender = true;
		return true;
	}
	else
	{
		this->setErrorCode( 10 );
		this->sendWarnMessage( "ShellBase::startRendering -> " + this->getErrorString() );
	}

	return false;
}

bool ShellBase::stopRendering()
{
	bool ret = true;

	if(m_CoreInterface == NULL)
	{
		this->setErrorCode( 20 );
		this->sendWarnMessage( "ShellBase::stopRendering -> " + this->getErrorString());
		return false;
	}

	if(m_coreInit && !m_CoreInterface->isDone())
	{
		//Fermo il thread di Download se sta facendo cose particolari
		m_Downloader->clearDownloadRequestQueue();

		if( !m_Renderer )
		{
			this->sendWarnMessage( "ShellBase::stopRendering -> Renderer seems not presents!" );
			return false;
		}

		if( !m_Renderer->closeRendering() )
		{
			this->sendWarnMessage( "ShellBase::stopRendering -> Closing renderer failed!" );
			return false;
		}

		delete m_Renderer;
		m_Renderer = NULL;

		m_doRender = false;
	
		//FIXME: DA FINIRE... forse da togliere
		if(m_instanceclassptr != NULL && m_resetwclassptr != NULL)
		{
			typedef bool (resetWindowHandlerDefinition)(void*);
	
			resetWindowHandlerDefinition* resetWindowHandler;
			resetWindowHandler = (resetWindowHandlerDefinition*) m_resetwclassptr;
			
			if( resetWindowHandler(m_instanceclassptr) )
			{
				//TODO: messages

				ret = true;
			}
			else
			{
				//TODO: messages

				return false;
			}
		}
		else
		{
			this->setErrorCode( 12 );
			this->sendWarnMessage( "ShellBase::stopRendering -> " + this->getErrorString() );

			return false;
		}
	}

	return ret;
}

bool ShellBase::doRenderingBridge()
{
	if(m_CoreInterface == NULL)
	{
		this->setErrorCode( 20 );
		this->sendWarnMessage( "ShellBase::doRendering -> " + this->getErrorString() );
		return false;
	}

	if(m_coreInit)
	{
		if(!m_CoreInterface->isDone())
			if(m_CoreInterface->RenderScene())
				return true;

	}
	else
	{
		this->setErrorCode( 10 );
		this->sendWarnMessage( "ShellBase::doRendering -> " + this->getErrorString() );
	}
	
	this->sendWarnMessage( "ShellBase::doRendering -> doRendering Failed!" );
	return false;
}


bool ShellBase::setDoneBridge()
{
	if(m_CoreInterface)
	{
		m_CoreInterface->setDone();
		return true;
	}

	return false;
}


/*********************************************************************************
 *
 * Download Functions
 *
 *********************************************************************************/

void ShellBase::startDownloading(bool timing)
{
	this->sendNotifyMessage(std::string("ShellBase::startDownloading -> starting Download."));

	//Attivo status bar
	this->execCoreCommand("LOADER STATUSBAR_COLOR LC_OSG_BLUE");
	this->execCoreCommand("LOADER STATUSBAR_VISIBILITY TRUE");
	
	//Attivo il messaggio di Download del Core
	this->execCoreCommand("LOADER SETMESSAGE_COLOR LC_OSG_BLUE");
	this->execCoreCommand("LOADER SETMESSAGE Downloading Files...");
}

void ShellBase::checkSecurityString(bool timing)
{
	this->sendNotifyMessage(std::string("ShellBase::checkSecurityString -> checking validity string."));

	this->execCoreCommand("LOADER STATUSBAR_VISIBILITY FALSE");
	this->execCoreCommand("LOADER SETMESSAGE Checking Validity...");
}

void ShellBase::startUnPackSession(bool timing)
{
	this->sendNotifyMessage(std::string("ShellBase::startUnPackSession -> checking validity string."));

	//Unpacking Session
	this->execCoreCommand("LOADER STATUSBAR_COLOR LC_OSG_GREEN");
	this->execCoreCommand("LOADER STATUSBAR_VISIBILITY TRUE");
	this->execCoreCommand("LOADER STATUSBARVALUE 0"); //Reset Status Bar Lenght
	
	//Attivo il messaggio di Unpack del Core
	this->execCoreCommand("LOADER SETMESSAGE_COLOR LC_OSG_GREEN");
	this->execCoreCommand("LOADER SETMESSAGE Unpacking Files...");
}

void ShellBase::downloadFinished(bool timing)
{
	this->sendNotifyMessage(std::string("ShellBase::downloadFinished -> Loading Unpacked Core."));

	//Loading Session
	this->execCoreCommand("LOADER STATUSBAR_VISIBILITY FALSE");
	this->execCoreCommand("LOADER SETMESSAGE "); //Spengo i messaggi

	if(timing)
		PR_Sleep(LOADING_CORE_MESSAGES_MIN);
}

void ShellBase::downloadCoreFileFinished(std::string tempfile, bool timing)
{
	this->sendNotifyMessage(std::string("ShellBase::downloadFinished -> Loading Unpacked Core."));

	//Loading Session
	this->execCoreCommand("LOADER STATUSBAR_VISIBILITY FALSE");
	this->execCoreCommand("LOADER SETMESSAGE "); //Spengo i messaggi

	//Send Load information at the Core
	this->execCoreCommand("LOADER LOADREQFILE " + tempfile);

	if(timing)
		PR_Sleep(LOADING_CORE_MESSAGES_MIN);
}

void ShellBase::downloadError(int error)
{
	std::string errmsg;

	if(error == 0) //TODO: spostare nel errormessage 
		errmsg = "Download error";
	else if(error == -1)
		errmsg = "URL not specified";
	else if(error == -2)
		errmsg = "HTTP response error";
	else if(error == -3)
		errmsg = "Error writing cache";

	this->sendWarnMessage(std::string("ShellBase::downloadError -> Downloading File Failed: Error: " + errmsg));

	this->execCoreCommand("LOADER STATUSBAR_VISIBILITY FALSE");
	this->execCoreCommand("LOADER SETMESSAGE_COLOR LC_OSG_RED");
	this->execCoreCommand("LOADER SETMESSAGE Downloading Failed!");
}

void ShellBase::securityStringError()
{
	this->sendWarnMessage(std::string("ShellBase::securityStringError -> check validity failed!."));

	this->execCoreCommand("LOADER STATUSBAR_VISIBILITY FALSE");

	this->execCoreCommand("LOADER SETMESSAGE_COLOR LC_OSG_RED");
	this->execCoreCommand("LOADER SETMESSAGE Validity Control Failed!");
}

void ShellBase::unpackError(int error)
{
	std::string errmsg;

	if(error < 0) //TODO: spostare nei messaggi di errore e gestirli tutti  
		errmsg = "Unpacking Failed! Error opening archive";
	else if(error == 0)
		errmsg = "Unpacking Failed! Empty archive";

	this->sendWarnMessage(std::string("ShellBase::unpackError -> Unpacking Error. CODE: " + errmsg));

	this->execCoreCommand("LOADER STATUSBAR_VISIBILITY FALSE");

	this->execCoreCommand("LOADER SETMESSAGE_COLOR LC_OSG_RED");
	this->execCoreCommand("LOADER SETMESSAGE Unpacking " + errmsg);
}

int ShellBase::doProgressDLStatus(double downtot, double downnow, double ultotal, double ulnow)
{
	std::string statmsg("LOADER STATUSBAR_VALUE ");

	std::ostringstream convstream; //creates an ostringstream object
	convstream << ( downnow / downtot ) << std::flush;
	statmsg += convstream.str(); 

	this->execCoreCommand(statmsg);
		
	return 0;
}

int ShellBase::doProgressUnpackStatus(unsigned int cicleno, int filenumber)
{
	std::string statmsg("LOADER STATUSBAR_VALUE ");

	double dcicl = cicleno;
	double dfnum = filenumber;

	std::ostringstream convstream; //creates an ostringstream object
	convstream << ( dcicl /  dfnum ) << std::flush;
	statmsg += convstream.str();

	this->execCoreCommand(statmsg);

	return 0;
}


/****************************************************************************
 *
 *		RequestDownload from Core
 *
 ****************************************************************************/

bool ShellBase::requestFileDownload(void* classptr, std::string url)
{
	ShellBase* classinst = (ShellBase*) classptr;

	if(classinst)
		return classinst->setDownloadRequest(url);

	return false;
}


bool ShellBase::setDownloadRequest(std::string url)
{
	this->sendNotifyMessage("ShellBase::setDownloadRequest -> requesting file: " + url);

	ShellDownloader::RequestDownload request;
	std::string tempdir, dlname;

	tempdir = m_initOption.getShellOption( INIT_OPTION_TEMPDIR );

	dlname = Utilities::FileUtils::getSimpleFileName( m_objectOption.getShellOption(OBJECT_OPTION_ADVCORE) );

	request.setDownloadURL(url);
	request.setDownloadTempDir( tempdir );

	m_Downloader->addDownloadRequest(request);

	return true;
}

/****************************************************************************
 *
 *		Usato da IE
 *
 ****************************************************************************/
std::string ShellBase::getEnvironmentTempDirectory()
{
	return m_Environm.getTempDirectory();
}

std::string ShellBase::getEnvironmentAppDirectory()
{
	return m_Environm.getAppDirectory();
}