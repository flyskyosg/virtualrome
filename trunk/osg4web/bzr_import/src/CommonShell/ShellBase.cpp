
#include <iostream>
#include <fstream>
#include <sstream>

#include <Utilities/StringUtils.h>
#include <Utilities/FileUtils.h>
#include <Utilities/SHA1.h>
#include <CommonCore/DynamicLoad.h>
#include <CommonCore/CoreInterface.h>
#include <CommonShell/ShellCommandParser.h>
#include <CommonShell/ShellBase.h>



ShellBase::ShellBase() : m_DynLoad(NULL),
	m_CoreInterface(NULL),
	m_loadcorename(OSG4WEB_LOADCORE_NAME),
	m_hWnd(NULL),
	m_eventfuncptr(NULL),
	m_resetwclassptr(NULL),
	m_prenderptr(NULL),
	m_crenderptr(NULL),
	m_dcoreptr(NULL),
	m_errorcode(0),
	m_UnRARList(NULL),
	m_doRender(false),
	m_coutbuf(NULL),
	m_cerrbuf(NULL),
	m_coreInit(false),
	m_fout(NULL),
	m_advancedcore("dummy_core_run")
{

}

ShellBase::~ShellBase()
{
	freeCompressedCore();

	if(this->isLogMessagesInitialized())
		if(! this->restoreLogMessages() )
			this->sendWarnMessage("ShellBase::~ShellBase -> Error closing messages redirection."); 
}

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

bool ShellBase::getInitOption(std::string name, std::string& value)
{
	value.clear(); //Resetto
	std::map<std::string, std::string>::iterator itr = m_initOptionsMap.begin();
	for( ; itr != m_initOptionsMap.end(); ++itr)
	{
		if( itr->first == name )
		{
			value = itr->second;
			return true;
		}
	}

	return false;
}

bool ShellBase::getObjectShellOption(std::string name, std::string& value)
{
	value.clear(); //Resetto
	std::map<std::string, std::string>::iterator itr = m_objectShellOptionsMap.begin();
	for( ; itr != m_objectShellOptionsMap.end(); ++itr)
	{
		if( itr->first == name )
		{
			value = itr->second;
			return true;
		}
	}

	return false;
}

bool ShellBase::configuringInitialOptions()
{
	std::string instdir, locinstdir, tempdir, coredir;

	//Se sono qui sicuramente ci sono le dir. Test skipped
	getInitOption(INIT_OPTION_INSTALLDIR, instdir); 
	getInitOption(INIT_OPTION_LOCALINSTDIR, locinstdir);
	getInitOption(INIT_OPTION_TEMPDIR, tempdir);
	getInitOption(INIT_OPTION_COREINSTDIR, coredir);

	if( !checkOrCreateDirectory(instdir) ||  !checkOrCreateDirectory(locinstdir) || !checkOrCreateDirectory(tempdir) || !checkOrCreateDirectory(coredir))
	{
		this->sendWarnMessage( "ShellBase::configuringInitialOptions -> makeDirectory failed!" );
		m_errorcode = 1;
		return false;
	}

	return true;
}

bool ShellBase::configuringObjectOptions()
{
	std::string temp;
	if(this->getObjectShellOption(OBJECT_OPTION_ENABLELOGS, temp)) //Se è presente l'indirizzo del core da caricare
	{
		if(!this->isLogMessagesInitialized())
			if(! this->initializeLogMessages(temp) )
				this->sendWarnMessage("ShellBase::ShellBase -> Error redirecting messages."); 
	}

	if(!this->getAdvancedCoreAddress().empty()) //Se è presente l'indirizzo del core da caricare
	{
		std::string fileName( this->getAdvancedCoreFileName() );

		//Trovo il nome della DLL da caricare
		std::string::size_type dotpos = fileName.find_last_of('.'); 
		if (dotpos == std::string::npos) 
			m_advancedcore = fileName;
		else
			m_advancedcore = std::string(fileName.begin(), fileName.begin() + dotpos);

		if(!this->getObjectShellOption(OBJECT_OPTION_ADVCORESHA1, temp)) //Controllo la presenza anche dello SHA1 del pacchetto Core
		{
			this->sendWarnMessage( "ShellBase::configuringObjectOptions -> Advanced Core SHA1 Option not set. Check your web page!" );
			return false;
		}

		if(this->getObjectShellOption(OBJECT_OPTION_ADVCOREDEP, temp)) //Se è presente il nome del pacchetto di dipendenze
		{
			if(!this->getObjectShellOption(OBJECT_OPTION_ADVCOREDEPSHA1, temp)) //Controllo se è stato settato lo sha1
			{
				this->sendWarnMessage( "ShellBase::configuringObjectOptions -> Advanced Core Dependancies SHA1 Option not set. Check your web page!" );
				return false;
			}
		}
	}

	return true;
}

std::string ShellBase::getEnvironmentTempDirectory()
{
	return m_Environm.getTempDirectory();
}

std::string ShellBase::getEnvironmentAppDirectory()
{
	return m_Environm.getAppDirectory();
}

//TODO: messaggi di errore
std::string ShellBase::execShellCommand(std::string str)
{
	std::string retstr;
	ShellCommandParser scommandp(str);

	if(!scommandp.isValidString())
		return "BAD_COMMAND";

	switch(scommandp.getShellCommand())
	{
	case ShellCommand::SC_RELOAD_LCORE: //TODO: finire il caricamento del base core
		{
			if(!m_loadcorename.empty())
			{
				if(this->isRunning())
					if(!this->closeAllLibraries())
						return "ERROR"; //TODO

				if(!this->startLoadingBaseCore())
					return "ERROR"; //TODO
				
				retstr = "DONE";
			}
			else
				retstr = "ERROR"; //TODO
		}
		break;
	case ShellCommand::SC_FORCE_COREADV_LOAD: //TODO: DA RIFARE... sono scritte solo CAZZATE
		{
			std::string advcoreaddress, advancedcorehash;
			if(!this->getAdvancedCoreSHA1().empty() || !this->getAdvancedCoreAddress().empty())
			{
				if(this->isRunning())
					if(!this->closeAllLibraries())
						return "ERROR"; //TODO
	
				if(!this->getAdvancedCoreSHA1().empty())
				{
					if(!this->initializeAdvancedCore())
					{
						//Reload LoadCore
						if(!this->startLoadingBaseCore())
						{
							//TODO:
							return false;
						}

						//Setting Error Message
						this->execCoreCommand("LOADCORE SETMESSAGE_COLOR LC_OSG_RED");
						this->execCoreCommand("LOADCORE SETMESSAGE Initialize Advanced Core Failed!");
					}
				}
				else
				{
					//Setting Error Message
					this->execCoreCommand("LOADCORE SETMESSAGE_COLOR LC_OSG_RED");
					this->execCoreCommand("LOADCORE SETMESSAGE Advanced Core SHA-1 HASH not set!");
				}

				retstr = "DONE";
			}
			else
			{
				retstr = "ERROR"; //TODO
			}
		}
		break;
	case ShellCommand::SC_SHELL_VERSION:
		{
			retstr = this->getShellVersion();
		}
		break;
	default:
		//FIXME: qua non ci deve finire
		retstr = "UNKNOWNCOMMAND";
		break;
	}

	return retstr;
}

std::string ShellBase::execCoreCommand(std::string str)
{
	if(m_coreInit)
		return m_CoreInterface->DoCommand(str);
	else
	{
		this->sendWarnMessage( "ShellBase::execCoreCommand -> core not initialized!" );
		m_errorcode = 10;
	}

	return std::string("CORENOTINITIALIZED");
}

bool ShellBase::doRendering()
{
	if(m_CoreInterface == NULL)
	{
		this->sendWarnMessage( "ShellBase::doRendering -> core interface not initialized!" );
		m_errorcode = 20;
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
		this->sendWarnMessage( "ShellBase::doRendering -> core not initialized!" );
		m_errorcode = 10;
	}
	
	this->sendWarnMessage( "ShellBase::doRendering -> doRendering Failed!" );
	return false;
}

bool ShellBase::startRendering()
{
	if(m_CoreInterface == NULL)
	{
		this->sendWarnMessage( "ShellBase::startRendering -> core interface not initialized!" );
		m_errorcode = 20;
		return false;
	}

	if(m_coreInit)
	{
		if(m_instanceclassptr != NULL && m_prenderptr != NULL)
		{
			typedef bool (prepareRenderingDefinition)(void*);
	
			prepareRenderingDefinition* prepareRendering;
			prepareRendering = (prepareRenderingDefinition*) m_prenderptr;
			
			if( prepareRendering(m_instanceclassptr) )
			{
				m_doRender = true;
				return true;
			}
			else
			{
				this->sendWarnMessage( "ShellBase::startRendering -> Start rendering failed!" );
				return false;
			}
		}
		else
		{
			this->sendWarnMessage( "ShellBase::startRendering -> Instance class pointer not initialized!" );
			m_errorcode = 11;
		}
	}
	else
	{
		this->sendWarnMessage( "ShellBase::startRendering -> core not initialized!" );
		m_errorcode = 10;
	}

	return false;
}

bool ShellBase::stopRendering()
{
	bool ret = true;

	if(m_CoreInterface == NULL)
	{
		this->sendWarnMessage( "ShellBase::stopRendering -> core interface not present!" );
		m_errorcode = 20;
		return false;
	}

	if(m_coreInit && !m_CoreInterface->isDone())
	{
		if(m_instanceclassptr != NULL && m_crenderptr != NULL)
		{
			typedef bool (deinitializeRenderingDefinition)(void*);
	
			deinitializeRenderingDefinition* deinitializeRendering;
			deinitializeRendering = (deinitializeRenderingDefinition*) m_crenderptr;
			
			if( deinitializeRendering(m_instanceclassptr) )
			{
				m_doRender = false;
			}
			else
			{
				this->sendWarnMessage( "ShellBase::stopRendering -> Stop rendering failed!" );
				return false;
			}
		}
		else
		{
			this->sendWarnMessage( "ShellBase::stopRendering -> Instance class pointer not initialized!" );
			m_errorcode = 12;

			return false;
		}

		//FIXME: Controllare che sia corretto
		if(m_instanceclassptr != NULL && m_resetwclassptr != NULL)
		{
			typedef bool (resetWindowHandlerDefinition)(void*);
	
			resetWindowHandlerDefinition* resetWindowHandler;
			resetWindowHandler = (resetWindowHandlerDefinition*) m_resetwclassptr;
			
			if( resetWindowHandler(m_instanceclassptr) )
			{
				//TODO: messaggi

				ret = true;
			}
			else
			{
				//TODO: messaggi

				return false;
			}
		}
		else
		{
			this->sendWarnMessage( "ShellBase::stopRendering -> Instance class pointer not initialized!" );
			m_errorcode = 12;

			return false;
		}
	}
	/*else  //Anche se non c'è me ne fotto
	{
		this->sendWarnMessage( "ShellBase::stopRendering -> core not initialized!" );
		m_errorcode = 10;

		return false;
	}*/

	return ret;
}

bool ShellBase::checkLoadCorePresence()
{
	std::string coredirectory;

	getInitOption(INIT_OPTION_LOCALINSTDIR , coredirectory); //FIXME: controllare se fare i test
	coredirectory = Utilities::FileUtils::convertFileNameToNativeStyle(coredirectory);

	//Check library presence
	std::string s = Utilities::FileUtils::convertFileNameToNativeStyle(
						coredirectory + "/" + 
						m_loadcorename + DEBUGAPPEND + 
						DynamicLoad::getLibraryExtension()
						);

	if(!Utilities::FileUtils::fileExists(s))
	{
		this->sendNotifyMessage( "ShellBase::checkLoadCorePresence-> LoadCore library not presentin local install dir. Serching in base install dir." );


		getInitOption(INIT_OPTION_INSTALLDIR , coredirectory); //FIXME: controllare se fare i test
		coredirectory = Utilities::FileUtils::convertFileNameToNativeStyle(coredirectory);

		//Check library presence
		s = Utilities::FileUtils::convertFileNameToNativeStyle(
						coredirectory + "/" + 
						m_loadcorename + DEBUGAPPEND + 
						DynamicLoad::getLibraryExtension()
						);

		if(!Utilities::FileUtils::fileExists(s))
		{
			this->sendWarnMessage( "ShellBase::checkLoadCorePresence-> LoadCore library not present!" );
			m_errorcode = 4;
			return false;
		}
	}

	if(!m_Environm.addDirectoryToPath(coredirectory))
	{
		this->sendWarnMessage( "ShellBase::checkLoadCorePresence-> error setting path env var!" );
		m_errorcode = 5;
		return false;
	}

	m_loadcoredir = coredirectory;

	return true;
}

std::string ShellBase::getAdvancedCoreSHA1()
{
	std::string temp;
	this->getObjectShellOption(OBJECT_OPTION_ADVCORESHA1, temp);
	return temp;
}

std::string ShellBase::getAdvancedCoreAddress() 
{ 
	std::string temp;
	this->getObjectShellOption(OBJECT_OPTION_ADVCORE, temp);
	return temp;
}

std::string ShellBase::getAdvancedCoreFileName() 
{ 
	return Utilities::FileUtils::getSimpleFileName(this->getAdvancedCoreAddress());
}

std::string ShellBase::getAdvancedCoreDirectory() 
{ 
	if( m_advancedcoredir.empty() ) 
	{
		std::string coredir;
		getInitOption(INIT_OPTION_COREINSTDIR, coredir); //FIXME: controllare se va fatto test

		m_advancedcoredir = Utilities::FileUtils::convertFileNameToNativeStyle(	coredir + "/" + this->getAdvancedCoreSHA1());
	}
	return m_advancedcoredir;
}

void ShellBase::setAdvancedCoreDirectory(std::string dir)
{
	this->setObjectShellOption(INIT_OPTION_COREINSTDIR, dir);
	m_advancedcoredir = dir;
}

bool ShellBase::startLoadingBaseCore()
{
	if(m_loadcorename.empty())
	{
		this->sendWarnMessage( "ShellBase::startLoadingBaseCore -> loadcore name not specified!" );
		m_errorcode = 2;
		return false;
	}

	if(m_hWnd == NULL)
	{
		this->sendWarnMessage( "ShellBase::startLoadingBaseCore-> window pointer not initialized!" );
		m_errorcode = 3;
		return false;
	}

	if(!this->checkLoadCorePresence())
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

	if( !this->loadDynamicCore(m_loadcorename + DEBUGAPPEND) )
	{
		this->sendWarnMessage( "ShellBase::startLoadingBaseCore -> runtime library loading failed!" );
		return false;
	}

	std::string logname;
	if(this->getObjectShellOption(OBJECT_OPTION_ENABLELOGS, logname)) //Se è presente l'indirizzo del core da caricare
		m_CoreInterface->forcingLogMessages();

	m_CoreInterface->setEventBridge(m_instanceclassptr, m_eventfuncptr);

	m_coreInit = m_CoreInterface->InitCore(m_hWnd, m_loadcoredir, this->getInitLoadCoreOptions());

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
		this->sendWarnMessage( "ShellBase::startLoadingBaseCore -> core not initialized!" );
		m_errorcode = 10;
		return false;
	}

	return true;
}

bool ShellBase::checkAdvCorePresence()
{
	std::string dir = getAdvancedCoreDirectory();

	if( !Utilities::FileUtils::fileExists(dir) )
	{
		if( !Utilities::FileUtils::makeDirectory(dir) )
		{
			this->sendWarnMessage( "ShellBase::checkAdvCorePresence-> makeDirectory failed!" );
			m_errorcode = 30; //FIXME: messaggio di errore al loadcore
			return false;
		}
	}

	std::string libraryfullpath(
		Utilities::FileUtils::convertFileNameToNativeStyle(
			dir + 
			"/" + 
			m_advancedcore + 
			DEBUGAPPEND + 
			DynamicLoad::getLibraryExtension()
			));
	
	//Check library presence
	if(!Utilities::FileUtils::fileExists(libraryfullpath))
	{
		//Non c'è in User Space, cerco anche nell'installazione locale
		this->sendNotifyMessage( "ShellBase::checkAdvCorePresence-> Advanced library is not present in user osg4web dir. Serching in firefox local install dir." );

		std::string coredirectory;
		getInitOption(INIT_OPTION_LOCALINSTDIR, coredirectory); //FIXME: controllare se fare i test
		coredirectory = Utilities::FileUtils::convertFileNameToNativeStyle(coredirectory + "/" + this->getAdvancedCoreSHA1());

		//Check library presence
		std::string s = Utilities::FileUtils::convertFileNameToNativeStyle(
						coredirectory + 
						"/" + 
						m_advancedcore + 
						DEBUGAPPEND + 
						DynamicLoad::getLibraryExtension()
						);

		if(!Utilities::FileUtils::fileExists(s))
		{
			//Non c'è in Firefox Local, cerco nell'installazione principale
			this->sendNotifyMessage( "ShellBase::checkAdvCorePresence-> Advanced library is not present in local install dir. Serching in base install dir." );

			getInitOption(INIT_OPTION_INSTALLDIR , coredirectory); //FIXME: controllare se fare i test
			coredirectory = Utilities::FileUtils::convertFileNameToNativeStyle(coredirectory + "/" + this->getAdvancedCoreSHA1());

			//Check library presence
			s = Utilities::FileUtils::convertFileNameToNativeStyle(
							coredirectory + 
							"/" + 
							m_advancedcore + 
							DEBUGAPPEND + 
							DynamicLoad::getLibraryExtension()
							);

			if(!Utilities::FileUtils::fileExists(s))
			{
				this->sendNotifyMessage( "ShellBase::checkAdvCorePresence-> Advanced library not found!" );
				return false;
			}
		
		}

		this->setAdvancedCoreDirectory(coredirectory);
	}
	
	return true;
}

bool ShellBase::initializeAdvancedCore()
{
	if(!this->getAdvancedCoreAddress().empty())
	{
		typedef bool (requestCoreDownloadingDefinition)(void*);
	
		requestCoreDownloadingDefinition* requestCoreDownloading;
		requestCoreDownloading = (requestCoreDownloadingDefinition*) m_dcoreptr;
			
		if( !requestCoreDownloading(m_instanceclassptr) )
		{
			//TODO: il thread non è partito
			this->sendWarnMessage( "ShellBase::initializeAdvancedCore-> request dowloading core message failed!" );
			m_errorcode = 31; //FIXME: messaggio di errore al loadcore
			return false;
		}
	}
	else
		return true;

	return true;
}

bool ShellBase::startLoadingAdvancedCore()
{
	bool ret = false;

	std::string advcoredir = getAdvancedCoreDirectory();

	std::string libraryfullpath(
		Utilities::FileUtils::convertFileNameToNativeStyle(
			advcoredir + 
			"/" + 
			m_advancedcore + 
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
	
	if( !this->loadDynamicCore(m_advancedcore + DEBUGAPPEND) )
	{
		this->sendWarnMessage( "ShellBase::startLoadingAdvancedCore -> runtime library loading failed!" );
		return false;
	}

	//TODO: reset di finestra

	std::string logname;
	if(this->getObjectShellOption(OBJECT_OPTION_ENABLELOGS, logname)) //Se è presente l'indirizzo del core da caricare
		m_CoreInterface->forcingLogMessages();

	m_CoreInterface->setEventBridge(m_instanceclassptr, m_eventfuncptr);
	
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
		this->sendWarnMessage( "ShellBase::startLoadingAdvancedCore -> core not initialized!" );
		m_errorcode = 10;
		return false;
	}

	//Applico la stringa di start se presente
	std::string startadvcoreopt;
	this->getObjectShellOption(OBJECT_OPTION_ADVCSTARTOPT, startadvcoreopt);
	if(!startadvcoreopt.empty())
		m_CoreInterface->AddStartOptions(startadvcoreopt);

	return ret;
}

bool ShellBase::loadDynamicCore(std::string loadcore)
{
	OSG4WebCC::createCoreInterfaceFunction* createClassInstance;

	std::string errmsg;
	m_DynLoad = DynamicLoad::loadLibrary(loadcore, errmsg);

	if(m_DynLoad == NULL)
	{
		this->sendWarnMessage( "ShellBase::loadDynamicCore -> loading runtime library failed! Error: " + errmsg );
		m_errorcode = 21;
		return false;
	}

	createClassInstance = (OSG4WebCC::createCoreInterfaceFunction*) m_DynLoad->getProcAddress("createClassInstance");
	
	if (createClassInstance == NULL) 
	{
		this->sendWarnMessage( "ShellBase::loadDynamicCore -> get library process address failed!" );
		m_errorcode = 22;
		return false;
	}

	m_CoreInterface = createClassInstance();

	if(m_CoreInterface == NULL)
	{
		this->sendWarnMessage( "ShellBase::loadDynamicCore -> class instance creation failed!" );
		m_errorcode = 23;
		return false;
	}

	return true;
}

bool ShellBase::deleteCurrentCore()
{
	if(m_DynLoad == NULL)
	{
		this->sendWarnMessage( "ShellBase::deleteCurrentCore -> dynamic library not present!" );
		m_errorcode = 24;
		return false;
	}

	OSG4WebCC::deleteCoreInterfaceFunction* deleteClassInstance;

	//Richiamo la funzione di distruzione della Classe Instanziata
	deleteClassInstance = (OSG4WebCC::deleteCoreInterfaceFunction*) m_DynLoad->getProcAddress("deleteClassInstance");
	if (deleteClassInstance == NULL) 
	{
		this->sendWarnMessage( "ShellBase::deleteCurrentCore -> error deleting class instance!" );
		m_errorcode = 25;
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

void ShellBase::setWindowHandler( HWND hwnd ) 
{ 
	m_hWnd = hwnd; 
}

std::string ShellBase::getShellVersion()	
{
	return std::string(SHELL_VERSION_STRING);
}

// Formattazione Stringa di Options al LoadCore:
// "Stringa arg di Object" "PROXY_HOSTNAME=url" <"PROXY_PORT=port">
std::string ShellBase::getInitLoadCoreOptions()
{
	std::string purl, pport;
	std::string retstr;

	this->getObjectShellOption(OBJECT_OPTION_LOADCOREOPT, retstr);

	if( getInitOption(INIT_OPTION_PROXYHNAME, purl) )
	{
		if(!retstr.empty())
		{
			retstr += " ";
		}

		retstr += "PROXY_HOSTNAME=";
		retstr += purl;
		
		if(!getInitOption(INIT_OPTION_PROXYHPORT, pport))
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

	this->getObjectShellOption(OBJECT_OPTION_ADVCINITOPT, retstr);

	//FIXME: rendere indipendente dal core
	if( getInitOption(INIT_OPTION_PROXYHNAME, purl) )
	{
		if(!retstr.empty())
		{
			retstr += " ";
		}

		retstr += "PROXY_HOSTNAME=";
		retstr += purl;
		
		if(!getInitOption(INIT_OPTION_PROXYHPORT, pport))
		{
			retstr += " PROXY_PORT=";
			retstr += pport;
		}
	}

	return retstr;
}

int ShellBase::openCompressedCore(std::string filename)
{
	int filecounter = 0;
	
	if(!filename.empty())
		m_TempArchive = Utilities::FileUtils::convertFileNameToNativeStyle(filename);
	else
	{
		this->sendWarnMessage( "ShellBase::openCompressedCore -> Filename Empty!" );
		return -1;
	}
	
	freeCompressedCore(); //Per sicurezza

	filecounter = urarlib_list((void*) m_TempArchive.c_str(), (ArchiveList_struct*)&m_UnRARList);

	return filecounter;
}

/*
 * 
 *
 *	Return: 0  -> OK and continue 
 *			>0 -> OK and finish
 *			<0 -> Error while UnPacking
 */
int ShellBase::unpackCompressedCoreFile()
{
	int ret = 1; //Return End
	char *data_ptr = NULL;
	unsigned long data_size = 0;  
	
	if(m_UnRARList != NULL)
	{
		if(urarlib_get(&data_ptr, &data_size, m_UnRARList->item.Name, (void*) m_TempArchive.c_str(), NULL)) //FIXME: Cosa fare con password?
		{
			if(m_UnRARList->item.FileAttr == 16)
			{
				Utilities::FileUtils::makeDirectory(Utilities::FileUtils::convertFileNameToNativeStyle(getAdvancedCoreDirectory() + "/" + std::string(m_UnRARList->item.Name)));
			}
			else if(!this->writeCoreFileToDisk(m_UnRARList->item.Name, data_ptr, data_size))
			{
				if(data_ptr != NULL) 
					free(data_ptr);

				this->sendWarnMessage( "ShellBase::unpackCompressedCoreFile -> Rar writing failed!" );

				return -2; //Return Writing Error
			}
	
			ret = 0; //Return Continue
		} 
		else 
		{ 
			if(data_ptr != NULL) 
				free(data_ptr);

			m_errorcode = 35;
			this->sendWarnMessage( "ShellBase::unpackCompressedCoreFile -> Rar open archive failed!" );

			return -1; //Return UnPacking Error
		}

		if(data_ptr != NULL) 
				free(data_ptr);
	
		m_UnRARList = (ArchiveList_struct*)m_UnRARList->next;
	}

	return ret;
}

bool ShellBase::writeCoreFileToDisk(char* filename, char* data, unsigned long datasize)
{
	bool ret = false;

	FILE* datafp;

	std::string completepath(Utilities::FileUtils::convertFileNameToNativeStyle(getAdvancedCoreDirectory() + "/" + std::string(filename)));

	if(!(((datafp = fopen(completepath.c_str(), "wb")) == NULL) || (data == NULL)))
	{ 
		unsigned long count = fwrite(data, 1, datasize, datafp);

		if(count != datasize)
		{
			m_errorcode = 34;
			this->sendWarnMessage( "ShellBase::writeCoreFileToDisk -> Writing File Failed!" );
		}

		fclose(datafp);

		ret = true;
	}
	else
	{
		m_errorcode = 32;
		this->sendWarnMessage( "ShellBase::writeCoreFileToDisk -> Open File Failed!" );
	}
	
	return ret;
}

void ShellBase::freeCompressedCore()
{
	if(m_UnRARList)
	{
		urarlib_freelist(m_UnRARList);
		m_UnRARList = NULL;
	}
}

bool ShellBase::checkOrCreateDirectory(std::string dir)
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

bool ShellBase::checkFileValidity(std::string fname, std::string hexhash)
{
	bool ret = false;
	bool opensucc = false;

	Utilities::CryptoAlg::SHA1 sha1filecont;

	sha1filecont.Reset();
	opensucc = sha1filecont.HashFile((char*) fname.c_str());
	sha1filecont.Final();

	std::string report = sha1filecont.ReportHash(Utilities::CryptoAlg::SHA1::REPORT_HEX);

	if(opensucc)
	{
		this->sendNotifyMessage( "ShellBase::checkFileValidity -> Control: " + hexhash);
		this->sendNotifyMessage( "ShellBase::checkFileValidity -> Generated: " + report);

		if(report == hexhash)
		{
			this->sendNotifyMessage( "ShellBase::checkFileValidity -> Package Valid!");
			ret = true;
		}
		else
		{
			m_errorcode = 33;
			this->sendWarnMessage( "ShellBase::checkFileValidity -> Validity Check Failed!" );
		}
	}
	else
	{
		m_errorcode = 32;
		this->sendWarnMessage( "ShellBase::checkFileValidity -> Open File Failed!" );
	}
	
	return ret;
}

std::string ShellBase::getErrorString()
{
	std::string retstr;

	switch(m_errorcode)
	{
	case 0:
		retstr = "ShellBase works correctly";
		break;
	case 1:
		retstr = "Making directory failed";
		break;
	case 2:
		retstr = "LoadCore name not present";
		break;
	case 3:
		retstr = "Window pointer not initialized";
		break;
	case 4:
		retstr = "LoadCore library not present";
		break;
	case 5:
		retstr = "Accessing environment vars failed";
		break;
	case 10:
		retstr = "Core not initialized";
		break;
	case 11:
		retstr = "Preparing core rendering process failed";
		break;
	case 12:
		retstr = "Deinitialize core rendering process failed";
		break;
	case 20:
		retstr = "Core Interface not initialized";
		break;
	case 21:
		retstr = "Loading Run-Time library failed";
		break;
	case 22:
		retstr = "Getting process library failed";
		break;
	case 23:
		retstr = "Creating library class instance failed";
		break;
	case 24:
		retstr = "Run-Time library not present";
		break;
	case 25:
		retstr = "Deleting library class instance failed";
		break;
	case 30:
		retstr = "Advanced Core not initialized";
		break;
	case 31:
		retstr = "Request dowloading core message failed";
		break;
	case 32:
		retstr = "Opening file failed";
		break;
	case 33:
		retstr = "Validity check failed";
		break;
	case 34:
		retstr = "Writing file failed";
		break;
	case 35:
		retstr = "Rar open archive failed!";
		break;
	default:
		retstr = "Unknown error - Default Message";
		break;
	}

	return retstr;
}