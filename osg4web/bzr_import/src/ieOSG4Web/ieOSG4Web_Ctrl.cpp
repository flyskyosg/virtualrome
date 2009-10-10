//c++ Macro Structures
#include <string>
#include <sstream>
#include <iostream>
#include <vector>

//Inclusione STDafx
#include <ieOSG4Web/ieOSG4Web_stdafx.h>

//OSG4Web main includes
#include <ieOSG4Web/ieOSG4Web_Defines.h>
#include <ieOSG4Web/ieOSG4Web.h>
#include <ieOSG4Web/ieOSG4Web_Ctrl.h>
#include <ieOSG4Web/ieOSG4Web_PropPage.h>

//Safety Proclamation
#include <ieOSG4Web/ieOSG4Web_Helpers.h>
#include <objsafe.h>
#include <assert.h>

//cURL downloading library
#include <curl/curl.h>

//Richiesta del Service manager
#include <Mshtml.h>




////////////////////////////////////////////////////////////////////////
//
//		Preparazione ActiveX
//
////////////////////////////////////////////////////////////////////////

// IMPLEMENT_DYNCREATE
IMPLEMENT_DYNCREATE(ieOSG4Web_Ctrl, COleControl)


// Message map
BEGIN_MESSAGE_MAP(ieOSG4Web_Ctrl, COleControl)
	ON_OLEVERB(AFX_IDS_VERB_PROPERTIES, OnProperties)
	ON_WM_CREATE()
	ON_WM_PAINT()
END_MESSAGE_MAP()


// Dispatch map
BEGIN_DISPATCH_MAP(ieOSG4Web_Ctrl, COleControl)
	DISP_FUNCTION_ID(ieOSG4Web_Ctrl, "ExecCoreCommand", dispidExecCoreCommand, ExecCoreCommand, VT_BSTR, VTS_BSTR)
	DISP_FUNCTION_ID(ieOSG4Web_Ctrl, "ExecShellCommand", dispidExecShellCommand, ExecShellCommand, VT_BSTR, VTS_BSTR)
END_DISPATCH_MAP()


// Event map
BEGIN_EVENT_MAP(ieOSG4Web_Ctrl, COleControl)
	EVENT_CUSTOM_ID("GetEvent", eventidGetEvent, GetEvent, VTS_BSTR)
END_EVENT_MAP()


// Property pages
BEGIN_PROPPAGEIDS(ieOSG4Web_Ctrl, 1)
	PROPPAGEID(PropPage::guid)
END_PROPPAGEIDS(ieOSG4Web_Ctrl)


// Initialize class factory and guid
IMPLEMENT_OLECREATE_EX(ieOSG4Web_Ctrl, "ieOSG4Web.ActiveXControl",
	0x559f0dcd, 0x759e, 0x48ee, 0xb1, 0xf2, 0xc9, 0x17, 0xaa, 0x6f, 0xb7, 0xea)


// Type library ID and version
IMPLEMENT_OLETYPELIB(ieOSG4Web_Ctrl, _tlid, _wVerMajor, _wVerMinor)


// Interface IDs
const IID BASED_CODE IID_DieOSG4Web =      
	{ 0xF72EBA29, 0xBC35, 0x4001, 
		{ 0xB2, 0x91, 0x1A,  0x8, 0x51, 0xF3, 0x48, 0x88 } 
	};

const IID BASED_CODE IID_DieOSG4WebEvents = 
	{ 0xAFFD9D00, 0x91CC, 0x468D, 
		{ 0x84, 0xC5, 0x19, 0xF1, 0x5F, 0xFC, 0xFD, 0x90 } 
	};


// Control type information
static const DWORD BASED_CODE _dwieOSG4WebOleMisc =
	OLEMISC_ACTIVATEWHENVISIBLE |
	OLEMISC_SETCLIENTSITEFIRST |
	OLEMISC_INSIDEOUT |
	OLEMISC_CANTLINKINSIDE |
	OLEMISC_RECOMPOSEONRESIZE;


// IMPLEMENT_OLECTLTYPE
IMPLEMENT_OLECTLTYPE(ieOSG4Web_Ctrl, IDS_IEOSG4WEB, _dwieOSG4WebOleMisc)




////////////////////////////////////////////////////////////////////////
//
//		Parametri Globali
//
////////////////////////////////////////////////////////////////////////


//Thread locking state
static PRLock* s_ThreadLocking = NULL;

//Downloading Core Thread locking state
static PRLock* s_DlCoreThreadLocking = NULL;

//Main Error Message
std::string s_PluginMessageError;



////////////////////////////////////////////////////////////////////////
//
//		Classe ieOSG4Web_Ctrl: Inizializzazione
//
////////////////////////////////////////////////////////////////////////

 ieOSG4Web_Ctrl::ieOSG4Web_Ctrl() :
#if defined(RENDER_WITH_TIMER_EVENT)
	m_Timer(RENDER_DELAY),
#endif
	m_FireMethod( SEND_ACTION_BY_CATCH ),
	m_ProxyEnabled( false ),
	m_ProxyName( "" ),
	m_ProxyPort( "8080" ),
	m_Initialized(false),
	m_ADVCore( "" ),
	m_ADVCoreSHA1Hash( "" ),
	m_ADVInitOptions( "useOriginalExternalReferences noLoadExternalReferenceFiles" ),
	m_ADVStartOptions( "" ),
	m_LoadCBOptions( "useOriginalExternalReferences noLoadExternalReferenceFiles" ),
	m_InstDir( "" ),
	m_TempDir( "" ),
	m_CoreInstallDir( "" ),
	m_Thread(NULL),
	m_ShutdownThread(false),
	m_DlCoreThread(NULL),
	m_DlCoreShutdownThread(false),
	m_Loading(false)
{
	InitializeIIDs(&IID_DieOSG4Web, &IID_DieOSG4WebEvents);

	m_ShellBase.initializeLog("ieOSG4Web");

	m_ShellBase.sendNotifyMessage("ieOSG4Web_Ctrl::ieOSG4Web_Ctrl -> Default Constructor");
	
	m_ShellBase.sendNotifyMessage("ieOSG4Web_Ctrl::ieOSG4Web_Ctrl -> Thread Locking Initialization");
	s_ThreadLocking = PR_NewLock();
	assert(s_ThreadLocking);

	s_DlCoreThreadLocking = PR_NewLock();
	assert(s_DlCoreThreadLocking);

	m_ShellBase.sendNotifyMessage("ieOSG4Web_Ctrl::ieOSG4Web_Ctrl -> Discovering Internet Explorer settings");

	m_InstDir = (LPCTSTR) GetInstallDir();
	if(m_InstDir.empty())
	{
		s_PluginMessageError = "retrieve plugin directory failed!";
		m_ShellBase.sendWarnMessage("nsPluginInstance::nsPluginInstance -> retrieve plugin directory failed!");
		return;
	}
	
	m_TempDir = (LPCTSTR) GetTempDir();
	if(m_TempDir.empty())
	{
		s_PluginMessageError = "retrieve temp directory failed!";
		m_ShellBase.sendWarnMessage("nsPluginInstance::nsPluginInstance -> retrieve temp directory failed!");
		return;
	}
	
	m_CoreInstallDir = (LPCTSTR) GetInstallCoreDir();
	if(m_CoreInstallDir.empty())
	{
		s_PluginMessageError = "retrieve core directory failed!";
		m_ShellBase.sendWarnMessage("nsPluginInstance::nsPluginInstance -> retrieve core directory failed!");
		return;
	}

	//Adding Extra Path
	m_TempDir.append(APPEND_TEMP_DIRECTORY);
	m_CoreInstallDir.append(APPEND_CORES_DIRECTORY);

	m_ShellBase.sendNotifyMessage("ieOSG4Web_Ctrl::ieOSG4Web_Ctrl -> configuring ShellBase with Internet Explorer settings");

	if(!m_ShellBase.setInstallDirectory(m_InstDir))
	{
		s_PluginMessageError = m_ShellBase.getErrorString();
		m_ShellBase.sendWarnMessage("ieOSG4Web_Ctrl::ieOSG4Web_Ctrl -> setting install directory failed!");
		return;
	}

	if(!m_ShellBase.setTempDirectory(m_TempDir))
	{
		s_PluginMessageError = m_ShellBase.getErrorString();
		m_ShellBase.sendWarnMessage("ieOSG4Web_Ctrl::ieOSG4Web_Ctrl -> setting temp directory failed!");
		return;
	}
		
	if(!m_ShellBase.setCoreAppDirectory(m_CoreInstallDir))
	{
		s_PluginMessageError = m_ShellBase.getErrorString();
		m_ShellBase.sendWarnMessage("ieOSG4Web_Ctrl::ieOSG4Web_Ctrl -> setting core install directory failed!");
		return;
	}

	this->GetProxyConfiguration();
	if( m_ProxyEnabled && m_ProxyName.GetLength()!= 0 )
	{
		m_ShellBase.setProxyParameters((LPCTSTR) m_ProxyName, (LPCTSTR) m_ProxyPort);
	}
}  

ieOSG4Web_Ctrl::~ieOSG4Web_Ctrl()
{
	m_ShellBase.sendNotifyMessage("ieOSG4Web_Ctrl::~ieOSG4Web_Ctrl -> default destructor");

	m_ShellBase.sendNotifyMessage("ieOSG4Web_Ctrl::ieOSG4Web_Ctrl -> Thread Locking destruction");
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

void ieOSG4Web_Ctrl::DoPropExchange(CPropExchange* pPX) //TODO: finire le PX_String
{
	ExchangeVersion(pPX, MAKELONG(_wVerMinor, _wVerMajor));
	COleControl::DoPropExchange(pPX);

	//Prende i parametri dai PARAM di OBJECT
	PX_String( pPX, "ADVCore", m_ADVCore, "" );
	PX_String( pPX, "ADVCoreStartOptions", m_ADVStartOptions, "" );
	PX_String( pPX, "ADVCoreInitOptions", m_ADVInitOptions, "" );
	PX_String( pPX, "ADVCoreSHA1", m_ADVCoreSHA1Hash, "" );
	PX_String( pPX, "LoadCoreOptions", m_LoadCBOptions, "" );
}

void ieOSG4Web_Ctrl::OnResetState() 
{
	COleControl::OnResetState();
	COleControl::ResetStockProps();

	m_ADVCore.Empty();
	m_ADVStartOptions.Empty();
	m_ADVInitOptions.Empty();
	m_ADVCoreSHA1Hash.Empty();
	m_LoadCBOptions.Empty();
}

void ieOSG4Web_Ctrl::OnClose(DWORD dwSaveOptions)
{
	m_ShellBase.sendNotifyMessage(std::string("ieOSG4Web_Ctrl::OnClose -> Shutting down plugin instance"));

	this->releaseDownloadCore();
	
	if(!m_ShellBase.closeAllLibraries())
	{
		m_ShellBase.sendWarnMessage(std::string("ieOSG4Web_Ctrl::OnClose -> Error closing all libraries."));
		s_PluginMessageError = m_ShellBase.getErrorString(); //TODO: correggere il messaggio
	}
	
	m_ShellBase.sendNotifyMessage(std::string("ieOSG4Web_Ctrl::OnClose -> Shutting down finished"));

	m_Initialized = false;

	COleControl::OnClose(dwSaveOptions);
}

int ieOSG4Web_Ctrl::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	m_ShellBase.sendNotifyMessage("ieOSG4Web_Ctrl::OnCreate -> Initializing Window.");

	if (COleControl::OnCreate(lpCreateStruct) == -1)
	{
		s_PluginMessageError = "An error occurs during instance creation";
		m_ShellBase.sendWarnMessage("ieOSG4Web_Ctrl::OnCreate -> initializing derived instance failed!");
		return -1;
	}

	//Linking Plugin to ShellBase
	m_ShellBase.sendNotifyMessage("ieOSG4Web_Ctrl::OnCreate -> Linking Plugin to ShellBase.");

	m_ShellBase.setWindowHandler( GetSafeHwnd() );
	m_ShellBase.setInstanceHandler( this );
	m_ShellBase.setResetWindowHandler( &ieOSG4Web_Ctrl::callResetWindowHandler );
	m_ShellBase.setTransportEventHandler( &ieOSG4Web_Ctrl::TransportEvent );
	m_ShellBase.setRenderingHandler( &ieOSG4Web_Ctrl::callPrepareRendering, &ieOSG4Web_Ctrl::callCloseRendering );
	m_ShellBase.setDownloadingCoreHandler( &ieOSG4Web_Ctrl::callRequestCoreDownloading );

	m_Initialized = true;

	m_ShellBase.sendNotifyMessage("ieOSG4Web_Ctrl::OnCreate -> setting Core Options.");

	std::string str("LOADCORE_OPTIONS=");

	if(!m_LoadCBOptions.IsEmpty())
	{
		str += (LPCTSTR) m_LoadCBOptions;

		if(m_ShellBase.execShellCommand(str) != std::string("DONE"))
		{
			s_PluginMessageError = "ShellBase - setting loading core options failed!";
			m_ShellBase.sendWarnMessage("ieOSG4Web_Ctrl::OnCreate -> setting loading core options failed.");
			return -1;
		}
	}

	//Set Advanced core through Init Parameters
	if(!m_ADVCore.IsEmpty())
	{
		str = "SET_COREADV=";
		str += (LPCTSTR) m_ADVCore;
	
		if(m_ShellBase.execShellCommand(str) != std::string("DONE"))
		{
			s_PluginMessageError = std::string("ShellBase - setting ") + std::string((LPCTSTR) m_ADVCore) + std::string(" failed!");
			m_ShellBase.sendWarnMessage(std::string("ieOSG4Web_Ctrl::OnCreate -> ") + std::string("setting ") + std::string( (LPCTSTR) m_ADVCore)  + std::string(" failed!"));
			return -1;
		}

		str = "SET_COREADV_INIT_OPTIONS=";
		str += (LPCTSTR) m_ADVInitOptions;

		if(m_ShellBase.execShellCommand(str) != std::string("DONE"))
		{
			s_PluginMessageError = std::string("ShellBase - setting ") + std::string((LPCTSTR) m_ADVInitOptions) + std::string(" init options failed!");
			m_ShellBase.sendWarnMessage(std::string("ieOSG4Web_Ctrl::OnCreate -> ") + std::string("setting ") + std::string( (LPCTSTR) m_ADVInitOptions) + std::string(" init options failed!"));
			return -1;
		}

		str = "SET_COREADV_START_OPTIONS=";
		str += (LPCTSTR) m_ADVStartOptions;

		if(m_ShellBase.execShellCommand(str) != std::string("DONE"))
		{
			s_PluginMessageError = std::string("ShellBase - setting ") + std::string((LPCTSTR) m_ADVStartOptions) + std::string(" start options failed!");
			m_ShellBase.sendWarnMessage(std::string("ieOSG4Web_Ctrl::OnCreate -> ") + std::string("setting ") + std::string( (LPCTSTR) m_ADVStartOptions ) + std::string(" start options failed!"));
			return -1;
		}

		str = "SET_COREADV_SHA1HASH=";
		str += (LPCTSTR) m_ADVCoreSHA1Hash;

		if(m_ShellBase.execShellCommand(str) != std::string("DONE"))
		{
			s_PluginMessageError = std::string("ShellBase - setting SHA-1 Hash string failed!");
			m_ShellBase.sendWarnMessage(std::string("ieOSG4Web_Ctrl::OnCreate -> setting SHA-1 Hash string failed!"));
			return -1;
		}
	}

	m_ShellBase.sendNotifyMessage("ieOSG4Web_Ctrl::OnCreate -> starting the Loading Core.");
	if(!m_ShellBase.startLoadingBaseCore())
	{
		s_PluginMessageError = m_ShellBase.getErrorString();
		m_ShellBase.sendWarnMessage(std::string("ieOSG4Web_Ctrl::OnCreate -> ") + s_PluginMessageError);
		return -1;
	}

	//Return if CoreString is inizialized and SHA-1 isn't
	if(m_ADVCore.IsEmpty() && m_ADVCoreSHA1Hash.IsEmpty())
	{
		//Setting Error Message
		this->loadingCoreCommand("LOADCORE SETMESSAGE_COLOR LC_OSG_RED");
		this->loadingCoreCommand("LOADCORE SETMESSAGE Advanced Core SHA-1 HASH not set!");

		m_ShellBase.sendWarnMessage(std::string("ieOSG4Web_Ctrl::OnCreate -> Advanced Core SHA-1 HASH not set!"));

		return 0;
	}

	if(!m_ShellBase.initializeAdvancedCore())
	{
		//Reload LoadCore
		if(!m_ShellBase.startLoadingBaseCore())
		{
			s_PluginMessageError = m_ShellBase.getErrorString();
			m_ShellBase.sendWarnMessage(std::string("ieOSG4Web_Ctrl::OnCreate -> ") + s_PluginMessageError);
			return false;
		}

		//Setting Error Message
		this->loadingCoreCommand("LOADCORE SETMESSAGE_COLOR LC_OSG_RED");
		this->loadingCoreCommand("LOADCORE SETMESSAGE Initializing Advanced Core Failed!");

		m_ShellBase.sendWarnMessage(std::string("ieOSG4Web_Ctrl::OnCreate -> Initializing Advanced Core Failed!"));
	}
		
	return 0;
}

bool ieOSG4Web_Ctrl::checkRunning()
{
	if(!m_ShellBase.isRunning())
		return false;
	else
		return true;
}

std::string ieOSG4Web_Ctrl::loadingCoreCommand(std::string line )
{
	std::string ret;

	m_ShellBase.sendNotifyMessage(std::string("ieOSG4Web_Ctrl::loadingCoreCommand -> Sending Loading Core Command."));

	//Richiedo CS sul Main Rendering Thread
	PR_Lock(s_ThreadLocking);
	ret = m_ShellBase.execCoreCommand(line);

	//Esco CS
	PR_Unlock(s_ThreadLocking);

	return ret;
}



////////////////////////////////////////////////////////////////////////
//
//		Funzioni di invio Eventi a JavaScript
//
////////////////////////////////////////////////////////////////////////

bool ieOSG4Web_Ctrl::TransportEvent(void* classptr, std::string eventstr)
{
	ieOSG4Web_Ctrl* ctrlptr = (ieOSG4Web_Ctrl*) classptr;

	if(ctrlptr != NULL)
		return ctrlptr->SwitchSendMethod(eventstr);
	else
		ctrlptr->sendWarnMessage(std::string("ieOSG4Web_Ctrl::TransportEvent -> Instance seems not present."));

	return false;
}

bool ieOSG4Web_Ctrl::SwitchSendMethod(std::string eventstr)
{
	m_ShellBase.sendNotifyMessage("ieOSG4Web_Ctrl::SwitchSendMethod -> Switching sending type.");

	bool retval = false;

	if( m_FireMethod ) //Controlla il metodo di invio eventi - true: FireEvent
		retval = this->SendActiveXEvent(eventstr);
	else //false: GetBack Event
		retval = this->FirePluginEvent(eventstr);

	return retval;
}

bool ieOSG4Web_Ctrl::SendActiveXEvent(std::string data)
{
	m_ShellBase.sendNotifyMessage("ieOSG4Web_Ctrl::SendActiveXEvent -> Sending event using Fire ActiveX way.");

	CString cdata;
	cdata.Append(data.c_str());

	GetEvent((LPCTSTR) cdata);

	return true; //FIXME: capire se è possibile gestire il valore di ritorno
}

bool ieOSG4Web_Ctrl::FirePluginEvent(std::string data)
{
	m_ShellBase.sendNotifyMessage("ieOSG4Web_Ctrl::SendActiveXEvent -> Sending event using GetBack way.");

	CString jsfname("eventCatcher");
	CString param1(data.c_str());

	CStringArray paramArray;
	paramArray.Add(param1);

	return SendGetBackEventToJS(jsfname, paramArray);
}

bool ieOSG4Web_Ctrl::SendGetBackEventToJS(CString& jsFunctionName, CStringArray& paramArray)
{
	bool retval = false;

	CComPtr< IServiceProvider > locISP;
	CComPtr< IWebBrowser2 > locBrowser;

	m_ShellBase.sendNotifyMessage("ieOSG4Web_Ctrl::SendGetBackEventToJS -> Opening Service Provider.");

	HRESULT locHR = GetClientSite()->QueryInterface(IID_IServiceProvider, (void **)&locISP);
	if (locHR == S_OK) 
	{
		locHR = locISP->QueryService(IID_IWebBrowserApp, IID_IWebBrowser2, (void **)&locBrowser);
		if (locHR == S_OK) 
		{
			CComPtr< IDispatch > pDisp;
			locHR = locBrowser->get_Document( &pDisp );

			CComQIPtr< IHTMLDocument2 > pDoc = pDisp;
			if( pDoc )
			{
				CComPtr<IDispatch> spScript;
				locHR = pDoc->get_Script( &spScript );
				if( spScript )
				{
					CComBSTR bstrMember(jsFunctionName);
					DISPID dispid = NULL;
					HRESULT hr = spScript->GetIDsOfNames(IID_NULL, &bstrMember, 1, LOCALE_SYSTEM_DEFAULT, &dispid);
					
					if(FAILED(hr))
					{
						CString error = GetSystemErrorMessage(hr);
						m_ShellBase.sendWarnMessage(std::string("ieOSG4Web_Ctrl::SendGetBackEventToJS -> Getting Javascript Interface Error: ") + std::string( (LPCTSTR) error ));
						//::MessageBox(NULL,"ieOSG4Web Error Message",CString("ieOSG4Web_Ctrl::SendGetBackEvent\nGet Javascript Interface:\n") + error,MB_OK);
						return retval;
					}

					const int arraySize = paramArray.GetSize();

					DISPPARAMS dispparams;
					memset(&dispparams, 0, sizeof dispparams);
					dispparams.cArgs = arraySize;
					dispparams.rgvarg = new VARIANT[dispparams.cArgs];
	
					for( int i = 0; i < arraySize; i++)
					{
						CComBSTR bstr = paramArray.GetAt(arraySize - 1 - i); // back reading
						bstr.CopyTo(&dispparams.rgvarg[i].bstrVal);
						dispparams.rgvarg[i].vt = VT_BSTR;
					}
					
					dispparams.cNamedArgs = 0;

					EXCEPINFO excepInfo;
					memset(&excepInfo, 0, sizeof excepInfo);
   					CComVariant vaResult;
					UINT nArgErr = (UINT)-1;  // initialize to invalid arg
         
					hr = spScript->Invoke(dispid, IID_NULL, 0, DISPATCH_METHOD, &dispparams, &vaResult, &excepInfo, &nArgErr);

					delete [] dispparams.rgvarg;
					if(FAILED(hr))
					{
						CString error = GetSystemErrorMessage(hr);
						m_ShellBase.sendWarnMessage(std::string("ieOSG4Web_Ctrl::SendGetBackEventToJS -> Invoking Javascript Method Error: ") + std::string( (LPCTSTR) error ) );
						//::MessageBox(NULL,"ieOSG4Web Error Message",CString("ieOSG4Web_Ctrl::SendGetBackEvent\nInvoking Javascript Method:\n") + error,MB_OK);
						return retval;
					}

					retval = true;
				}
				else
					m_ShellBase.sendWarnMessage(std::string("ieOSG4Web_Ctrl::SendGetBackEventToJS -> Opening \"IDispatch\" Failed!"));
			}
			else
				m_ShellBase.sendWarnMessage(std::string("ieOSG4Web_Ctrl::SendGetBackEventToJS -> Opening \"IHTMLDocument2\" Failed!"));
		}
		else
			m_ShellBase.sendWarnMessage(std::string("ieOSG4Web_Ctrl::SendGetBackEventToJS -> Opening \"IID_IWebBrowserApp\" Failed!"));
	}
	else
		m_ShellBase.sendWarnMessage(std::string("ieOSG4Web_Ctrl::SendGetBackEventToJS -> Opening Service Provider Failed!"));

	return retval;
}


////////////////////////////////////////////////////////////////////////
//
//		Funzioni esposte a JavaScript
//
////////////////////////////////////////////////////////////////////////

void ieOSG4Web_Ctrl::GetEvent(LPCTSTR line) //Fire Event to JS
{
	m_ShellBase.sendNotifyMessage(std::string("ieOSG4Web_Ctrl::GetEvent -> Firing Event."));
	FireEvent(eventidGetEvent, EVENT_PARAM(VTS_BSTR), line);
}

BSTR ieOSG4Web_Ctrl::ExecShellCommand(LPCTSTR line )
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	CString retstr;

	m_ShellBase.sendNotifyMessage(std::string("ieOSG4Web_Ctrl::ExecShellCommand -> Sending Shell Command."));

	retstr.Append( ( m_ShellBase.execShellCommand(std::string((LPCTSTR) line)) ).c_str() );
	
	BSTR str = retstr.AllocSysString();
	return str;
}

BSTR ieOSG4Web_Ctrl::ExecCoreCommand(LPCTSTR line )
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	CString retstr;

	m_ShellBase.sendNotifyMessage(std::string("ieOSG4Web_Ctrl::ExecCoreCommand -> Sending Core Command."));

	//Return if loading is in progress
	if(m_Loading)
	{
		m_ShellBase.sendWarnMessage(std::string("ieOSG4Web_Ctrl::ExecCoreCommand -> Loading in progress, Command discarded."));
		retstr.Append("COMMAND_DISCARDED");
		
		BSTR str = retstr.AllocSysString();
		return str;
	}

	//Richiedo CS sul Main Rendering Thread
	PR_Lock(s_ThreadLocking);

	retstr.Append( ( m_ShellBase.execCoreCommand(std::string((LPCTSTR) line)) ).c_str() );

	//Esco CS
	PR_Unlock(s_ThreadLocking);

	BSTR str = retstr.AllocSysString();
	return str;
}


////////////////////////////////////////////////////////////////////////
//
//		Funzioni gestione degli Errori
//
////////////////////////////////////////////////////////////////////////

CString ieOSG4Web_Ctrl::GetSystemErrorMessage(DWORD dwError)
{
	CString strError;
	LPTSTR lpBuffer;

	if(!FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
			NULL,  dwError,
			MAKELANGID(LANG_NEUTRAL, SUBLANG_SYS_DEFAULT),
			(LPTSTR) &lpBuffer, 0, NULL))

	{
		strError = "FormatMessage Native Error" ;
	}
	else
	{
		strError = lpBuffer;
		LocalFree(lpBuffer);
	}
	return strError;
}

void ieOSG4Web_Ctrl::sendWarnMessage(std::string message)
{
	m_ShellBase.sendWarnMessage(message);
}


////////////////////////////////////////////////////////////////////////
//
//		Funzioni gestione sistema: interfacciamento a Windows
//
////////////////////////////////////////////////////////////////////////

CString ieOSG4Web_Ctrl::GetTempDir()
{
	CString TempDir = "";

	m_ShellBase.sendNotifyMessage("ieOSG4Web_Ctrl::GetTempDir -> Getting \"Temporary\" System Directory");
	TempDir.Append(m_ShellBase.getEnvironmentTempDirectory().c_str());

	return TempDir;
}

CString ieOSG4Web_Ctrl::GetInstallCoreDir()
{
	CString ICDir = "";

	m_ShellBase.sendNotifyMessage("ieOSG4Web_Ctrl::GetTempDir -> Getting \"Application Data\" System Directory");
	ICDir.Append(m_ShellBase.getEnvironmentAppDirectory().c_str());

	return ICDir;
}

CString ieOSG4Web_Ctrl::GetInstallDir()
{
	m_ShellBase.sendNotifyMessage("ieOSG4Web_Ctrl::GetInstallDir -> Retrieving Installation Directory");

	CString InstallDir = "";

	LONG res;
	HKEY hKeyCSLID;
	
	m_ShellBase.sendNotifyMessage("ieOSG4Web_Ctrl::GetInstallDir -> Opening Windows Registry");

	res = ::RegOpenKey(HKEY_CLASSES_ROOT,"CLSID", &hKeyCSLID); 
	if( res == ERROR_SUCCESS)
	{
		HKEY hKeyMyCSLID ;
		res = ::RegOpenKey(hKeyCSLID,"{559F0DCD-759E-48EE-B1F2-C917AA6FB7EA}", &hKeyMyCSLID); 
		if( res == ERROR_SUCCESS)
		{
			HKEY hKeyInProcServer ;
			res = ::RegOpenKeyEx(hKeyMyCSLID,"InprocServer32",NULL, KEY_QUERY_VALUE, &hKeyInProcServer); 
			if( res == ERROR_SUCCESS) 
			{
				char Data[2*_MAX_PATH]; 
				DWORD cbData = 2*MAX_PATH;
				res = ::RegQueryValueEx(hKeyInProcServer,NULL,NULL,NULL,(LPBYTE)Data,&cbData);
				if( cbData < 2*MAX_PATH ) // meaning success
				{
					Data[cbData]=0; // terminate the string;
	
					char drive[_MAX_DRIVE];
					char dir[_MAX_DIR];
					char fname[_MAX_FNAME];
					char ext[_MAX_EXT];
					char path[_MAX_PATH];
					_splitpath( Data, drive, dir, fname, ext );
					_makepath( path,  drive, dir, "", "" );

					// remove the eventually slash at the end
					int len = strlen(path);
					if( path[len-1] == '/' || path[len-1] == '\\' )
						path[len-1] = 0;
          
					InstallDir = path;
				}
				else
					m_ShellBase.sendWarnMessage("nsPluginInstance::GetInstallDir -> Quering \"directory\" value failed.");
			
				RegCloseKey(hKeyInProcServer);
			}
			else
				m_ShellBase.sendWarnMessage("nsPluginInstance::GetInstallDir -> Getting \"InprocServer32\" key failed.");
		
			RegCloseKey(hKeyMyCSLID);
		}
		else
			m_ShellBase.sendWarnMessage("nsPluginInstance::GetInstallDir -> Getting \"{559F0DCD-759E-48EE-B1F2-C917AA6FB7EA}\" key failed.");

		RegCloseKey(hKeyCSLID);
	}
	else
		m_ShellBase.sendWarnMessage("nsPluginInstance::GetInstallDir -> Getting \"CLSID\" key failed.");
	
	return InstallDir;
}

void ieOSG4Web_Ctrl::GetProxyConfiguration()
{
	CString ProxyString = "";
	DWORD ProxyEnable = 0; 
	char ProxyServer[2*MAX_PATH];

	m_ShellBase.sendNotifyMessage("ieOSG4Web_Ctrl::GetProxyConfiguration -> Retrieving Proxy Configuration");

	LONG res;
	HKEY hKey1; 
	m_ShellBase.sendNotifyMessage("ieOSG4Web_Ctrl::GetProxyConfiguration -> Opening Windows Registry");

	res = ::RegOpenKey(HKEY_CURRENT_USER,"Software", &hKey1); 
	if( res == ERROR_SUCCESS)
	{
		HKEY hKey2 ;
		res = ::RegOpenKey(hKey1,"Microsoft", &hKey2); 
		if( res == ERROR_SUCCESS)
		{
			HKEY hKey3;
			res = ::RegOpenKey(hKey2,"Windows", &hKey3); 
			if( res == ERROR_SUCCESS) 
			{
				HKEY hKey4;
				res = ::RegOpenKey(hKey3,"CurrentVersion", &hKey4); 
				if( res == ERROR_SUCCESS) 
				{
					HKEY hKey5;
					res = ::RegOpenKeyEx(hKey4,"Internet Settings",NULL,KEY_QUERY_VALUE, &hKey5);  
					if( res == ERROR_SUCCESS) 
					{
						DWORD cbData = 2*MAX_PATH;
						DWORD type = 0;
						//res = ::RegQueryValueEx(hKey5,"ProxyEnable",NULL,&type,(LPBYTE)(&ProxyEnable),NULL);
						res = ::RegQueryValueEx(hKey5,"ProxyEnable",NULL,&type,(LPBYTE)(&ProxyEnable),&cbData);
						if( res == ERROR_SUCCESS) // meaning success
						{
							//assert(type == REG_DWORD);
							m_ProxyEnabled = (ProxyEnable != 0);
						}
						else
								m_ShellBase.sendWarnMessage("nsPluginInstance::GetProxyConfiguration -> Quering \"ProxyEnable\" value failed.");
				
						cbData = 2*MAX_PATH;
						res = ::RegQueryValueEx(hKey5,"ProxyServer",NULL,&type,(LPBYTE)ProxyServer,&cbData);
						if( res == ERROR_SUCCESS) // meaning success
						{
							//assert(type == REG_SZ);
							ProxyServer[cbData]=0;
							CString tmp(ProxyServer);
							int pos = tmp.Find(":");
							assert( pos > 0);
							m_ProxyName = tmp.Left(pos);
							m_ProxyPort = tmp.Right( tmp.GetLength()-pos-1);
						}
						else
							m_ShellBase.sendWarnMessage("nsPluginInstance::GetProxyConfiguration -> Quering \"ProxyServer\" values failed.");

						RegCloseKey(hKey5);
					}
					else
						m_ShellBase.sendWarnMessage("nsPluginInstance::GetProxyConfiguration -> Getting \"Internet Settings\" key failed.");

					RegCloseKey(hKey4);
				}
				else
					m_ShellBase.sendWarnMessage("nsPluginInstance::GetProxyConfiguration -> Getting \"CurrentVersion\" key failed.");

				RegCloseKey(hKey3);
			}
			else
				m_ShellBase.sendWarnMessage("nsPluginInstance::GetProxyConfiguration -> Getting \"Windows\" key failed.");

			RegCloseKey(hKey2);
		}
		else
			m_ShellBase.sendWarnMessage("nsPluginInstance::GetProxyConfiguration -> Getting \"Microsoft\" key failed.");

		RegCloseKey(hKey1);
	}
	else
		m_ShellBase.sendWarnMessage("nsPluginInstance::GetProxyConfiguration -> Getting \"Software\" key failed.");
}

BOOL ieOSG4Web_Ctrl::ieOSG4Web_CtrlFactory::UpdateRegistry(BOOL bRegister)
{
	if (bRegister)
	{
#ifdef _DEBUG
		::MessageBox(NULL,"Begin Registering","Internet Explorer OSG4Web",MB_OK);
#endif

		BOOL retval = AfxOleRegisterControlClass(
			AfxGetInstanceHandle(),
			m_clsid,
			m_lpszProgID,
			IDS_IEOSG4WEB,
			IDB_IEOSG4WEB,
			afxRegApartmentThreading,
			_dwieOSG4WebOleMisc,
			_tlid,
			_wVerMajor,
			_wVerMinor);

		// mark as safe for scripting--failure OK
		HRESULT hr = CreateComponentCategory(CATID_SafeForScripting, L"Controls that are safely scriptable");

		if (SUCCEEDED(hr))
			// only register if category exists
			RegisterCLSIDInCategory(m_clsid, CATID_SafeForScripting);
			// don't care if this call fails

		// mark as safe for data initialization
		hr = CreateComponentCategory(CATID_SafeForInitializing, L"Controls safely initializable from persistent data");

		if (SUCCEEDED(hr))
			// only register if category exists
			RegisterCLSIDInCategory(m_clsid, CATID_SafeForInitializing);
			// don't care if this call fails

#ifdef _DEBUG
		::MessageBox(NULL,"Registering Done","Internet Explorer OSG4Web",MB_OK);
#endif

		return retval;
	}
	else
		return AfxOleUnregisterClass(m_clsid, m_lpszProgID);
}



///////////////////////////////////////////////////////////////////////////////////////
//
//		Funzioni di gestione finestra
//
//////////////////////////////////////////////////////////////////////////////////////

bool ieOSG4Web_Ctrl::callResetWindowHandler(void* maininst)
{
	ieOSG4Web_Ctrl *ctrlptr = (ieOSG4Web_Ctrl *) maininst;

	if(ctrlptr)
		return ctrlptr->resetWindowHandler();
	else
		ctrlptr->sendWarnMessage(std::string("ieOSG4Web_Ctrl::callResetWindowHandler -> Instance seems not present."));

	return false;
}

bool ieOSG4Web_Ctrl::resetWindowHandler()
{
	m_ShellBase.sendNotifyMessage("ieOSG4Web_Ctrl::resetWindowHandler -> resetting window handler.");

	//TODO: capire come resettare sta cazzo di Windows ID

	return true;
}


///////////////////////////////////////////////////////////////////////////////////////
//
//		Funzioni di Rendering
//
//////////////////////////////////////////////////////////////////////////////////////

void ieOSG4Web_Ctrl::callRender(void* maininst)
{
	ieOSG4Web_Ctrl *ctrlptr = (ieOSG4Web_Ctrl *) maininst;

	if(ctrlptr)
		ctrlptr->doRender();
	else
		ctrlptr->sendWarnMessage(std::string("ieOSG4Web_Ctrl::callRender -> Instance seems not present."));
}

bool ieOSG4Web_Ctrl::callPrepareRendering(void* maininst)
{
	ieOSG4Web_Ctrl *ctrlptr = (ieOSG4Web_Ctrl *) maininst;

	if(ctrlptr)
		return ctrlptr->prepareRendering();
	else
		ctrlptr->sendWarnMessage(std::string("ieOSG4Web_Ctrl::callPrepareRendering -> Instance seems not present."));

	return false;
}
	
bool ieOSG4Web_Ctrl::callCloseRendering(void* maininst)
{
	ieOSG4Web_Ctrl *ctrlptr = (ieOSG4Web_Ctrl *) maininst;

	if(ctrlptr)
		return ctrlptr->closeRendering();
	else
		ctrlptr->sendWarnMessage(std::string("ieOSG4Web_Ctrl::callCloseRendering -> Instance seems not present."));

	return false;
}

bool ieOSG4Web_Ctrl::prepareRendering()
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

	m_ShutdownThread = false;

	m_ShellBase.sendNotifyMessage(std::string("ieOSG4Web_Ctrl::prepareRendering -> Start Rendering Thread."));

	m_Thread = PR_CreateThread(PR_USER_THREAD, callRender, this,
			      PR_PRIORITY_NORMAL, PR_GLOBAL_THREAD,
			      PR_JOINABLE_THREAD, 0);

	if(m_Thread == NULL)
	{
		m_ShellBase.sendWarnMessage(std::string("ieOSG4Web_Ctrl::prepareRendering -> Rendering Thread isn't started."));
		ret = false;
	}
	else
	{
		m_ShellBase.sendNotifyMessage(std::string("ieOSG4Web_Ctrl::prepareRendering -> Rendering Thread is up and running."));
		ret = true;
	}

#endif
	return ret;
}

bool ieOSG4Web_Ctrl::closeRendering()
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
		m_ShellBase.sendNotifyMessage(std::string("ieOSG4Web_Ctrl::closeRendering -> Join Rendering Thread."));

		m_ShutdownThread = true;

		PR_JoinThread(m_Thread);

		m_ShellBase.sendNotifyMessage(std::string("ieOSG4Web_Ctrl::closeRendering -> Delete Rendering Thread."));

		PR_Sleep(EXIT_DELAY);

		m_ShutdownThread = false;
		m_Thread = NULL;
	}
	else
		return false;
#endif

	return true;
}

void ieOSG4Web_Ctrl::doRender()
{
	PR_Lock(s_ThreadLocking);

	m_ShellBase.sendNotifyMessage(std::string("ieOSG4Web_Ctrl::doRender -> Render Started."));
	
	for (;;)
	{
		//TODO: aggiungere messaggi di errore 
		m_ShellBase.doRendering();
		
		if (this->getThreadShutDown())
			break;
			
		PR_Unlock(s_ThreadLocking);

		PR_Sleep(RENDER_DELAY);

		PR_Lock(s_ThreadLocking);
	}

	m_ShellBase.sendNotifyMessage(std::string("ieOSG4Web_Ctrl::doRender -> Render Finished."));
	
	PR_Unlock(s_ThreadLocking);
	return;
}

void ieOSG4Web_Ctrl::doRenderFrame()
{
	m_ShellBase.doRendering();
}



///////////////////////////////////////////////////////////////////////////////////////
//
//		Funzioni Thread di Download
//
//////////////////////////////////////////////////////////////////////////////////////

bool ieOSG4Web_Ctrl::callRequestCoreDownloading(void* maininst)
{
	ieOSG4Web_Ctrl *ctrlptr = (ieOSG4Web_Ctrl *) maininst;

	if(ctrlptr)
		return ctrlptr->requestCoreDownloading();
	else
		ctrlptr->sendWarnMessage(std::string("ieOSG4Web_Ctrl::callRequestCoreDownloading -> Instance seems not present."));

	return false;
}

bool ieOSG4Web_Ctrl::requestCoreDownloading()
{
	bool ret = true;

	assert(this->getDlCoreThread() == NULL);

	m_DlCoreShutdownThread = false;

	m_ShellBase.sendNotifyMessage(std::string("ieOSG4Web_Ctrl::requestCoreDownloading -> Start Downloading Thread."));

	m_DlCoreThread = PR_CreateThread(PR_USER_THREAD, callDownloadCore, this,
			      PR_PRIORITY_NORMAL, PR_GLOBAL_THREAD,
			      PR_JOINABLE_THREAD, 0);

	if(m_DlCoreThread == NULL)
	{
		m_ShellBase.sendWarnMessage(std::string("ieOSG4Web_Ctrl::requestCoreDownloading -> Downloading Thread is not started."));
		ret = false;
	}
	else
	{
		m_ShellBase.sendNotifyMessage(std::string("ieOSG4Web_Ctrl::requestCoreDownloading -> Downloading Thread is up and running."));
		ret = true;
	}

	return ret;
}

void ieOSG4Web_Ctrl::callDownloadCore(void* maininst)
{
	ieOSG4Web_Ctrl *classptr = (ieOSG4Web_Ctrl *) maininst;

	if(classptr)
		classptr->doDownloadCore();
	else
		classptr->sendWarnMessage(std::string("ieOSG4Web_Ctrl::callDownloadCore -> Instance seems not present."));
}


void ieOSG4Web_Ctrl::doDownloadCore()
{
	PR_Lock(s_DlCoreThreadLocking);

	m_ShellBase.sendNotifyMessage(std::string("nsPluginInstance::doDownloadCore -> Downloading Started."));

	m_Loading = true;

	std::string fdlcoreaddress = m_ShellBase.getAdvancedCoreAddress();
	std::string fdlcorename = m_ShellBase.getAdvancedCoreFileName();
	std::string tempdl; 
	std::string tempdldir( m_TempDir );
	std::string tempheaderdl;

	tempdldir = tempdldir + "\\" + (LPCSTR) m_ADVCoreSHA1Hash;
	tempdl = tempdldir + "\\" + fdlcorename;
	tempheaderdl = tempdldir + "\\" + "headerdl.html";

	CURL* curl;
	CURLcode res;
	FILE* outfile;
	FILE* headerfile;

	std::string proxy;

	//Check Core Presence
	if(m_ShellBase.checkAdvCorePresence())
	{
		//Attivo status bar
		this->loadingCoreCommand("LOADCORE STATUSBAR_VISIBILITY FALSE");

		//Attivo il messaggio di Download del Core
		this->loadingCoreCommand("LOADCORE SETMESSAGE_COLOR LC_OSG_BLUE");
		this->loadingCoreCommand("LOADCORE SETMESSAGE Core Found...");

		PR_Sleep(2000); ////Delay der far visualizzare il messaggio

		//Loading Session
		this->loadingCoreCommand("LOADCORE STATUSBAR_VISIBILITY FALSE");
		this->loadingCoreCommand("LOADCORE SETMESSAGE "); //Spengo i messaggi

		m_ShellBase.sendNotifyMessage(std::string("ieOSG4Web_Ctrl::doDownloadCore -> Loading Unpacked Core"));

		if(!m_ShellBase.startLoadingAdvancedCore()) //Start Advanced Core Loading
		{
			m_ShellBase.sendWarnMessage(std::string("ieOSG4Web_Ctrl::doDownloadCore -> Loading advanced core failed! Starting Loading Core"));

			//Reload LoadCore
			if(!m_ShellBase.startLoadingBaseCore())
			{
				s_PluginMessageError = m_ShellBase.getErrorString();
				m_ShellBase.sendWarnMessage(std::string("ieOSG4Web_Ctrl::doDownloadCore -> Starting Loading Core failed... BYE BYE..."));
				return;
			}

			//Setting Error Message
			this->loadingCoreCommand("LOADCORE SETMESSAGE_COLOR LC_OSG_RED");
			this->loadingCoreCommand("LOADCORE SETMESSAGE Loading Advanced Core Failed!");
		}
		else
			m_ShellBase.sendNotifyMessage(std::string("ieOSG4Web_Ctrl::doDownloadCore -> Advanced Core is up and running"));

		PR_Unlock(s_DlCoreThreadLocking);

		m_Loading = false;

		return;
	}

	//Downloading Session
	if(!m_ShellBase.checkFileExistance(tempdl))
	{
		if(!m_ShellBase.checkOrCreateDirectory(tempdldir))
		{
			m_ShellBase.sendWarnMessage(std::string("ieOSG4Web_Ctrl::doDownloadCore -> Creating Downloading Temp Directory Failed."));

			this->loadingCoreCommand("LOADCORE STATUSBAR_VISIBILITY FALSE");

			this->loadingCoreCommand("LOADCORE SETMESSAGE_COLOR LC_OSG_RED");
			this->loadingCoreCommand("LOADCORE SETMESSAGE Creating Temp Directory Failed!");

			PR_Unlock(s_DlCoreThreadLocking);
			return;
		}

		curl = curl_easy_init();

		if(curl)
		{
			m_ShellBase.sendNotifyMessage(std::string("ieOSG4Web_Ctrl::doDownloadCore -> Curl Session Initialized."));

			outfile = fopen(tempdl.c_str(), "wb"); //FIXME: vedere se si possono usare gli stream
			
			if (outfile == NULL) 
			{
				this->loadingCoreCommand("LOADCORE STATUSBAR_VISIBILITY FALSE");

				this->loadingCoreCommand("LOADCORE SETMESSAGE_COLOR LC_OSG_RED");
				this->loadingCoreCommand("LOADCORE SETMESSAGE Opening temp file failed!");

				curl_easy_cleanup(curl);

				m_ShellBase.sendWarnMessage(std::string("ieOSG4Web_Ctrl::doDownloadCore -> Error Opening Out File."));

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

				m_ShellBase.sendWarnMessage(std::string("ieOSG4Web_Ctrl::doDownloadCore -> Error Opening Header File."));

				fclose(outfile);
				m_ShellBase.removingFile(tempdl);

				PR_Unlock(s_DlCoreThreadLocking);
				return;
			}
		
			curl_easy_setopt(curl, CURLOPT_VERBOSE, 1);
			//curl_easy_setopt(curl, CURLOPT_HEADER, 1);
			curl_easy_setopt(curl, CURLOPT_URL, fdlcoreaddress.c_str());
	
			//Proxy Settings
			if(m_ProxyEnabled)
			{
				proxy = (LPCTSTR) m_ProxyName;

				if( !m_ProxyPort.IsEmpty() )
				{
					proxy += ":";
					proxy += (LPCTSTR) m_ProxyPort;
				}

				curl_easy_setopt(curl, CURLOPT_PROXY, proxy.c_str()); 
			}

			//TODO: capire come fare per gestire user e password del proxy

			curl_easy_setopt(curl, CURLOPT_WRITEDATA, outfile);
			curl_easy_setopt(curl, CURLOPT_WRITEHEADER, headerfile);
			curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, &ieOSG4Web_Ctrl::writeDLStream);
			curl_easy_setopt(curl, CURLOPT_READFUNCTION, &ieOSG4Web_Ctrl::readDLStream);
			curl_easy_setopt(curl, CURLOPT_NOPROGRESS, FALSE);
			curl_easy_setopt(curl, CURLOPT_PROGRESSFUNCTION, &ieOSG4Web_Ctrl::callProgressDLStatus);
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

				m_ShellBase.removingFile(tempdl);

				m_ShellBase.sendWarnMessage(std::string("ieOSG4Web_Ctrl::doDownloadCore -> Downloading Failed."));

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
						this->loadingCoreCommand("LOADCORE STATUSBAR_VISIBILITY FALSE");

						this->loadingCoreCommand("LOADCORE SETMESSAGE_COLOR LC_OSG_RED");
						this->loadingCoreCommand("LOADCORE SETMESSAGE Server response: " + headline);

						curl_easy_cleanup(curl);

						m_ShellBase.sendWarnMessage(std::string("ieOSG4Web_Ctrl::doDownloadCore -> Server Error: " + headline));

						checkheaderfile.close();
						m_ShellBase.removingFile(tempdl);

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
				m_ShellBase.removingFile(tempdl);

				m_ShellBase.sendWarnMessage(std::string("ieOSG4Web_Ctrl::doDownloadCore -> Error Opening Header File. Checking server response failed"));

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

			m_ShellBase.sendWarnMessage(std::string("ieOSG4Web_Ctrl::doDownloadCore -> Curl initialization Failed."));
			PR_Unlock(s_DlCoreThreadLocking);

			return;
		}
	}
	else
		m_ShellBase.sendNotifyMessage(std::string("ieOSG4Web_Ctrl::doDownloadCore -> Temporary package is present, Skipping Downloading."));

	this->loadingCoreCommand("LOADCORE SETMESSAGE Checking Validity...");
	m_ShellBase.sendNotifyMessage(std::string("ieOSG4Web_Ctrl::doDownloadCore -> Check Package Validity."));

	if(!m_ShellBase.checkFileValidity(tempdl, (LPCTSTR) m_ADVCoreSHA1Hash))
	{
		this->loadingCoreCommand("LOADCORE STATUSBAR_VISIBILITY FALSE");

		this->loadingCoreCommand("LOADCORE SETMESSAGE_COLOR LC_OSG_RED");
		this->loadingCoreCommand("LOADCORE SETMESSAGE Validity Control Failed!");

		m_ShellBase.sendWarnMessage(std::string("ieOSG4Web_Ctrl::doDownloadCore -> Validity Control Failed!"));

		//Cleaning Archive List
		m_ShellBase.freeCompressedCore();
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

	m_ShellBase.sendNotifyMessage(std::string("ieOSG4Web_Ctrl::doDownloadCore -> Unpacking Core."));
	
	int coreunpack = 0; //Condizione di loading

	//Opening Archive List
	int filenumber = m_ShellBase.openCompressedCore(tempdl);

	if(filenumber < 0)
	{
		//TODO: messaggi di Errore
		this->loadingCoreCommand("LOADCORE STATUSBAR_VISIBILITY FALSE");

		this->loadingCoreCommand("LOADCORE SETMESSAGE_COLOR LC_OSG_RED");
		this->loadingCoreCommand("LOADCORE SETMESSAGE Unpacking Failed! Error opening archive");

		m_ShellBase.sendWarnMessage(std::string("ieOSG4Web_Ctrl::doDownloadCore -> Unpacking Failed! Error opening archive"));

		//Cleaning Archive List
		m_ShellBase.freeCompressedCore();
		PR_Unlock(s_DlCoreThreadLocking);

		return;
	}

	if(filenumber == 0)
	{
		//TODO: messaggi di Errore
		this->loadingCoreCommand("LOADCORE STATUSBAR_VISIBILITY FALSE");

		this->loadingCoreCommand("LOADCORE SETMESSAGE_COLOR LC_OSG_RED");
		this->loadingCoreCommand("LOADCORE SETMESSAGE Unpacking Failed! Empty archive");

		m_ShellBase.sendWarnMessage(std::string("ieOSG4Web_Ctrl::doDownloadCore -> Unpacking Failed! Empty archive"));

		//Cleaning Archive List
		m_ShellBase.freeCompressedCore();
		PR_Unlock(s_DlCoreThreadLocking);

		return;
	}
	
	unsigned int cicleno = 0;
	while(coreunpack == 0)
	{
		coreunpack = m_ShellBase.unpackCompressedCoreFile();

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

		m_ShellBase.sendWarnMessage(std::string("ieOSG4Web_Ctrl::doDownloadCore -> Unpacking Failed! Error during decompression"));

		//Cleaning Archive List
		m_ShellBase.freeCompressedCore();
		PR_Unlock(s_DlCoreThreadLocking);

		return;
	}
	
	//Cleaning Archive List
	m_ShellBase.freeCompressedCore();

	//Loading Session
	this->loadingCoreCommand("LOADCORE STATUSBAR_VISIBILITY FALSE");
	this->loadingCoreCommand("LOADCORE SETMESSAGE "); //Spengo i messaggi

	m_ShellBase.sendNotifyMessage(std::string("ieOSG4Web_Ctrl::doDownloadCore -> Loading Unpacked Core"));

	if(!m_ShellBase.startLoadingAdvancedCore()) //Start Advanced Core Loading
	{
		m_ShellBase.sendWarnMessage(std::string("ieOSG4Web_Ctrl::doDownloadCore -> Loading advanced core failed! Starting Loading Core"));

		//Reload LoadCore
		if(!m_ShellBase.startLoadingBaseCore())
		{
			s_PluginMessageError = m_ShellBase.getErrorString();
			m_ShellBase.sendWarnMessage(std::string("ieOSG4Web_Ctrl::doDownloadCore -> Starting Loading Core failed... BYE BYE..."));
			return;
		}

		//Setting Error Message
		this->loadingCoreCommand("LOADCORE SETMESSAGE_COLOR LC_OSG_RED");
		this->loadingCoreCommand("LOADCORE SETMESSAGE Loading Advanced Core Failed!");
	}
	else
		m_ShellBase.sendNotifyMessage(std::string("ieOSG4Web_Ctrl::doDownloadCore -> Advanced Core is up and running"));

	m_Loading = false;
	
	PR_Unlock(s_DlCoreThreadLocking);
	return;
}

int ieOSG4Web_Ctrl::callProgressDLStatus(void* classptr, double downtot, double downnow, double ultotal, double ulnow)
{
	ieOSG4Web_Ctrl *instance = (ieOSG4Web_Ctrl *) classptr;

	if(classptr)
		return instance->doProgressDLStatus(downtot, downnow, ultotal, ulnow);
	else
	{
		instance->sendWarnMessage(std::string("ieOSG4Web_Ctrl::callProgressDLStatus -> Instance seems not present."));
		return -1;
	}
}

int ieOSG4Web_Ctrl::doProgressDLStatus(double downtot, double downnow, double ultotal, double ulnow)
{
	if(this->getDlCoreThreadShutDown())
	{
		m_ShellBase.sendNotifyMessage(std::string("ieOSG4Web_Ctrl::doProgressDLStatus -> Forcing Downloading Thread Exit."));
		return 1; //Condizione di uscita per il thread di Download
	}

	std::string statmsg("LOADCORE STATUSBAR_VALUE ");

	std::ostringstream convstream; //creates an ostringstream object
	convstream << (downnow * 300.0 / downtot) << std::flush; //Uso la dimensione 3x
	statmsg += convstream.str(); 

	this->loadingCoreCommand(statmsg);
		
	return 0;
}

size_t ieOSG4Web_Ctrl::writeDLStream(void *ptr, size_t size, size_t nmemb, FILE *stream)
{
	return fwrite(ptr, size, nmemb, stream);
}

size_t ieOSG4Web_Ctrl::readDLStream(void *ptr, size_t size, size_t nmemb, FILE *stream)
{
	return fread(ptr, size, nmemb, stream);
}

bool ieOSG4Web_Ctrl::releaseDownloadCore()
{
	if (this->getDlCoreThread())
	{
		m_DlCoreShutdownThread = true;

		m_ShellBase.sendNotifyMessage(std::string("ieOSG4Web_Ctrl::releaseDownloadCore -> Join Downloading Thread."));

		PR_JoinThread(m_DlCoreThread);

		PR_Sleep(EXITDLCORE_DELAY);

		m_ShellBase.sendNotifyMessage(std::string("ieOSG4Web_Ctrl::releaseDownloadCore -> Delete Downloading Thread."));

		m_DlCoreShutdownThread = false;
		m_DlCoreThread = NULL;
	}
	
	return true;
}


///////////////////////////////////////////////////////////////////////////////////////
//
//		Funzioni Draw Finestra
//
//////////////////////////////////////////////////////////////////////////////////////

void ieOSG4Web_Ctrl::OnPaint()
{
	CPaintDC dc(this);
	
	if(!this->checkRunning())
	{
		RECT rcBounds;
		GetClientRect(&rcBounds);
		dc.Rectangle(&rcBounds); 

		dc.TextOut(2,2, (std::string("OSG4Web Instance Error: ") + s_PluginMessageError).c_str());
	}
}

void ieOSG4Web_Ctrl::OnTimer(UINT nIDEvent)
{
	if(this->checkRunning())
		this->doRenderFrame();

	COleControl::OnTimer(nIDEvent);
}