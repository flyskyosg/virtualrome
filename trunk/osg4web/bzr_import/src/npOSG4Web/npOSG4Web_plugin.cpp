
#if defined(WIN32)
	#include <windows.h>
	#include <windowsx.h>
#else // unix
 	#include <unistd.h>
    #include <X11/Xlib.h>
	#include <X11/Intrinsic.h>
	#include <X11/cursorfont.h>
#endif

#include <sstream>
#include <string>

#include "nsISupportsUtils.h" // some usefule macros are defined here
#include "nsXPCOM.h"
#include "nsIFile.h"
#include "nsStringAPI.h"
#include "nsDirectoryServiceDefs.h"
#include "nsIServiceManager.h"
#include "nsIPrefService.h"
#include "nsCOMPtr.h"
#include "nsIProperties.h"
#include "npapi.h"

//cURL downloading library
#include <curl/curl.h>

#include <npOSG4Web/npOSG4Web_Defines.h>
#include <npOSG4Web/npOSG4Web_plugin.h>


/////////////////////////////////////
//
// Global Structures
//

//Thread locking state
static PRLock* s_ThreadLocking = NULL;

//Downloading Core Thread locking state
static PRLock* s_DlCoreThreadLocking = NULL;


//Main Error Message
std::string s_PluginMessageError;

/////////////////////////////////////
//
// general initialization and shutdown
//
NPError NS_PluginInitialize()
{
	s_ThreadLocking = PR_NewLock();
	assert(s_ThreadLocking);

	s_DlCoreThreadLocking = PR_NewLock();
	assert(s_DlCoreThreadLocking);

  	return NPERR_NO_ERROR;
}

void NS_PluginShutdown()
{
	if (s_ThreadLocking)
	{
		PR_DestroyLock(s_ThreadLocking);
		s_ThreadLocking = NULL;
	}

	if (s_DlCoreThreadLocking)
	{
		PR_DestroyLock(s_DlCoreThreadLocking);
		s_DlCoreThreadLocking = NULL;
	}
}


#if defined(XP_UNIX)
//////////////////////////////////////
//
// return MIME Description
//
char* NPP_GetMIMEDescription(void)
{
	return(MIME_TYPES_DESCRIPTION);
}

//////////////////////////////////////
//
// return Plug-In values
//
NPError NS_PluginGetValue(NPPVariable aVariable, void *aValue)
{
  	NPError err = NPERR_NO_ERROR;
  	
	switch (aVariable) 
	{
    	case NPPVpluginNameString:
     		*((char **)aValue) = PLUGIN_NAME;
     	 	break;
    	case NPPVpluginDescriptionString:
      		*((char **)aValue) = PLUGIN_DESCRIPTION;
      		break;
    	default:
      		err = NPERR_INVALID_PARAM;
      	break;
  	}
  	
	return err;
}
#endif

/////////////////////////////////////////////////////////////
//
// construction and destruction of our plugin instance object
//
nsPluginInstanceBase * NS_NewPluginInstance(nsPluginCreateData * aCreateDataStruct)
{
  	if(!aCreateDataStruct)
    		return NULL;

  	nsPluginInstance * plugin = new nsPluginInstance(aCreateDataStruct->instance);

	//FIXME: togliere  
	//NPN_SetValue(aCreateDataStruct->instance, NPPVpluginWindowBool, NULL);
	
  	NPError rv = plugin->SetInitialData(aCreateDataStruct->argc, aCreateDataStruct->argn, aCreateDataStruct->argv);

  	if(rv != NS_OK)
		return NULL;
  
  	return plugin;
}

//////////////////////////////////////
//
// destroy Plug-In Base
//
void NS_DestroyPluginInstance(nsPluginInstanceBase * aPlugin)
{
  	if(aPlugin)
    		delete (nsPluginInstance *)aPlugin;
}

////////////////////////////////////////
//
// nsPluginInstance class implementation
//
nsPluginInstance::nsPluginInstance(NPP aInstance) : nsPluginInstanceBase(),
  	mInstance(aInstance),
  	mInitialized(false),
	mScriptablePeer(NULL),
	mADVCore(NULL),
	mADVCoreSHA1Hash(NULL),
	mADVCoreDep(NULL),
	mADVCoreDepSHA1Hash(NULL),
	mADVInitOptions(new std::string( "useOriginalExternalReferences noLoadExternalReferenceFiles" )),
	mADVStartOptions(NULL),
	mLoadCBOptions(new std::string( "useOriginalExternalReferences noLoadExternalReferenceFiles" )),
	mInstDir(new std::string()),
	mTempDir(new std::string()),
	mCoreInstallDir(new std::string()),
	mThread(NULL),
	mShutdownThread(false),
	mDlCoreThread(NULL),
	mDlCoreShutdownThread(false),
	mLoading(false),
#if defined(WIN32)
#if defined(RENDER_WITH_TIMER_EVENT)
	mTimer(RENDER_DELAY),
#endif
	lpOldProc(NULL),
  	mhWnd(NULL)
#else
	//TODO:linux
	//FIXME: da rifare completamente
  	mhWnd(0),
  	mXtwidget(0),
  	mFontInfo(0)
#endif
{
	mShellBase.initializeLog("npOSG4Web");

	mShellBase.sendNotifyMessage("nsPluginInstance::nsPluginInstance -> Default Constructor");
	mShellBase.sendNotifyMessage("nsPluginInstance::nsPluginInstance -> Discovering Firefox settings");

	nsresult rv = getProxySettings(mProxyHName, mProxyPort);
	if(rv != NS_OK)
	{
		s_PluginMessageError = "retrieve proxy info failed!";
		mShellBase.sendWarnMessage("nsPluginInstance::nsPluginInstance -> retrieve proxy info failed!");
		delete mInstDir;
		return;
	}

	rv = getCurrPlugDir(mInstDir, NPOSG4WEB_DIRECTORY);
	if(rv != NS_OK)
	{
		s_PluginMessageError = "retrieve plugin directory failed!";
		mShellBase.sendWarnMessage("nsPluginInstance::nsPluginInstance -> retrieve plugin directory failed!");
		return;
	}
//TODO: controllare
#if defined(_DEBUG) //Attach Debug directory or Profile OSG4Web Extension directory
	mInstDir->append(APPEND_NPOSG4WEB_DIRECTORY);
#else
	mInstDir->append(APPEND_NPOSG4WEB_EXTINSTALLDIR);
#endif

	rv = getCurrPlugDir(mTempDir, NS_OS_TEMP_DIR);
	if(rv != NS_OK)
	{
		s_PluginMessageError = "retrieve temp directory failed!";
		mShellBase.sendWarnMessage("nsPluginInstance::nsPluginInstance -> retrieve temp directory failed!");
		return;
	}

	mTempDir->append(APPEND_TEMP_DIRECTORY);

#if defined(NPOSG4WEB_COREDIR_PLUGIN) //Se settata imposto la directory di lavoro dei core in plugin di firefox
	rv = getCurrPlugDir(mCoreInstallDir, NPOSG4WEB_DIRECTORY); 
#else
	rv = getCurrPlugDir(mCoreInstallDir, COREINSTALL_DIRECTORY);
#endif

	if(rv != NS_OK)
	{
		s_PluginMessageError = "retrieve core directory failed!";
		mShellBase.sendWarnMessage("nsPluginInstance::nsPluginInstance -> retrieve core directory failed!");
		return;
	}

	mCoreInstallDir->append(APPEND_CORES_DIRECTORY);

	mShellBase.sendNotifyMessage("nsPluginInstance::nsPluginInstance -> configuring ShellBase with firefox settings");

	if(!mShellBase.setInstallDirectory(*mInstDir))
	{
		s_PluginMessageError = mShellBase.getErrorString();
		mShellBase.sendWarnMessage("nsPluginInstance::nsPluginInstance -> setting install directory failed!");
		return;
	}

	if(!mShellBase.setTempDirectory(*mTempDir))
	{
		s_PluginMessageError = mShellBase.getErrorString();
		mShellBase.sendWarnMessage("nsPluginInstance::nsPluginInstance -> setting temp directory failed!");
		return;
	}
		
	if(!mShellBase.setCoreAppDirectory(*mCoreInstallDir))
	{
		s_PluginMessageError = mShellBase.getErrorString();
		mShellBase.sendWarnMessage("nsPluginInstance::nsPluginInstance -> setting core install directory failed!");
		return;
	}

	if( !mProxyHName.empty() )
		mShellBase.setProxyParameters(mProxyHName, mProxyPort);
}

//////////////////////////////////////
//
// Plug-In instance distructor
//
nsPluginInstance::~nsPluginInstance()
{
  	mShellBase.sendNotifyMessage("nsPluginInstance::~nsPluginInstance -> default destructor");

	if(mScriptablePeer)
	{
  		mScriptablePeer->SetInstance(NULL);
	  	NS_IF_RELEASE(mScriptablePeer);
	}

  	if( mADVCore )
		delete mADVCore;

	if( mADVCoreSHA1Hash )
		delete mADVCoreSHA1Hash;

	if( mADVCoreDep )
		delete mADVCoreDep;

	if( mADVCoreDepSHA1Hash )
		delete mADVCoreDepSHA1Hash;

	if( mADVInitOptions )
		delete mADVInitOptions;

	if( mADVStartOptions )
		delete mADVStartOptions;

	if( mLoadCBOptions )
		delete mLoadCBOptions;

	if( mInstDir )
		delete mInstDir;

	if( mTempDir )
		delete mTempDir;

	if( mCoreInstallDir )
		delete mCoreInstallDir;
}

#if defined(WIN32)
static LRESULT CALLBACK PluginWinProc(HWND, UINT, WPARAM, LPARAM);
#else
//TODO: linux
//FIXME: da rifare completamente
static void xt_event_handler(Widget xtwidget, nsPluginInstance *plugin, XEvent *xevent, Boolean *b);
static void xt_timer_draw_proc(XtPointer clientdata, XtIntervalId* id);
#endif

//////////////////////////////////////
//
// Set Parameter from plug-In base. 
// Estrapola la stringa contenente il nome del main model dai parametri iniziali
//
NPError nsPluginInstance::SetInitialData(PRUint16 argc, char* argn[], char* argv[])
{
	mShellBase.sendNotifyMessage("nsPluginInstance::SetInitialData -> getting plugin HTML initial parameters.");

	char* advcoreptr = NULL;
	char* advcorestr = "ADVCore"; //Parametri di Init

	char* advcorestartoptptr = NULL;
	char* advcorestartoptstr = "ADVCoreStartOptions";

	char* advcoreinitoptptr = NULL;
	char* advcoreinitoptstr = "ADVCoreInitOptions";

	char* advcorehashptr = NULL;
	char* advcorehashstr = "ADVCoreSHA1";

	char* loadcbopteptr = NULL;
	char* loadcboptestr = "LoadCoreOptions";

	char* advcoredepptr = NULL;
	char* advcoredepstr = "ADVCoreDep";

	char* advcoredephashptr = NULL;
	char* advcoredephashstr = "ADVCoreDepSHA1";

	NPError rv = NPERR_NO_ERROR;

	if (argn != NULL && argv != NULL) 
	{
		for (int i = 0; i < argc; i++)   
		{
#if defined(XP_UNIX) || defined(XP_MAC)
			if (strcasecmp(advcorestr, argn[i]) == 0)
#else
			if (stricmp(advcorestr, argn[i]) == 0) 
#endif
			{
				advcoreptr = (char*) argv[i];
			}
#if defined(XP_UNIX) || defined(XP_MAC)
			if (strcasecmp(advcoreinitoptstr, argn[i]) == 0)
#else
			if (stricmp(advcoreinitoptstr, argn[i]) == 0) 
#endif
			{
				advcoreinitoptptr = (char*) argv[i];
			}
#if defined(XP_UNIX) || defined(XP_MAC)
			if (strcasecmp(advcorestartoptstr, argn[i]) == 0)
#else
			if (stricmp(advcorestartoptstr, argn[i]) == 0) 
#endif
			{
				advcorestartoptptr = (char*) argv[i];
			}
#if defined(XP_UNIX) || defined(XP_MAC)
			if (strcasecmp(advcorehashstr, argn[i]) == 0)
#else
			if (stricmp(advcorehashstr, argn[i]) == 0) 
#endif
			{
				advcorehashptr = (char*) argv[i];
			}
#if defined(XP_UNIX) || defined(XP_MAC)
			if (strcasecmp(advcoredepstr, argn[i]) == 0)
#else
			if (stricmp(advcoredepstr, argn[i]) == 0) 
#endif
			{
				advcoredepptr = (char*) argv[i];
			}
#if defined(XP_UNIX) || defined(XP_MAC)
			if (strcasecmp(advcoredephashstr, argn[i]) == 0)
#else
			if (stricmp(advcoredephashstr, argn[i]) == 0) 
#endif
			{
				advcoredephashptr = (char*) argv[i];
			}
#if defined(XP_UNIX) || defined(XP_MAC)
			if (strcasecmp(loadcboptestr, argn[i]) == 0)
#else
			if (stricmp(loadcboptestr, argn[i]) == 0) 
#endif
			{
				loadcbopteptr = (char*) argv[i];
			}
		}
	}
	else
	{
		return NPERR_GENERIC_ERROR;
	}

	if(advcoreptr)
		mADVCore = new std::string(advcoreptr);
	else
		mADVCore = new std::string();

	if(advcorehashptr)
		mADVCoreSHA1Hash = new std::string(advcorehashptr);
	else
		mADVCoreSHA1Hash = new std::string();

	if(advcoreinitoptptr)
		mADVInitOptions = new std::string(advcoreinitoptptr);
	else
		mADVInitOptions = new std::string();

	if(advcorestartoptptr)
		mADVStartOptions = new std::string(advcorestartoptptr);
	else
		mADVStartOptions = new std::string();

	if(loadcbopteptr)
		mLoadCBOptions = new std::string(loadcbopteptr);
	else
		mLoadCBOptions = new std::string();

	if(advcoredepptr)
		mADVCoreDep = new std::string(advcoredepptr);
	else
		mADVCoreDep = new std::string();

	if(advcoredephashptr)
		mADVCoreDepSHA1Hash = new std::string(advcoredephashptr);
	else
		mADVCoreDepSHA1Hash = new std::string();

	return rv;
}

nsresult nsPluginInstance::getProxySettings(std::string &proxyhostname, std::string &port)
{
	nsresult rv;

	nsIServiceManager *serviceManager;
	rv = NS_GetServiceManager((nsIServiceManager **)&serviceManager);
	if (NS_FAILED(rv))
	{
		mShellBase.sendWarnMessage("nsPluginInstance::getProxySettings -> error while retriving service manager.");
		return rv;
	}
	else
		NS_ADDREF(serviceManager);
		
	// nsIPrefService
	nsIPrefService *prefService;
	rv = serviceManager->GetServiceByContractID(NS_PREFSERVICE_CONTRACTID, NS_GET_IID(nsIPrefService), (void **)&prefService);
	if (NS_FAILED(rv)) 
	{
		mShellBase.sendWarnMessage("nsPluginInstance::getProxySettings -> preference service not retrieved.");
		NS_RELEASE(serviceManager);
	    return rv;
	}
	else
		NS_ADDREF(prefService);

	nsIPrefBranch* prefBranch;
	rv = prefService->GetBranch("network.proxy.", (nsIPrefBranch **)&prefBranch);
	if (NS_FAILED(rv)) 
	{
		mShellBase.sendWarnMessage("nsPluginInstance::getProxySettings -> network.proxy. service is not present.");
		NS_RELEASE(prefService);
	    return rv;
	}
	else
		NS_ADDREF(prefBranch);

	PRInt32 typeval, pport;
	char* phname = nsnull;

	rv = prefBranch->GetIntPref("type", &typeval);
	if (NS_FAILED(rv)) 
	{
		mShellBase.sendWarnMessage("nsPluginInstance::getProxySettings -> empty network.proxy.type().");
	    return rv;
	}
		
	if(typeval == 1) //Impostazione manuale
	{
		rv = prefBranch->GetCharPref("http", &phname);
		if (NS_FAILED(rv)) 
		{
			mShellBase.sendWarnMessage("nsPluginInstance::getProxySettings -> network.proxy.http not set.");
			NS_RELEASE(prefBranch);
			return rv;
		}

		proxyhostname.append(phname);

		rv = prefBranch->GetIntPref("http_port", &pport);
		if (NS_FAILED(rv)) 
		{
			mShellBase.sendWarnMessage("nsPluginInstance::getProxySettings -> network.proxy.http_port not set.");
			NS_RELEASE(prefBranch);
			return rv;
		}
		//TODO: capire come fare per richiamare le preferenze globali senza caricare file
		std::ostringstream convstream; //creates an ostringstream object
		convstream << pport << std::flush;
	    port += convstream.str(); 
		
	}
	else if(typeval == 4) //Caso impostazioni automatiche
	{
		//TODO
		mShellBase.sendWarnMessage("nsPluginInstance::getProxySettings -> network.proxy.type AUTO SETTINGS... TODO.");
	}
	
	NS_RELEASE(prefBranch);

	return rv;
}

//////////////////////////////////////
//
// return working plugin directory
//
nsresult nsPluginInstance::getCurrPlugDir(std::string* dir, const char* diropt)
{
	nsresult rv;
	nsIFile* nsdir;
	nsString path;

	nsIServiceManager *sm;
	rv = NS_GetServiceManager((nsIServiceManager **)&sm);
	if (NS_FAILED(rv))
	{
		mShellBase.sendWarnMessage("nsPluginInstance::getCurrPlugDir -> error while retriving service manager.");
		return rv;
	}
	else
		NS_ADDREF(sm);

	nsIProperties *directoryService;
	rv = sm->GetServiceByContractID(NS_DIRECTORY_SERVICE_CONTRACTID, NS_GET_IID(nsIProperties), (void **)&directoryService);
	if (NS_FAILED(rv)) 
	{
		mShellBase.sendWarnMessage("nsPluginInstance::getCurrPlugDir -> error while retriving directory service.");
		NS_RELEASE(sm);
	    return rv;
	}
	else
		NS_ADDREF(directoryService);

	rv = directoryService->Get( diropt, NS_GET_IID(nsIFile),(void **)&nsdir);

	if (NS_FAILED(rv)) 
	{
		mShellBase.sendWarnMessage("nsPluginInstance::getCurrPlugDir -> " + std::string(diropt) + " request failed.");
		NS_RELEASE(directoryService);
		NS_RELEASE(sm);
		return rv;
	}
	else
		NS_ADDREF(nsdir);

 	NS_RELEASE(directoryService);
	NS_RELEASE(sm);

	nsdir->GetPath(path);

	nsCString utf8path = NS_ConvertUTF16toUTF8(path);
	PRUint32 len = utf8path.Length();

	dir->resize(len);

	for(PRUint32 iter = 0; iter < len; ++iter)
    	dir->at((unsigned int) iter) = (utf8path.get())[(unsigned int) iter];
	
	NS_RELEASE(nsdir);

	return rv;
}

//////////////////////////////////////
//
// Inizializza il plugin.
// Richiede working plugin directory, inizializza osgviewcore, lancia il 
// caricamento del main model, attiva il ciclo di rendering
//
NPBool nsPluginInstance::init(NPWindow* aWindow)
{
	mShellBase.sendNotifyMessage("nsPluginInstance::init -> Initializing Window.");

	mInitialized = false;

	if(aWindow == NULL || mInstDir == NULL)
	{
		mShellBase.sendWarnMessage("nsPluginInstance::init -> window pointer is not present.");
		return mInitialized;
	}

#if defined(WIN32)
	mhWnd = (HWND)aWindow->window;
	if(mhWnd == NULL)
	{
		s_PluginMessageError = "invalid window pointer!";
		mShellBase.sendWarnMessage("nsPluginInstance::init -> window pointer is not valid.");
		return mInitialized;
	}

	// subclass window so we can intercept window messages and
	// do our drawing to it
	lpOldProc = SubclassWindow(mhWnd, (WNDPROC)PluginWinProc);
 
	// associate window with our nsPluginInstance object so we can access 
	// it in the window procedure
	SetWindowLong(mhWnd, GWL_USERDATA, (LONG) this);
	
#else 
	//TODO: linux
	//FIXME: da rifare completamente
	if (!SetWindow(aWindow))
	{
		s_PluginMessageError = "invalid window pointer!";
		return mInitialized;
	}

	//FIXME: CORREGGE IL PROBLEMA DI CARICAMENTO DELLE DIPENDENZA DI LIBRERIA --WINDOWS--
	//putenv((char*) (std::string("PATH=") + *dir + std::string(":")  + path + std::string(":") + *dir).c_str()); //FIXME: controllare quale nella sequenza va bene

	//mXttimeid = XtAppAddTimeOut( XtWidgetToApplicationContext(mXtwidget), mTimer, xt_timer_draw_proc, this );
#endif

	//Linking Plugin to ShellBase
	mShellBase.sendNotifyMessage("nsPluginInstance::init -> Linking Plugin to ShellBase.");

	mShellBase.setWindowHandler( mhWnd );
	mShellBase.setInstanceHandler( this );
	mShellBase.setResetWindowHandler( &nsPluginInstance::callResetWindowHandler );
	mShellBase.setTransportEventHandler( &nsPluginInstance::TransportEvent );
	mShellBase.setRenderingHandler( &nsPluginInstance::callPrepareRendering, &nsPluginInstance::callCloseRendering );
	mShellBase.setDownloadingCoreHandler( &nsPluginInstance::callRequestCoreDownloading );

	return true;
}

NPBool nsPluginInstance::initLoadCore()
{
	mShellBase.sendNotifyMessage("nsPluginInstance::initLoadCore -> setting Core Options.");

	std::string str("LOADCORE_OPTIONS=");

	mInitialized = true;

	if(!mLoadCBOptions->empty())
	{
		str += *mLoadCBOptions;

		if(mShellBase.execShellCommand(str) != std::string("DONE"))
		{
			s_PluginMessageError = "ShellBase - setting loading core options failed!";
			mShellBase.sendWarnMessage("nsPluginInstance::initLoadCore -> setting loading core options failed.");
			return false;
		}
	}

	//Sets Advanced core through Init Parameters
	if(!mADVCore->empty())
	{
		str = "SET_COREADV=";
		str += *mADVCore;
	
		if(mShellBase.execShellCommand(str) != std::string("DONE"))
		{
			s_PluginMessageError = std::string("ShellBase - setting ") + *mADVCore + std::string(" failed!");
			mShellBase.sendWarnMessage(std::string("nsPluginInstance::initLoadCore -> ") + std::string("setting ") + *mADVCore + std::string(" failed!"));
			return false;
		}

		str = "SET_COREADV_INIT_OPTIONS=";
		str += *mADVInitOptions;

		if(mShellBase.execShellCommand(str) != std::string("DONE"))
		{
			s_PluginMessageError = std::string("ShellBase - setting ") + *mADVInitOptions + std::string(" init options failed!");
			mShellBase.sendWarnMessage(std::string("nsPluginInstance::initLoadCore -> ") + std::string("setting ") + *mADVInitOptions + std::string(" init options failed!"));
			return false;
		}

		str = "SET_COREADV_START_OPTIONS=";
		str += *mADVStartOptions;

		if(mShellBase.execShellCommand(str) != std::string("DONE"))
		{
			s_PluginMessageError = std::string("ShellBase - setting ") + *mADVStartOptions + std::string(" start options failed!");
			mShellBase.sendWarnMessage(std::string("nsPluginInstance::initLoadCore -> ") + std::string("setting ") + *mADVStartOptions + std::string(" start options failed!"));
			return false;
		}

		str = "SET_COREADV_SHA1HASH=";
		str += *mADVCoreSHA1Hash;

		if(mShellBase.execShellCommand(str) != std::string("DONE"))
		{
			s_PluginMessageError = std::string("ShellBase - setting SHA-1 Hash string failed!");
			mShellBase.sendWarnMessage(std::string("nsPluginInstance::initLoadCore -> setting SHA-1 Hash string failed!"));
			return false;
		}

		//Richiesta di un pacchetto di dipendenza da parte del core
		if(!mADVCoreDep->empty())
		{
			str = "SET_COREADVDEP=";
			str += *mADVCoreDep;
	
			if(mShellBase.execShellCommand(str) != std::string("DONE"))
			{
				s_PluginMessageError = std::string("ShellBase - setting dep. ") + *mADVCoreDep + std::string(" failed!");
				mShellBase.sendWarnMessage(std::string("nsPluginInstance::initLoadCore -> ") + std::string("setting dep. ") + *mADVCoreDep + std::string(" failed!"));
				return false;
			}

			str = "SET_COREADVDEP_SHA1HASH=";
			str += *mADVCoreDepSHA1Hash;

			if(mShellBase.execShellCommand(str) != std::string("DONE"))
			{
				s_PluginMessageError = std::string("ShellBase - setting dep. SHA-1 Hash string failed!");
				mShellBase.sendWarnMessage(std::string("nsPluginInstance::initLoadCore -> setting dep. SHA-1 Hash string failed!"));
				return false;
			}
		}
	}

	mShellBase.sendNotifyMessage("nsPluginInstance::initLoadCore -> starting the Loading Core.");
	if(!mShellBase.startLoadingBaseCore())
	{
		s_PluginMessageError = mShellBase.getErrorString();
		mShellBase.sendWarnMessage(std::string("nsPluginInstance::initLoadCore -> ") + s_PluginMessageError);
		return false;
	}

	//Return if CoreString is inizialized and SHA-1 isn't
	if(!mADVCore->empty() && mADVCoreSHA1Hash->empty())
	{
		//Setting Error Message
		this->loadingCoreCommand("LOADCORE SETMESSAGE_COLOR LC_OSG_RED");
		this->loadingCoreCommand("LOADCORE SETMESSAGE Advanced Core SHA-1 HASH not set!");

		mShellBase.sendWarnMessage(std::string("nsPluginInstance::initLoadCore -> Advanced Core SHA-1 HASH not set!"));

		return mInitialized;
	}

	if(!mShellBase.initializeAdvancedCore())
	{
		//Reload LoadCore
		if(!mShellBase.startLoadingBaseCore())
		{
			s_PluginMessageError = mShellBase.getErrorString();
			mShellBase.sendWarnMessage(std::string("nsPluginInstance::initLoadCore -> ") + s_PluginMessageError);
			return false;
		}

		//Setting Error Message
		this->loadingCoreCommand("LOADCORE SETMESSAGE_COLOR LC_OSG_RED");
		this->loadingCoreCommand("LOADCORE SETMESSAGE Initializing Advanced Core Failed!");

		mShellBase.sendWarnMessage(std::string("nsPluginInstance::initLoadCore -> Initializing Advanced Core Failed!"));
	}
	
	return mInitialized;
}

NPError nsPluginInstance::SendGetEvent(std::string evnt)
{
	NPError errcode;
	std::string eventstr("javascript:eventCatcher(\"");
	
	eventstr +=  evnt + std::string("\")");

	//Invio segnale al browser
	errcode = NPN_GetURL(mInstance, eventstr.c_str(), "_self");

	mShellBase.sendNotifyMessage(std::string("nsPluginInstance::SendGetEvent -> Sending Message to JS"));

	if(errcode != NPERR_NO_ERROR)
	{
		std::ostringstream convstream;
		convstream << errcode << std::flush;

		mShellBase.sendWarnMessage(std::string("nsPluginInstance::TransportEvent -> NPN_GetURL NPError code: ") + convstream.str());
	}

	return errcode;
}

std::string nsPluginInstance::loadingCoreCommand(std::string line )
{
	std::string ret;

	mShellBase.sendNotifyMessage(std::string("nsPluginInstance::loadingCoreCommand -> Sending Loading Core Command."));

	//Richiedo CS sul Main Rendering Thread
	PR_Lock(s_ThreadLocking);
	ret = mShellBase.execCoreCommand(line);

	//Esco CS
	PR_Unlock(s_ThreadLocking);

	return ret;
}

bool nsPluginInstance::TransportEvent(void* classptr, std::string eventstr)
{
	nsPluginInstance* ctrlptr = (nsPluginInstance*) classptr;

	if(ctrlptr != NULL)
	{
		NPError error = ctrlptr->SendGetEvent(eventstr);
		
		if( error == NPERR_NO_ERROR)
			return true;
	}

	return false;
}

void nsPluginInstance::sendWarnMessage(std::string message)
{
	mShellBase.sendWarnMessage(message);
}

//////////////////////////////////////
//
// Chiude l'istanza plug-In
//
void nsPluginInstance::shut()
{
	mShellBase.sendNotifyMessage(std::string("nsPluginInstance::shut -> Shutting down plugin instance"));

	this->releaseDownloadCore();
	
	if(!mShellBase.closeAllLibraries())
	{
		mShellBase.sendWarnMessage(std::string("nsPluginInstance::shut -> Error closing all libraries."));
		s_PluginMessageError = mShellBase.getErrorString(); //TODO: correggere il messaggio
	}
	
#if defined(WIN32)
	// subclass it back
	SubclassWindow(mhWnd, lpOldProc);
	mhWnd = NULL;
#else
	//TODO: da finire
	//FIXME: da rifare completamente
	if( mXttimeid != 0 )
	{
		XtRemoveWorkProc( mXttimeid );
        	mXttimeid = 0;
	}

	osgViewCore::ReleaseScene(mhWnd);
#endif

	mShellBase.sendNotifyMessage(std::string("nsPluginInstance::shut -> Shutting down finished"));

	mInitialized = false;
}


/*
NPError nsPluginInstance::SetWindow(NPWindow* aWindow)
{
	// keep window parameters
	mWindow = aWindow;
	return NPERR_NO_ERROR;
}

uint16 nsPluginInstance::HandleEvent(void* aEvent)
{
	NPEvent * event = (NPEvent *)aEvent;
	switch (event->event) {
	case WM_PAINT: 
	{
	if(!mWindow)
		break;

		// get the dirty rectangle to update or repaint the whole window
		RECT * drc = (RECT *)event->lParam;
		if(drc)
			FillRect((HDC)event->wParam, drc, (HBRUSH)(COLOR_ACTIVECAPTION+1));
		else 
		{
			RECT rc;
			rc.bottom = mWindow->y + mWindow->height;
			rc.left   = mWindow->x;
			rc.right  = mWindow->x + mWindow->width;
			rc.top    = mWindow->y;
			FillRect((HDC)event->wParam, &rc, (HBRUSH)(COLOR_ACTIVECAPTION+1));
		}
		break;
	} 
	case WM_KEYDOWN:
	{
		Beep(1000,100);
		break;
	}
	default:
		return 0;
	}
	return 1;
}
*/

//////////////////////////////////////
//
// Check if the instance is initialized
//
NPBool nsPluginInstance::isInitialized()
{
  return mInitialized;
}

// ==============================
// ! Scriptability related code !
// ==============================
//
NPError	nsPluginInstance::GetValue(NPPVariable aVariable, void *aValue)
{
  	NPError rv = NPERR_NO_ERROR;

  	if (aVariable == NPPVpluginScriptableInstance) 
  	{
  		// addref happens in getter, so we don't addref here
    	nsIScriptablePluginOSG * scriptablePeer = getScriptablePeer();
    	if (scriptablePeer) 
		{
     			*(nsISupports **)aValue = scriptablePeer;
    	} 
		else
			rv = NPERR_OUT_OF_MEMORY_ERROR;
  	}
  	else if (aVariable == NPPVpluginScriptableIID) 
  	{
    	static nsIID scriptableIID = NS_ISCRIPTABLEPLUGINOSG_IID;
    	nsIID* ptr = (nsIID *)NPN_MemAlloc(sizeof(nsIID));
    
		if (ptr) 
		{
        	*ptr = scriptableIID;
        	*(nsIID **)aValue = ptr;
    	} 
		else
      		rv = NPERR_OUT_OF_MEMORY_ERROR;
  	}

  	return rv;
}


///////////////////////////////////////////////////////////////////////////////////////
//
//
//		Funzioni di gestione finestra
//
//
//////////////////////////////////////////////////////////////////////////////////////



bool nsPluginInstance::callResetWindowHandler(void* maininst)
{
	nsPluginInstance *instance = (nsPluginInstance *) maininst;

	if(instance)
		return instance->resetWindowHandler();
	else
		instance->sendWarnMessage(std::string("nsPluginInstance::callResetWindowHandler -> Instance seems not present."));

	return false;
}

bool nsPluginInstance::resetWindowHandler()
{
	mShellBase.sendNotifyMessage("nsPluginInstance::resetWindowHandler -> resetting window handler.");

#if defined(WIN32)
	// subclass it back
	SubclassWindow(mhWnd, lpOldProc);

	// subclass window so we can intercept window messages and
	// do our drawing to it
	lpOldProc = SubclassWindow(mhWnd, (WNDPROC)PluginWinProc);
 
	// associate window with our nsPluginInstance object so we can access 
	// it in the window procedure
	SetWindowLong(mhWnd, GWL_USERDATA, (LONG) this);

#else
	//TODO: linux
	//FIXME: da rifare completamente
#endif

	return true;
}


///////////////////////////////////////////////////////////////////////////////////////
//
//
//		Funzioni di Rendering
//
//
//////////////////////////////////////////////////////////////////////////////////////

void nsPluginInstance::callRender(void* maininst)
{
	nsPluginInstance *instance = (nsPluginInstance *) maininst;

	if(instance)
		instance->doRender();
	else
		instance->sendWarnMessage(std::string("nsPluginInstance::callRender -> Instance seems not present."));
}

bool nsPluginInstance::callPrepareRendering(void* maininst)
{
	nsPluginInstance *instance = (nsPluginInstance *) maininst;

	if(instance)
		return instance->prepareRendering();
	else
		instance->sendWarnMessage(std::string("nsPluginInstance::callPrepareRendering -> Instance seems not present."));

	return false;
}
	
bool nsPluginInstance::callCloseRendering(void* maininst)
{
	nsPluginInstance *instance = (nsPluginInstance *) maininst;

	if(instance)
		return instance->closeRendering();
	else
		instance->sendWarnMessage(std::string("nsPluginInstance::callCloseRendering -> Instance seems not present."));

	return false;
}

bool nsPluginInstance::prepareRendering()
{
	bool ret = false;

#if defined(RENDER_WITH_TIMER_EVENT)
#if defined(WIN32)
	SetTimer(mhWnd, 0, mTimer, NULL);
#else
	//TODO: linux
	//FIXME: da rifare completamente
#endif

	ret = true;
#else
	assert(this->getThread() == NULL);

	mShutdownThread = false;

	mShellBase.sendNotifyMessage(std::string("nsPluginInstance::prepareRendering -> Start Rendering Thread."));

	mThread = PR_CreateThread(PR_USER_THREAD, callRender, this,
			      PR_PRIORITY_NORMAL, PR_GLOBAL_THREAD,
			      PR_JOINABLE_THREAD, 0);

	if(mThread == NULL)
	{
		mShellBase.sendWarnMessage(std::string("nsPluginInstance::prepareRendering -> Rendering Thread isn't started."));
		ret = false;
	}
	else
	{
		mShellBase.sendNotifyMessage(std::string("nsPluginInstance::prepareRendering -> Rendering Thread is up and running."));
		ret = true;
	}

#endif
	return ret;
}

bool nsPluginInstance::closeRendering()
{
#if defined(RENDER_WITH_TIMER_EVENT)
#if defined(WIN32)
	KillTimer(mhWnd, NULL);
#else
	//TODO: linux
	//FIXME: da rifare completamente
#endif
#else
	if (this->getThread())
	{
		mShellBase.sendNotifyMessage(std::string("nsPluginInstance::closeRendering -> Join Rendering Thread."));

		mShutdownThread = true;

		PR_JoinThread(mThread);

		mShellBase.sendNotifyMessage(std::string("nsPluginInstance::closeRendering -> Delete Rendering Thread."));

		PR_Sleep(EXIT_DELAY);

		mShutdownThread = false;
		mThread = NULL;
	}
	else
		return false;
#endif

	return true;
}

void nsPluginInstance::doRender()
{
	PR_Lock(s_ThreadLocking);

	mShellBase.sendNotifyMessage(std::string("nsPluginInstance::doRender -> Render Started."));
	
	for (;;)
	{
		//TODO: aggiungere messaggi di errore 
		mShellBase.doRendering();
		
		if (this->getThreadShutDown())
			break;
			
		PR_Unlock(s_ThreadLocking);

		PR_Sleep(RENDER_DELAY);

		PR_Lock(s_ThreadLocking);
	}

	mShellBase.sendNotifyMessage(std::string("nsPluginInstance::doRender -> Render Finished."));
	
	PR_Unlock(s_ThreadLocking);
	return;
}

void nsPluginInstance::doRenderFrame()
{
	mShellBase.doRendering();
}

///////////////////////////////////////////////////////////////////////////////////////
//
//
//		Funzioni Thread di Download
//
//
//////////////////////////////////////////////////////////////////////////////////////

bool nsPluginInstance::callRequestCoreDownloading(void* maininst)
{
	nsPluginInstance *instance = (nsPluginInstance *) maininst;

	if(instance)
		return instance->requestCoreDownloading();
	else
		instance->sendWarnMessage(std::string("nsPluginInstance::callRequestCoreDownloading -> Instance seems not present."));

	return false;
}

bool nsPluginInstance::requestCoreDownloading()
{
	bool ret = true;

	assert(this->getDlCoreThread() == NULL);

	mDlCoreShutdownThread = false;

	mShellBase.sendNotifyMessage(std::string("nsPluginInstance::requestCoreDownloading -> Start Downloading Thread."));

	mDlCoreThread = PR_CreateThread(PR_USER_THREAD, callDownloadCore, this,
			      PR_PRIORITY_NORMAL, PR_GLOBAL_THREAD,
			      PR_JOINABLE_THREAD, 0);

	if(mDlCoreThread == NULL)
	{
		mShellBase.sendWarnMessage(std::string("nsPluginInstance::requestCoreDownloading -> Downloading Thread is not started."));
		ret = false;
	}
	else
	{
		mShellBase.sendNotifyMessage(std::string("nsPluginInstance::requestCoreDownloading -> Downloading Thread is up and running."));
		ret = true;
	}

	return ret;
}

void nsPluginInstance::callDownloadCore(void* maininst)
{
	nsPluginInstance *instance = (nsPluginInstance *) maininst;

	if(instance)
		instance->doDownloadCore();
	else
		instance->sendWarnMessage(std::string("nsPluginInstance::callDownloadCore -> Instance seems not present."));
}

bool nsPluginInstance::downloadUnpackPackage(std::string packagename)
{
	//TODO: finire il supporto al download di più pacchetti... mettere in env anche la directory del pacchetto di dipendenza

	return false;
}


void nsPluginInstance::doDownloadCore()
{
	PR_Lock(s_DlCoreThreadLocking);

	mShellBase.sendNotifyMessage(std::string("nsPluginInstance::doDownloadCore -> Downloading Started."));

	mLoading = true;

	std::string fdlcoreaddress = mShellBase.getAdvancedCoreAddress();
	std::string fdlcorename = mShellBase.getAdvancedCoreFileName();
	std::string tempdl; 
	std::string tempdldir( *mTempDir );
	std::string tempheaderdl;

#if defined(WIN32)
	#define PATH_SEP "\\"
//	tempdldir = tempdldir + "\\" + *mADVCoreSHA1Hash;
//	tempdl = tempdldir + "\\" + fdlcorename;
//	tempheaderdl = tempdldir + "\\" + "headerdl.html";
#else
	#define PATH_SEP "/"
//	tempdldir = tempdldir + "/" + mShellBase.getAdvancedCoreSHA1();
//	tempdl = tempdldir + "/" + fdlcorename;
//	tempheaderdl = tempdldir + "/" + "headerdl.html";
#endif
	tempdldir = tempdldir + PATH_SEP + *mADVCoreSHA1Hash;
	tempdl = tempdldir + PATH_SEP + fdlcorename;
	tempheaderdl = tempdldir + PATH_SEP + "headerdl.html";


	CURL* curl;
	CURLcode res;
	FILE* outfile;
	FILE* headerfile;

	std::string proxy;

	//Check Core Presence
	if(mShellBase.checkAdvCorePresence())
	{
		//Attivo status bar
		this->loadingCoreCommand("LOADCORE STATUSBAR_VISIBILITY FALSE");

		//Attivo il messaggio di Download del Core
		this->loadingCoreCommand("LOADCORE SETMESSAGE_COLOR LC_OSG_BLUE");
		this->loadingCoreCommand("LOADCORE SETMESSAGE Core Found...");

		PR_Sleep(1500); //Delay per far visualizzare il messaggio //FIXME: mettere in define

		//Loading Session
		this->loadingCoreCommand("LOADCORE STATUSBAR_VISIBILITY FALSE");
		this->loadingCoreCommand("LOADCORE SETMESSAGE "); //Spengo i messaggi

		mShellBase.sendNotifyMessage(std::string("nsPluginInstance::doDownloadCore -> Loading Unpacked Core"));

		if(!mShellBase.startLoadingAdvancedCore()) //Start Advanced Core Loading
		{
			mShellBase.sendWarnMessage(std::string("nsPluginInstance::doDownloadCore -> Loading advanced core failed! Starting Loading Core"));

			//Reload LoadCore
			if(!mShellBase.startLoadingBaseCore())
			{
				s_PluginMessageError = mShellBase.getErrorString();
				mShellBase.sendWarnMessage(std::string("nsPluginInstance::doDownloadCore -> Starting Loading Core failed... BYE BYE..."));
				return;
			}

			//Setting Error Message
			this->loadingCoreCommand("LOADCORE SETMESSAGE_COLOR LC_OSG_RED");
			this->loadingCoreCommand("LOADCORE SETMESSAGE Loading Advanced Core Failed!");
		}
		else
			mShellBase.sendNotifyMessage(std::string("nsPluginInstance::doDownloadCore -> Advanced Core is up and running"));

		PR_Unlock(s_DlCoreThreadLocking);

		mLoading = false;

		return;
	}

	//Downloading Session
	if(!mShellBase.checkFileExistance(tempdl))
	{
		if(!mShellBase.checkOrCreateDirectory(tempdldir))
		{
			mShellBase.sendWarnMessage(std::string("nsPluginInstance::doDownloadCore -> Creating Downloading Temp Directory Failed."));

			this->loadingCoreCommand("LOADCORE STATUSBAR_VISIBILITY FALSE");

			this->loadingCoreCommand("LOADCORE SETMESSAGE_COLOR LC_OSG_RED");
			this->loadingCoreCommand("LOADCORE SETMESSAGE Creating Temp Directory Failed!");

			PR_Unlock(s_DlCoreThreadLocking);
			return;
		}

		curl = curl_easy_init();

		if(curl)
		{
			mShellBase.sendNotifyMessage(std::string("nsPluginInstance::doDownloadCore -> Curl Session Initialized."));

			outfile = fopen(tempdl.c_str(), "wb"); //FIXME: vedere se si possono usare gli stream
			
			if (outfile == NULL) 
			{
				this->loadingCoreCommand("LOADCORE STATUSBAR_VISIBILITY FALSE");

				this->loadingCoreCommand("LOADCORE SETMESSAGE_COLOR LC_OSG_RED");
				this->loadingCoreCommand("LOADCORE SETMESSAGE Opening temp file failed!");

				curl_easy_cleanup(curl);

				mShellBase.sendWarnMessage(std::string("nsPluginInstance::doDownloadCore -> Error Opening Out File."));

				PR_Unlock(s_DlCoreThreadLocking);
				return;
			}

			headerfile = fopen(tempheaderdl.c_str(), "w");

			if (headerfile == NULL) 
			{
				//TODO: aggiungere i messaggi di errore al core e gestirli con curl_info
				this->loadingCoreCommand("LOADCORE STATUSBAR_VISIBILITY FALSE");

				this->loadingCoreCommand("LOADCORE SETMESSAGE_COLOR LC_OSG_RED");
				this->loadingCoreCommand("LOADCORE SETMESSAGE Opening temp file failed!");

				curl_easy_cleanup(curl);

				mShellBase.sendWarnMessage(std::string("nsPluginInstance::doDownloadCore -> Error Opening Header File."));

				fclose(outfile);
				mShellBase.removingFile(tempdl);

				PR_Unlock(s_DlCoreThreadLocking);
				return;
			}
		
			curl_easy_setopt(curl, CURLOPT_VERBOSE, 1);
			//curl_easy_setopt(curl, CURLOPT_HEADER, 1);
			curl_easy_setopt(curl, CURLOPT_URL, fdlcoreaddress.c_str());
	
			//Proxy Settings
			if(!mProxyHName.empty())
			{
				proxy = mProxyHName;

				if(!mProxyPort.empty())
				{
					proxy += ":";
					proxy += mProxyPort;
				}

				curl_easy_setopt(curl, CURLOPT_PROXY, proxy.c_str()); 
			}

			//TODO: capire come fare per gestire user e password del proxy

			curl_easy_setopt(curl, CURLOPT_WRITEDATA, outfile);
			curl_easy_setopt(curl, CURLOPT_WRITEHEADER, headerfile);
			curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, &nsPluginInstance::writeDLStream);
			curl_easy_setopt(curl, CURLOPT_READFUNCTION, &nsPluginInstance::readDLStream);
			curl_easy_setopt(curl, CURLOPT_NOPROGRESS, FALSE);
			curl_easy_setopt(curl, CURLOPT_PROGRESSFUNCTION, &nsPluginInstance::callProgressDLStatus);
			curl_easy_setopt(curl, CURLOPT_PROGRESSDATA, this);

			//Attivo status bar
			this->loadingCoreCommand("LOADCORE STATUSBAR_VISIBILITY TRUE");
			this->loadingCoreCommand("LOADCORE STATUSBAR_COLOR LC_OSG_BLUE");

			//Attivo il messaggio di Download del Core
			this->loadingCoreCommand("LOADCORE SETMESSAGE_COLOR LC_OSG_BLUE");
			this->loadingCoreCommand("LOADCORE SETMESSAGE Downloading Core...");
		
			res = curl_easy_perform(curl);
		
			fclose(outfile);
			fclose(headerfile);

			if(res != CURLE_OK)
			{
				this->loadingCoreCommand("LOADCORE STATUSBAR_VISIBILITY FALSE");

				this->loadingCoreCommand("LOADCORE SETMESSAGE_COLOR LC_OSG_RED");
				this->loadingCoreCommand("LOADCORE SETMESSAGE Downloading Failed!");

				curl_easy_cleanup(curl);

				mShellBase.removingFile(tempdl);

				mShellBase.sendWarnMessage(std::string("nsPluginInstance::doDownloadCore -> Downloading Failed."));

				PR_Unlock(s_DlCoreThreadLocking);
				return;
			}
			
			//Opening Header File and checking server response
			std::string headline;
			std::ifstream checkheaderfile(tempheaderdl.c_str());

			if(checkheaderfile.is_open())
			{
				if (! checkheaderfile.eof() )
				{
					getline( checkheaderfile, headline );

					std::string::size_type pos = headline.find("200");

					if(pos == std::string::npos) //Error in Downloading
					{
						this->loadingCoreCommand("LOADCORE STATUSBAR_VISIBILITY=FALSE");

						this->loadingCoreCommand("LOADCORE SETMESSAGE_COLOR LC_OSG_RED");
						this->loadingCoreCommand("LOADCORE SETMESSAGE Server response: " + headline);

						curl_easy_cleanup(curl);

						mShellBase.sendWarnMessage(std::string("nsPluginInstance::doDownloadCore -> Server Error: " + headline));

						checkheaderfile.close();
						mShellBase.removingFile(tempdl);

						PR_Unlock(s_DlCoreThreadLocking);
						return;
					}
				}
				
				checkheaderfile.close();
			}
			else
			{
				this->loadingCoreCommand("LOADCORE STATUSBAR_VISIBILITY FALSE");

				this->loadingCoreCommand("LOADCORE SETMESSAGE_COLOR LC_OSG_RED");
				this->loadingCoreCommand("LOADCORE SETMESSAGE Checking server response failed!");

				curl_easy_cleanup(curl);
				mShellBase.removingFile(tempdl);

				mShellBase.sendWarnMessage(std::string("nsPluginInstance::doDownloadCore -> Error Opening Header File. Checking server response failed"));

				PR_Unlock(s_DlCoreThreadLocking);
				return;
			}

			curl_easy_cleanup(curl);
		}
		else
		{
			this->loadingCoreCommand("LOADCORE STATUSBAR_VISIBILITY FALSE");

			this->loadingCoreCommand("LOADCORE SETMESSAGE_COLOR LC_OSG_RED");
			this->loadingCoreCommand("LOADCORE SETMESSAGE Download Initialization Failed!");

			mShellBase.sendWarnMessage(std::string("nsPluginInstance::doDownloadCore -> Curl initialization Failed."));
			PR_Unlock(s_DlCoreThreadLocking);

			return;
		}
	}
	else
		mShellBase.sendNotifyMessage(std::string("nsPluginInstance::doDownloadCore -> Temporary package is present, Skipping Downloading."));

	this->loadingCoreCommand("LOADCORE SETMESSAGE Checking Validity...");
	mShellBase.sendNotifyMessage(std::string("nsPluginInstance::doDownloadCore -> Check Package Validity."));

	if(!mShellBase.checkFileValidity(tempdl, *mADVCoreSHA1Hash))
	{
		this->loadingCoreCommand("LOADCORE STATUSBAR_VISIBILITY FALSE");

		this->loadingCoreCommand("LOADCORE SETMESSAGE_COLOR LC_OSG_RED");
		this->loadingCoreCommand("LOADCORE SETMESSAGE Validity Control Failed!");

		mShellBase.sendWarnMessage(std::string("nsPluginInstance::doDownloadCore -> Validity Control Failed!"));

		//Cleaning Archive List
		mShellBase.freeCompressedCore();
		PR_Unlock(s_DlCoreThreadLocking);

		return;
	}

	//Unpacking Session
	this->loadingCoreCommand("LOADCORE STATUSBAR_COLOR LC_OSG_GREEN");
	this->loadingCoreCommand("LOADCORE STATUSBAR_VISIBILITY TRUE");
	this->loadingCoreCommand("LOADCORE STATUSBARVALUE 0"); //Reset Status Bar Lenght
	
	//Attivo il messaggio di Download del Core
	this->loadingCoreCommand("LOADCORE SETMESSAGE_COLOR LC_OSG_GREEN");
	this->loadingCoreCommand("LOADCORE SETMESSAGE Unpacking Core...");

	mShellBase.sendNotifyMessage(std::string("nsPluginInstance::doDownloadCore -> Unpacking Core."));
	
	int coreunpack = 0; //Condizione di loading

	//Opening Archive List
	int filenumber = mShellBase.openCompressedCore(tempdl);

	if(filenumber < 0)
	{
		//TODO: messaggi di Errore
		this->loadingCoreCommand("LOADCORE STATUSBAR_VISIBILITY FALSE");

		this->loadingCoreCommand("LOADCORE SETMESSAGE_COLOR LC_OSG_RED");
		this->loadingCoreCommand("LOADCORE SETMESSAGE Unpacking Failed! Error opening archive");

		mShellBase.sendWarnMessage(std::string("nsPluginInstance::doDownloadCore -> Unpacking Failed! Error opening archive"));

		//Cleaning Archive List
		mShellBase.freeCompressedCore();
		PR_Unlock(s_DlCoreThreadLocking);

		return;
	}

	if(filenumber == 0)
	{
		//TODO: messaggi di Errore
		this->loadingCoreCommand("LOADCORE STATUSBAR_VISIBILITY FALSE");

		this->loadingCoreCommand("LOADCORE SETMESSAGE_COLOR LC_OSG_RED");
		this->loadingCoreCommand("LOADCORE SETMESSAGE Unpacking Failed! Empty archive");

		mShellBase.sendWarnMessage(std::string("nsPluginInstance::doDownloadCore -> Unpacking Failed! Empty archive"));

		//Cleaning Archive List
		mShellBase.freeCompressedCore();
		PR_Unlock(s_DlCoreThreadLocking);

		return;
	}
	
	unsigned int cicleno = 0;
	while(coreunpack == 0)
	{
		coreunpack = mShellBase.unpackCompressedCoreFile();

		//FIXME: cotrollare che non servano le CS 
		std::string statmsg("LOADCORE STATUSBARVALUE ");

		std::ostringstream convstream; //creates an ostringstream object
		convstream << (cicleno * 300.0 / filenumber) << std::flush; //Uso la dimensione 3x
		statmsg += convstream.str(); 

		//mShellBase.execCoreCommand(statmsg); //Posso usare direttamente mShellBase per evitare il delay della CS. E' SAFE
		this->loadingCoreCommand(statmsg);

		cicleno++;

		PR_Unlock(s_DlCoreThreadLocking);

		if (this->getDlCoreThreadShutDown())
			break;
		
		PR_Lock(s_DlCoreThreadLocking);
	}

	if(coreunpack < 0)
	{
		//TODO: messaggi di Errore
		this->loadingCoreCommand("LOADCORE STATUSBAR_VISIBILITY FALSE");

		this->loadingCoreCommand("LOADCORE SETMESSAGE_COLOR LC_OSG_RED");
		this->loadingCoreCommand("LOADCORE SETMESSAGE Unpacking Failed! Error during decompression");

		mShellBase.sendWarnMessage(std::string("nsPluginInstance::doDownloadCore -> Unpacking Failed! Error during decompression"));

		//Cleaning Archive List
		mShellBase.freeCompressedCore();
		PR_Unlock(s_DlCoreThreadLocking);

		return;
	}
	
	//Cleaning Archive List
	mShellBase.freeCompressedCore();

	//Loading Session
	this->loadingCoreCommand("LOADCORE STATUSBAR_VISIBILITY FALSE");
	this->loadingCoreCommand("LOADCORE SETMESSAGE "); //Spengo i messaggi

	mShellBase.sendNotifyMessage(std::string("nsPluginInstance::doDownloadCore -> Loading Unpacked Core"));

	if(!mShellBase.startLoadingAdvancedCore()) //Start Advanced Core Loading
	{
		mShellBase.sendWarnMessage(std::string("nsPluginInstance::doDownloadCore -> Loading advanced core failed! Starting Loading Core"));

		//Reload LoadCore
		if(!mShellBase.startLoadingBaseCore())
		{
			s_PluginMessageError = mShellBase.getErrorString();
			mShellBase.sendWarnMessage(std::string("nsPluginInstance::doDownloadCore -> Starting Loading Core failed... BYE BYE..."));
			return;
		}

		//Setting Error Message
		this->loadingCoreCommand("LOADCORE SETMESSAGE_COLOR LC_OSG_RED");
		this->loadingCoreCommand("LOADCORE SETMESSAGE Loading Advanced Core Failed!");
	}
	else
		mShellBase.sendNotifyMessage(std::string("nsPluginInstance::doDownloadCore -> Advanced Core is up and running"));

	mLoading = false;
	
	PR_Unlock(s_DlCoreThreadLocking);
	return;
}

int nsPluginInstance::callProgressDLStatus(void* classptr, double downtot, double downnow, double ultotal, double ulnow)
{
	nsPluginInstance *instance = (nsPluginInstance *) classptr;


	if(instance)
		return instance->doProgressDLStatus(downtot, downnow, ultotal, ulnow);
	else
	{
		instance->sendWarnMessage(std::string("nsPluginInstance::callProgressDLStatus -> Instance seems not present."));
		return -1;
	}
}


//FIXME: controllare questa funzione
int nsPluginInstance::doProgressDLStatus(double downtot, double downnow, double ultotal, double ulnow)
{
	if(this->getDlCoreThreadShutDown())
	{
		mShellBase.sendNotifyMessage(std::string("nsPluginInstance::doProgressDLStatus -> Forcing Downloading Thread Exit."));
		return 1; //Condizione di uscita per il thread di Download
	}

	//FIXME: cotrollare che non servano le CS 
	std::string statmsg("LOADCORE STATUSBAR_VALUE ");

	std::ostringstream convstream; //creates an ostringstream object
	convstream << (downnow * 300.0 / downtot) << std::flush; //Uso la dimensione 3x
	statmsg += convstream.str(); 

	//mShellBase.execCoreCommand(statmsg); //Posso usare direttamente mShellBase per evitare il delay della CS. E' SAFE
	this->loadingCoreCommand(statmsg);
		
	return 0;
}

size_t nsPluginInstance::writeDLStream(void *ptr, size_t size, size_t nmemb, FILE *stream)
{
	return fwrite(ptr, size, nmemb, stream);
}

size_t nsPluginInstance::readDLStream(void *ptr, size_t size, size_t nmemb, FILE *stream)
{
	return fread(ptr, size, nmemb, stream);
}

bool nsPluginInstance::releaseDownloadCore()
{
	if (this->getDlCoreThread())
	{
		mDlCoreShutdownThread = true;

		mShellBase.sendNotifyMessage(std::string("nsPluginInstance::releaseDownloadCore -> Join Downloading Thread."));

		PR_JoinThread(mDlCoreThread);

		PR_Sleep(EXITDLCORE_DELAY);

		mShellBase.sendNotifyMessage(std::string("nsPluginInstance::releaseDownloadCore -> Delete Downloading Thread."));

		mDlCoreShutdownThread = false;
		mDlCoreThread = NULL;
	}
	
	return true;
}


///////////////////////////////////////////////////////////////////////////////////////
//
//
//		Funzioni Draw Finestra
//
//
//////////////////////////////////////////////////////////////////////////////////////

#if defined(WIN32)

//////////////////////////////////////
//
// Plug-In Event Handler Function
//
static LRESULT CALLBACK PluginWinProc(HWND hWnd, UINT eventmsg, WPARAM wParam, LPARAM lParam)
{
	LRESULT lr;

	nsPluginInstance *plugin = (nsPluginInstance *)(LONG_PTR) GetWindowLong(hWnd, GWL_USERDATA);

	switch (eventmsg) 
	{
	case WM_TIMER:
		if (plugin)
			if(plugin->checkRunning())
				plugin->doRenderFrame();
		break;
    case WM_PAINT: //TODO: finire
		{
			if (!plugin)
			{
				PAINTSTRUCT ps;
				HDC hdc = BeginPaint(hWnd, &ps);
				RECT rc;
				GetClientRect(hWnd, &rc);
				FrameRect(hdc, &rc, GetStockBrush(BLACK_BRUSH));
				std::string errormsg("OSG4Web Instance Error: Windows pointer error");
				DrawText(hdc, errormsg.c_str(), errormsg.length(), &rc, DT_SINGLELINE | DT_CENTER | DT_VCENTER);
				EndPaint(hWnd, &ps);

				return 0;
			}
			else
			{
				if( plugin->checkErrorPresence() )
				{
					PAINTSTRUCT ps;
					HDC hdc = BeginPaint(hWnd, &ps);
					RECT rc;
					GetClientRect(hWnd, &rc);
					FrameRect(hdc, &rc, GetStockBrush(BLACK_BRUSH));
					std::string errormsg("OSG4Web Instance Error: " + s_PluginMessageError);
					DrawText(hdc, errormsg.c_str(), errormsg.length(), &rc, DT_SINGLELINE | DT_CENTER | DT_VCENTER);	
					EndPaint(hWnd, &ps);

					return 0;
				}
			}

			//return 0;
		}
		break;
	case WM_ERASEBKGND: //Corregge il problema di flickering durante il ridimensionamento e la selezione
		if (plugin)
			if(plugin->checkRunning())
			{
				return 0;
			}
		break;
/*	case WM_KEYUP: //FIXME: non ne grabba nessuno... non vengono passati dall'handler di firefox
		Beep(1000,100);
		break;
	case WM_KEYDOWN:
		Beep(1000,100);
		break;

	case WM_NCACTIVATE:
		Beep(1000,100);
		break;
	case WM_SETFOCUS:
		Beep(1000,100);
		break;
	case WM_KILLFOCUS:
		Beep(1000,100);
		break;
*/
	default:
		break;
	}
 
	//forward windows messages to producer
	lr = DefWindowProc(hWnd, eventmsg, wParam, lParam);

	return lr;
}

#else

//TODO:linux
//FIXME: da rifare completamente

//////////////////////////////////////
//
// Plug-In Event Handler Function
//
static void xt_event_handler(Widget xtwidget, nsPluginInstance *plugin, XEvent *xevent, Boolean *b)
{
	switch (xevent->type) 
	{
	case MapNotify:
		if(plugin->GetTimerId() != 0)
		{
			XtRemoveWorkProc( plugin->GetTimerId() );
			plugin->SetTimerId( 0 );
		}
        	
		plugin->SetTimerId( XtAppAddTimeOut( XtWidgetToApplicationContext( plugin->GetWidget()), plugin->GetTimerId(), xt_timer_draw_proc, plugin ) );
		break;
	case UnmapNotify: //UnMap Event: stop the rendering
		if(plugin->GetTimerId() != 0)
		{
			XtRemoveWorkProc( plugin->GetTimerId() );
			plugin->SetTimerId( 0 );
		}
		break;
	default:
		break;
	}
}

//////////////////////////////////////
//
// Timer event function
//
static void xt_timer_draw_proc( XtPointer clientdata, XtIntervalId* id )
{
	nsPluginInstance* plugin = (nsPluginInstance*) clientdata;
	if(plugin)
	{
		plugin->Draw();
		
		if( !plugin->GetSceneInstances() )
		{
			//TODO: mandare un messaggio di errore sulla schermata principale
		}
		
		plugin->SetTimerId( XtAppAddTimeOut( XtWidgetToApplicationContext(plugin->GetWidget()), plugin->getRenderingTimer(), xt_timer_draw_proc, plugin ) );
	}
}

//////////////////////////////////////
//
// Creates X11 SubWindow and sets the events functions
//
NPError nsPluginInstance::SetWindow(NPWindow* aWindow)
{
	if(aWindow == NULL)
		return FALSE;

	mX = aWindow->x;
	mY = aWindow->y;
	mWidth = aWindow->width;
	mHeight = aWindow->height;
  
	if (mhWnd != (Window) aWindow->window) 
	{
	 	mhWnd = (Window) aWindow->window;
		NPSetWindowCallbackStruct *ws_info = (NPSetWindowCallbackStruct *)aWindow->ws_info;
  
		mDisplay = ws_info->display;
		mVisual = ws_info->visual;
		mDepth = ws_info->depth;
		mColormap = ws_info->colormap;

		// add xt event handler
		Widget xtwidget = XtWindowToWidget(mDisplay, mhWnd);
		if (xtwidget && mXtwidget != xtwidget) 
		{
			mXtwidget = xtwidget;
			long event_mask = ExposureMask;
			XSelectInput(mDisplay, mhWnd, event_mask);
			XtAddEventHandler(xtwidget, event_mask, false, (XtEventHandler)xt_event_handler, this);
		}
	}
		
	return true;
}

//////////////////////////////////////
//
// Start the SceneGraph rendering 
//
#endif

bool nsPluginInstance::checkRunning()
{
	if(!mShellBase.isRunning())
		return false;
	else
		return true;
}

bool nsPluginInstance::checkErrorPresence()
{
	if(mShellBase.isThereErrors())
		return true;
	
	return false;
}


// ==============================
// ! Scriptability related code !
// ==============================

//////////////////////////////////////
//
// this method will return the scriptable object (and create it if necessary)
//
nsScriptablePeer* nsPluginInstance::getScriptablePeer()
{
  	if (!mScriptablePeer) 
  	{
    		mScriptablePeer = new nsScriptablePeer(this);
    		if(!mScriptablePeer)
     			return NULL;

    		NS_ADDREF(mScriptablePeer);
  	}

  	// add reference for the caller requesting the object
  	NS_ADDREF(mScriptablePeer);
  	return mScriptablePeer;
}



//////////////////////////////////////
//
// Funzione di trasporto dei comandi da JavaScript al Core
//
std::string nsPluginInstance::execShellCommand(std::string line )
{
	mShellBase.sendNotifyMessage(std::string("nsPluginInstance::execShellCommand -> Sending Shell Command."));
	return mShellBase.execShellCommand(line);
}

std::string nsPluginInstance::execCoreCommand(std::string line )
{
	std::string ret;

	mShellBase.sendNotifyMessage(std::string("nsPluginInstance::execCoreCommand -> Sending Core Command."));

	//Return if loading is in progress
	if(mLoading)
	{
		mShellBase.sendWarnMessage(std::string("nsPluginInstance::execCoreCommand -> Loading in progress, Command discarded."));
		return std::string("COMMAND_DISCARDED");
	}

	//Richiedo CS sul Main Rendering Thread
	PR_Lock(s_ThreadLocking);
	ret = mShellBase.execCoreCommand(line);

	//Esco CS
	PR_Unlock(s_ThreadLocking);

	return ret;
}
