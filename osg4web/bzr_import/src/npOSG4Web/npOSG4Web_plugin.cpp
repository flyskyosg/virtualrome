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
#include "xpcom-config.h"

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

#include <npOSG4Web/npOSG4Web_Defines.h>
#include <npOSG4Web/npOSG4Web_plugin.h>


/***************************************************************************
 *
 * General initialization and shutdown
 *
 ***************************************************************************/
NPError NS_PluginInitialize()
{
  	return NPERR_NO_ERROR;
}

void NS_PluginShutdown()
{

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


/***************************************************************************
 *
 * Construction and destruction of our plugin instance object
 *
 ***************************************************************************/

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

void NS_DestroyPluginInstance(nsPluginInstanceBase * aPlugin)
{
  	if(aPlugin)
    		delete (nsPluginInstance *)aPlugin;
}



/***************************************************************************
 *
 * nsPluginInstance class implementation
 *
 ***************************************************************************/

nsPluginInstance::nsPluginInstance(NPP aInstance) : nsPluginInstanceBase(),
  	mInstance(aInstance),
  	mInitialized(false),
	mInitOptionsSet(false),
	mScriptablePeer(NULL),
	mLoading(true),
#if defined(WIN32)
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

	std::string tempstr, tempstr2;
	std::string* reftempstr = new std::string;

	nsresult rv = getProxySettings(tempstr, tempstr2);
	if(rv != NS_OK)
	{
		mShellBase.setErrorCode( 40 );
		mShellBase.sendWarnMessage("nsPluginInstance::nsPluginInstance -> " + mShellBase.getErrorString());
		delete reftempstr;
		return;
	}

	if(!tempstr.empty())
	{
		mShellBase.setInitOption(INIT_OPTION_PROXYHNAME, tempstr);
		mShellBase.setInitOption(INIT_OPTION_PROXYHPORT, tempstr2);
	}
	
	rv = getCurrPlugDir(reftempstr, NPOSG4WEB_DIRECTORY);
	if(rv != NS_OK)
	{
		mShellBase.setErrorCode( 41 );
		mShellBase.sendWarnMessage("nsPluginInstance::nsPluginInstance -> " + mShellBase.getErrorString());
		delete reftempstr;
		return;
	}

	reftempstr->append(APPEND_NPOSG4WEB_DIRECTORY);
	mShellBase.setInitOption(INIT_OPTION_INSTALLDIR, *reftempstr);
	reftempstr->clear();

	rv = getCurrPlugDir(reftempstr, NPOSG4WEB_DIRECTORY_LOCAL);
	if(rv != NS_OK)
	{
		mShellBase.setErrorCode( 42 );
		mShellBase.sendWarnMessage("nsPluginInstance::nsPluginInstance -> " + mShellBase.getErrorString());
		delete reftempstr;
		return;
	}

	reftempstr->append(APPEND_NPOSG4WEB_DIRECTORY);
	mShellBase.setInitOption(INIT_OPTION_LOCALINSTDIR, *reftempstr);
	reftempstr->clear();

	rv = getCurrPlugDir(reftempstr, NS_OS_TEMP_DIR);
	if(rv != NS_OK)
	{
		mShellBase.setErrorCode( 43 );
		mShellBase.sendWarnMessage("nsPluginInstance::nsPluginInstance -> " + mShellBase.getErrorString());
		delete reftempstr;
		return;
	}

	reftempstr->append(APPEND_TEMP_DIRECTORY);
	mShellBase.setInitOption( INIT_OPTION_TEMPDIR, *reftempstr);
	reftempstr->clear();

#if defined(NPOSG4WEB_COREDIR_PLUGIN) //Se settata imposto la directory di lavoro dei core in plugin di firefox
	rv = getCurrPlugDir(reftempstr, NPOSG4WEB_DIRECTORY); 
#else
	rv = getCurrPlugDir(reftempstr, COREINSTALL_DIRECTORY);
#endif

	if(rv != NS_OK)
	{
		mShellBase.setErrorCode( 44 );
		mShellBase.sendWarnMessage("nsPluginInstance::nsPluginInstance -> " + mShellBase.getErrorString());
		delete reftempstr;
		return;
	}

#if !defined(NPOSG4WEB_COREDIR_PLUGIN) //Se settata imposto la directory di lavoro dei core in plugin di firefox
	reftempstr->append(APPEND_CORES_DIRECTORY);
#else
	reftempstr->append(APPEND_NPOSG4WEB_DIRECTORY);
#endif
	
	mShellBase.setInitOption( INIT_OPTION_COREINSTDIR, *reftempstr );
	reftempstr->clear();

	//Check dei parametri iniziali
	mShellBase.sendNotifyMessage("nsPluginInstance::nsPluginInstance -> configuring ShellBase with firefox settings");
	mInitOptionsSet = mShellBase.configuringInitialOptions();
}

//Distructor
nsPluginInstance::~nsPluginInstance()
{
  	mShellBase.sendNotifyMessage("nsPluginInstance::~nsPluginInstance -> default destructor");

	if(mScriptablePeer)
	{
  		mScriptablePeer->SetInstance(NULL);
	  	NS_IF_RELEASE(mScriptablePeer);
	}
}

#if defined(WIN32)
static LRESULT CALLBACK PluginWinProc(HWND, UINT, WPARAM, LPARAM);
#else
//TODO: linux
//FIXME: da rifare completamente
static void xt_event_handler(Widget xtwidget, nsPluginInstance *plugin, XEvent *xevent, Boolean *b);
static void xt_timer_draw_proc(XtPointer clientdata, XtIntervalId* id);
#endif


// Set Parameter from plug-In base. 
// Estract the plugins arguments from argx argv argn passed by firefox
NPError nsPluginInstance::SetInitialData(PRUint16 argc, char* argn[], char* argv[])
{
	NPError rv = NPERR_NO_ERROR;

	mShellBase.sendNotifyMessage("nsPluginInstance::SetInitialData -> getting plugin HTML initial parameters.");

	if (argn != NULL && argv != NULL) 
	{
		for (int i = 0; i < argc; i++)
		{
			if(argn[i] != NULL)
			{
				std::string command(argn[i]);
				if(argv[i] != NULL)
				{
					std::string value(argv[i]);
					mShellBase.setObjectOption(command, value);
				}
			}
		}
	}
	else
		return NPERR_GENERIC_ERROR;

	return rv;
}

// Gets Proxy Parameters from Firefox Settings
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


// Return plugin current directory 
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

	if(!mInitOptionsSet)
	{
		mShellBase.sendWarnMessage("nsPluginInstance::init -> Error retrieving inititialization options.");
		return mInitialized;
	}

	if(aWindow == NULL)
	{
		mShellBase.setErrorCode( 46 ); 
		mShellBase.sendWarnMessage("nsPluginInstance::init -> " + mShellBase.getErrorString());
		return mInitialized;
	}

#if defined(WIN32)
	mhWnd = (HWND)aWindow->window;
	if(mhWnd == NULL)
	{
		mShellBase.setErrorCode( 45 ); 
		mShellBase.sendWarnMessage("nsPluginInstance::init -> " + mShellBase.getErrorString() );
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

	return true;
}

NPBool nsPluginInstance::initLoader()
{
	mInitialized = true;

	mShellBase.sendNotifyMessage("nsPluginInstance::initLoader -> starting the Loading Core.");
	if(!mShellBase.startLoadingBaseCore())
		return false;

	mLoading = false;

	if(!mShellBase.initializeAdvancedCore())
	{
		//Reload Core Loader
		if(!mShellBase.startLoadingBaseCore())
		{
			mShellBase.sendWarnMessage(std::string("nsPluginInstance::initLoader -> ") + mShellBase.getErrorString());
			return false;
		}

		mShellBase.sendWarnMessage(std::string("nsPluginInstance::initLoader -> Initializing Advanced Core Failed!"));
	}
	
	return mInitialized;
}


//////////////////////////////////////
//
// Chiude l'istanza plug-In
//
void nsPluginInstance::shut()
{
	std::string ogltest;

	mShellBase.sendNotifyMessage(std::string("nsPluginInstance::shut -> Shutting down plugin instance"));

	mLoading = true;

	mShellBase.forcingShutDown();
	
	if(!mShellBase.closeAllLibraries())
		mShellBase.sendWarnMessage(std::string("nsPluginInstance::shut -> Error closing all libraries."));
	
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


/* FUNZIONI PER GESTIRE IN MODALITA' WINDOWLESS
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

//FIXME: finire la gestione degli handler (forse togliere)

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
//		Funzioni Draw Finestra
//
//
//////////////////////////////////////////////////////////////////////////////////////

#if defined(WIN32)

LRESULT nsPluginInstance::handleWindowEvents(HWND hWnd, UINT eventmsg, WPARAM wParam, LPARAM lParam)
{
	switch (eventmsg) 
	{
#if (_WIN32_WINNT >= 0x0400) || (_WIN32_WINDOWS > 0x0400)
		case WM_MOUSEWHEEL:
#endif
		case WM_LBUTTONDOWN:
		case WM_LBUTTONUP:
		case WM_LBUTTONDBLCLK:
		case WM_RBUTTONDOWN:
		case WM_RBUTTONUP:
		case WM_RBUTTONDBLCLK:
		case WM_MBUTTONDOWN:
		case WM_MBUTTONUP:
		case WM_MBUTTONDBLCLK:
			SetFocus(hWnd);
			break;
	case WM_PAINT:
		{
			if( this->showFatalCoreErrors() )
			{
				PAINTSTRUCT ps;
				HDC hdc = BeginPaint(hWnd, &ps);
				RECT rc;
				GetClientRect(hWnd, &rc);
				FrameRect(hdc, &rc, GetStockBrush(BLACK_BRUSH));
				std::string errormsg("OSG4Web Instance Error: " + mShellBase.getErrorString() );
				DrawText(hdc, errormsg.c_str(), errormsg.length(), &rc, DT_SINGLELINE | DT_CENTER | DT_VCENTER);	
				EndPaint(hWnd, &ps);

				return (0L);
			}
// FIXME: finire la parte di request Explicit
//			else 
//				this->requestExplicitRendering(); 
		}
		break;
//	case WM_SIZE:
//		{
//			this->requestExplicitRendering();
//		}
//		break;
	case WM_ERASEBKGND:
		if(this->checkRunning()) //Corregge il problema di flickering durante il ridimensionamento e la selezione
		{
			//this->requestExplicitRendering();
			return (0L); //Non passo l'erase signal
		}
		break;
	default:
		break;
	}

	return (::DefWindowProc(hWnd, eventmsg, wParam, lParam));
}



//////////////////////////////////////
//
// Plug-In Event Handler Function
//
static LRESULT CALLBACK PluginWinProc(HWND hWnd, UINT eventmsg, WPARAM wParam, LPARAM lParam)
{
	nsPluginInstance *plugin = (nsPluginInstance *)(LONG_PTR) GetWindowLong(hWnd, GWL_USERDATA);

	if (!plugin)
	{
		switch (eventmsg) 
		{
		case WM_MOVE:
		case WM_SIZE:
		case WM_PAINT:
			{
				std::string errormsg("OSG4Web Instance Error: Plugin has not been initialized");

				PAINTSTRUCT ps;
				RECT rc;
				HDC hdc = BeginPaint(hWnd, &ps);

				GetClientRect(hWnd, &rc);
				FrameRect(hdc, &rc, GetStockBrush(BLACK_BRUSH));
				DrawText(hdc, errormsg.c_str(), errormsg.length(), &rc, DT_SINGLELINE | DT_CENTER | DT_VCENTER);
				EndPaint(hWnd, &ps);
			}
			break;
		default:
			break;
		}
	}
	else
		return plugin->handleWindowEvents(hWnd, eventmsg, wParam, lParam);

	return (::DefWindowProc(hWnd, eventmsg, wParam, lParam)); //Passo il default alla window sopra
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

#endif


/************************************************************************
 *
 *		Rendering auxiliar functions
 *
 ************************************************************************/

bool nsPluginInstance::checkRunning()
{
	if(!mShellBase.isRunning())
		return false;
	else
		return true;
}

void nsPluginInstance::requestExplicitRendering()
{
	mShellBase.requestExplicitRendering();
}

bool nsPluginInstance::showFatalCoreErrors()
{
	return ( mShellBase.isThereErrors() && !checkRunning());
}



/*********************************************************************************
 *
 *		==============================
 *		! Scriptability related code !
 *		==============================
 *
 * this method will return the scriptable object (and create it if necessary)
 *
 *********************************************************************************/
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


/******************************************************************************
 *
 * Funzione di trasporto dei comandi da JavaScript al ShellBase
 *
 ******************************************************************************/

std::string nsPluginInstance::execShellCommand(std::string line )
{
	mShellBase.sendNotifyMessage(std::string("nsPluginInstance::execShellCommand -> Sending Shell Command."));
	return mShellBase.execShellCommand(line);
}

std::string nsPluginInstance::execCoreCommand(std::string line )
{
	mShellBase.sendNotifyMessage(std::string("nsPluginInstance::execCoreCommand -> Sending Core Command."));

	//Return if loading is in progress
	if(mLoading)
	{
		mShellBase.sendWarnMessage(std::string("nsPluginInstance::execCoreCommand -> Loading in progress, Command discarded."));
		return std::string("COMMAND_DISCARDED");
	}

	return mShellBase.execCoreCommand(line);
}


//////////////////////////////////////
//
// Funzione di trasporto dei comandi da ShellBase  a Javascript
//
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