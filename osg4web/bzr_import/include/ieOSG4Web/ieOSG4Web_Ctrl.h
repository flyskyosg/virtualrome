#ifndef __IEOSG4WEB_CTRL__
#define __IEOSG4WEB_CTRL__ 1

#include <CommonShell/ShellBase.h>

class ieOSG4Web_Ctrl : public COleControl
{
	DECLARE_DYNCREATE(ieOSG4Web_Ctrl)

public:
	ieOSG4Web_Ctrl();
	virtual void DoPropExchange(CPropExchange* pPX);
	virtual void OnResetState();

protected:
	~ieOSG4Web_Ctrl();
	DECLARE_OLECREATE_EX(ieOSG4Web_Ctrl)		// Class factory and guid
	DECLARE_OLETYPELIB(ieOSG4Web_Ctrl)			// GetTypeInfo
	DECLARE_PROPPAGEIDS(ieOSG4Web_Ctrl)			// Property page IDs
	DECLARE_OLECTLTYPE(ieOSG4Web_Ctrl)			// Type name and misc status
	DECLARE_MESSAGE_MAP()						// message map
	DECLARE_DISPATCH_MAP()						// dispatch map
	DECLARE_EVENT_MAP()							// event map

// Dispatch and event IDs
public:


	#define DISPID_ADVCore						100
	#define DISPID_ADVCoreStartOptions			101
	#define DISPID_ADVCoreInitOptions			102
	#define DISPID_ADVCoreSHA1					103
	#define DISPID_ADVCoreDep					104
	#define DISPID_ADVCoreDepSHA1				105
	#define DISPID_LoadCoreOptions				106 
	#define DISPID_EnableDiskLogs				107
	#define DISPID_ExecCoreCommand				108
	#define DISPID_ExecShellCommand				109

	enum DispIDEnum
	{
		dispidADVCore						= DISPID_ADVCore,
		dispidADVCoreStartOptions			= DISPID_ADVCoreStartOptions,
		dispidADVCoreInitOptions			= DISPID_ADVCoreInitOptions,
		dispidADVCoreSHA1					= DISPID_ADVCoreSHA1,
		dispidADVCoreDep					= DISPID_ADVCoreDep,
		dispidADVCoreDepSHA1				= DISPID_ADVCoreDepSHA1,
		dispidLoadCoreOptions				= DISPID_LoadCoreOptions,
		dispidEnableDiskLogs				= DISPID_EnableDiskLogs,
		dispidExecCoreCommand				= DISPID_ExecCoreCommand,  
		dispidExecShellCommand				= DISPID_ExecShellCommand,
	};


#define EVENTID_GetEvent					500

	enum EventIDEnum
	{
		eventidGetEvent						= EVENTID_GetEvent		
	};

protected:
	//Sending method types
	enum EventMethod
	{
		SEND_ACTION_BY_CATCH = 0,
		SEND_ACTION_BY_FIRE
	};

	//Getting initialization Parameters
	CString				GetInstallDir();
	CString				GetInstallCoreDir();
	CString				GetTempDir();
	void				GetProxyConfiguration();

	//JS and Settings Parameters
	CString				m_ADVCore;
	CString				m_ADVStartOptions;
	CString				m_ADVInitOptions;
	CString				m_ADVCoreSHA1Hash;
	CString				m_ADVCoreDep;
	CString				m_ADVCoreDepSHA1Hash;
	CString				m_LoadCBOptions;
	CString				m_EnableDiskLogs;

	//Proxy Parameters
	bool				m_ProxyEnabled;
	CString				m_ProxyName;
	CString				m_ProxyPort;

public:
	afx_msg int			OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void		OnClose(DWORD dwSaveOptions);
	afx_msg void		OnPaint();

protected:
	//Exsposed to JS
	BSTR 				ExecCoreCommand(LPCTSTR);
	BSTR 				ExecShellCommand(LPCTSTR);

    void 				GetEvent(LPCTSTR);

public:
	//Checks OSG4Web Initialization
	bool				isInitialized() { return m_Initialized; };
	//Checks Core Running
	bool				checkRunning();

	//Initialize the Loading Core
	bool				initLoadCore();

	//Switcha il metodo di invio messaggi a JavaScript
	bool				SwitchSendMethod(std::string);

	//reset Window Handler
	bool				resetWindowHandler();

	//Invia Messaggio di Warn
	void				sendWarnMessage(std::string message);	

public:	
	//Static Transport Functions
	static bool			TransportEvent(void*, std::string);
	
	static bool			callResetWindowHandler(void*);
		
private:
	//Invia Evento tramite metodo Fire
	bool				SendActiveXEvent(std::string);
	//Invia Evento tramite metodo Getback
	bool				FirePluginEvent(std::string);
	bool				SendGetBackEventToJS(CString& jsFunctionName, CStringArray& paramArray);

	void				requestExplicitRendering();
	bool				showFatalCoreErrors();

	//Formattazione messaggi di errore
	CString				GetSystemErrorMessage(DWORD dwError);

private:
	//Decide il metodo di invio dell'evento al browser Fire vs GetUrl
	EventMethod			m_FireMethod;

	//Shell Comune
	ShellBase			m_ShellBase;

	//controllo di inizializzazione
	bool				m_Initialized;
	bool				m_InitOptionsSet;
	bool				m_Loading;
};

#endif //__IEOSG4WEB_CTRL__