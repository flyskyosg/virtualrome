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




/*****************************************************************************
 *
 *		Preparazione ActiveX
 *
 *****************************************************************************/

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



/*************************************************************************
 *
 *		Classe ieOSG4Web_Ctrl: Inizializzazione
 *
 *************************************************************************/

 ieOSG4Web_Ctrl::ieOSG4Web_Ctrl() :
	m_ADVCore( "" ),
	m_ADVStartOptions( "" ),
	m_ADVInitOptions( "" ),
	m_ADVCoreSHA1Hash( "" ),
	m_ADVCoreDep( "" ),
	m_ADVCoreDepSHA1Hash( "" ),
	m_LoadCBOptions( "" ),
	m_EnableDiskLogs( "" ),
	m_FireMethod( SEND_ACTION_BY_CATCH ),
	m_ProxyEnabled( false ),
	m_ProxyName( "" ),
	m_ProxyPort( "8080" ),
	m_Initialized(false),
	m_InitOptionsSet(false),
	m_Loading(false)
{
	InitializeIIDs(&IID_DieOSG4Web, &IID_DieOSG4WebEvents);

	m_ShellBase.initializeLog("ieOSG4Web");

	m_ShellBase.sendNotifyMessage("ieOSG4Web_Ctrl::ieOSG4Web_Ctrl -> Default Constructor");
	m_ShellBase.sendNotifyMessage("ieOSG4Web_Ctrl::ieOSG4Web_Ctrl -> Discovering Internet Explorer settings");

	std::string reftempstr;
	reftempstr = (LPCTSTR) GetInstallDir();
	if(reftempstr.empty())
	{
		m_ShellBase.setErrorCode( 41 );
		m_ShellBase.sendWarnMessage("ieOSG4Web_Ctrl::ieOSG4Web_Ctrl -> " + m_ShellBase.getErrorString());
		return;
	}

	//reftempstr.append(APPEND_IEOSG4WEB_DIRECTORY); //FIXME: controllare
	m_ShellBase.setInitOption(INIT_OPTION_INSTALLDIR, reftempstr);
	reftempstr.clear();
	

	reftempstr = (LPCTSTR) GetTempDir();
	if(reftempstr.empty())
	{
		m_ShellBase.setErrorCode( 43 );
		m_ShellBase.sendWarnMessage("ieOSG4Web_Ctrl::ieOSG4Web_Ctrl -> " + m_ShellBase.getErrorString());
		return;
	}

	reftempstr.append(APPEND_TEMP_DIRECTORY);
	m_ShellBase.setInitOption( INIT_OPTION_TEMPDIR, reftempstr);
	reftempstr.clear();
	
#if defined(IEOSG4WEB_COREDIR_PLUGIN)
	reftempstr = (LPCTSTR) GetInstallDir();
#else
	reftempstr = (LPCTSTR) GetInstallCoreDir();
#endif

	if(reftempstr.empty())
	{
		m_ShellBase.setErrorCode( 42 );
		m_ShellBase.sendWarnMessage("ieOSG4Web_Ctrl::ieOSG4Web_Ctrl -> " + m_ShellBase.getErrorString());
		return;
	}

	reftempstr.append(APPEND_CORES_DIRECTORY);
	m_ShellBase.setInitOption( INIT_OPTION_LOCALINSTDIR, reftempstr ); //FIXME: per adesso metto lo stesso... settare in core che è ie quindi non c'è
	m_ShellBase.setInitOption( INIT_OPTION_COREINSTDIR, reftempstr );
	reftempstr.clear();

	this->GetProxyConfiguration();
	if( m_ProxyEnabled && m_ProxyName.GetLength()!= 0 )
	{
		m_ShellBase.setInitOption(INIT_OPTION_PROXYHNAME, (LPCTSTR) m_ProxyName);
		m_ShellBase.setInitOption(INIT_OPTION_PROXYHPORT, (LPCTSTR) m_ProxyPort);
	}

	//Check dei parametri iniziali
	m_ShellBase.sendNotifyMessage("ieOSG4Web_Ctrl::ieOSG4Web_Ctrl -> configuring ShellBase with firefox settings");
	m_InitOptionsSet = m_ShellBase.configuringInitialOptions();


	//TODO: manca LOCAL INSTALL DIR che non c'è
}  

ieOSG4Web_Ctrl::~ieOSG4Web_Ctrl()
{
	m_ShellBase.sendNotifyMessage("ieOSG4Web_Ctrl::~ieOSG4Web_Ctrl -> default destructor");
}

void ieOSG4Web_Ctrl::DoPropExchange(CPropExchange* pPX) //TODO: finire le PX_String
{
	ExchangeVersion(pPX, MAKELONG(_wVerMinor, _wVerMajor));
	COleControl::DoPropExchange(pPX);

	//Prende i parametri dai PARAM di OBJECT
	PX_String( pPX, OBJECT_OPTION_ADVCORE, m_ADVCore, "" );
	PX_String( pPX, OBJECT_OPTION_ADVCSTARTOPT, m_ADVStartOptions, "" );
	PX_String( pPX, OBJECT_OPTION_ADVCINITOPT, m_ADVInitOptions, "" );
	PX_String( pPX, OBJECT_OPTION_ADVCORESHA1, m_ADVCoreSHA1Hash, "" );
	PX_String( pPX, OBJECT_OPTION_ADVCOREDEP, m_ADVCoreDep, "" );
	PX_String( pPX, OBJECT_OPTION_ADVCOREDEPSHA1, m_ADVCoreDepSHA1Hash, "" );
	PX_String( pPX, OBJECT_OPTION_LOADEROPT, m_LoadCBOptions, "" );
	PX_String( pPX, OBJECT_OPTION_ENABLELOGS, m_EnableDiskLogs, "" );
}

void ieOSG4Web_Ctrl::OnResetState() 
{
	COleControl::OnResetState();
	COleControl::ResetStockProps();

	m_ADVCore.Empty();
	m_ADVStartOptions.Empty();
	m_ADVInitOptions.Empty();
	m_ADVCoreSHA1Hash.Empty();
	m_ADVCoreDep.Empty();
	m_ADVCoreDepSHA1Hash.Empty();
	m_LoadCBOptions.Empty();
	m_EnableDiskLogs.Empty();
}

void ieOSG4Web_Ctrl::OnClose(DWORD dwSaveOptions)
{
	m_ShellBase.sendNotifyMessage(std::string("ieOSG4Web_Ctrl::OnClose -> Shutting down plugin instance"));

	m_Loading = true;
	
	if(!m_ShellBase.closeAllLibraries())
		m_ShellBase.sendWarnMessage(std::string("ieOSG4Web_Ctrl::OnClose -> Error closing all libraries."));
	
	m_ShellBase.sendNotifyMessage(std::string("ieOSG4Web_Ctrl::OnClose -> Shutting down finished"));

	m_Initialized = false;

	COleControl::OnClose(dwSaveOptions);
}

int ieOSG4Web_Ctrl::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	m_ShellBase.sendNotifyMessage("ieOSG4Web_Ctrl::OnCreate -> Initializing Window.");

	m_Initialized = false;

	if (COleControl::OnCreate(lpCreateStruct) == -1)
	{
		m_ShellBase.sendWarnMessage("ieOSG4Web_Ctrl::OnCreate -> initializing derived instance failed!");
		return -1;
	}

	if (!m_InitOptionsSet)
	{
		m_ShellBase.sendWarnMessage("nsPluginInstance::init -> Error retrieving inititialization options.");
		return -1;
	}
	
	//Linking Plugin to ShellBase
	m_ShellBase.sendNotifyMessage("ieOSG4Web_Ctrl::OnCreate -> Linking Plugin to ShellBase.");

	m_ShellBase.setWindowHandler( GetSafeHwnd() );
	m_ShellBase.setInstanceHandler( this );
	m_ShellBase.setResetWindowHandler( &ieOSG4Web_Ctrl::callResetWindowHandler );
	m_ShellBase.setTransportEventHandler( &ieOSG4Web_Ctrl::TransportEvent );

	m_Initialized = true;

	m_ShellBase.sendNotifyMessage("ieOSG4Web_Ctrl::OnCreate -> setting Object Options.");

	m_ShellBase.setObjectOption(OBJECT_OPTION_ADVCORE, (LPCSTR) m_ADVCore);
	m_ShellBase.setObjectOption(OBJECT_OPTION_ADVCSTARTOPT, (LPCSTR) m_ADVStartOptions);
	m_ShellBase.setObjectOption(OBJECT_OPTION_ADVCINITOPT, (LPCSTR) m_ADVInitOptions);
	m_ShellBase.setObjectOption(OBJECT_OPTION_ADVCORESHA1, (LPCSTR) m_ADVCoreSHA1Hash);
	m_ShellBase.setObjectOption(OBJECT_OPTION_ADVCOREDEP, (LPCSTR) m_ADVCoreDep);
	m_ShellBase.setObjectOption(OBJECT_OPTION_ADVCOREDEPSHA1, (LPCSTR) m_ADVCoreDepSHA1Hash);
	m_ShellBase.setObjectOption(OBJECT_OPTION_LOADEROPT, (LPCSTR) m_LoadCBOptions);
	m_ShellBase.setObjectOption(OBJECT_OPTION_ENABLELOGS, (LPCSTR) m_EnableDiskLogs);


	m_ShellBase.sendNotifyMessage("ieOSG4Web_Ctrl::OnCreate -> starting the Loading Core.");
	if(!m_ShellBase.startLoadingBaseCore())
		return 0;

	m_Loading = false;

	if(!m_ShellBase.initializeAdvancedCore())
	{
		//Reload Loader Core
		if(!m_ShellBase.startLoadingBaseCore())
		{
			m_ShellBase.sendWarnMessage(std::string("ieOSG4Web_Ctrl::OnCreate -> ") + m_ShellBase.getErrorString());
			return -1;
		}

		m_ShellBase.sendWarnMessage(std::string("ieOSG4Web_Ctrl::OnCreate -> Initializing Advanced Core Failed!"));
	}

	return 0;
}


/**************************************************************************
 *
 *		Funzioni di invio Eventi a JavaScript
 *
 **************************************************************************/

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


/***********************************************************************
 *
 *		Funzioni esposte a JavaScript
 *
 ***********************************************************************/

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

	retstr.Append( ( m_ShellBase.execCoreCommand(std::string((LPCTSTR) line)) ).c_str() );

	BSTR str = retstr.AllocSysString();
	return str;
}


/************************************************************************
 *
 *		Funzioni gestione degli Errori
 *
 ************************************************************************/

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



/*************************************************************************************
 *
 *		Funzioni di gestione finestra
 *
 *************************************************************************************/

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


/************************************************************************
 *
 *		Rendering auxiliar functions
 *
 ************************************************************************/

bool ieOSG4Web_Ctrl::checkRunning()
{
	if(!m_ShellBase.isRunning())
		return false;
	else
		return true;
}

void ieOSG4Web_Ctrl::requestExplicitRendering()
{
	m_ShellBase.requestExplicitRendering();
}

bool ieOSG4Web_Ctrl::showFatalCoreErrors()
{
	return ( m_ShellBase.isThereErrors() && !checkRunning());
}


/*************************************************************************************
 *
 *		Funzioni Draw Finestra
 *
 *************************************************************************************/

void ieOSG4Web_Ctrl::OnPaint()
{
	CPaintDC dc(this);
	
	if( this->showFatalCoreErrors() )
	{
		RECT rcBounds;
		GetClientRect(&rcBounds);
		dc.Rectangle(&rcBounds); 

		dc.TextOut(2,2, (std::string("OSG4Web Instance Error: ") + m_ShellBase.getErrorString()).c_str());
	}
}
