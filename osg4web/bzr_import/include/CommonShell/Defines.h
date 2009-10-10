#ifndef __OSG4WEB_SHELLBASE_DEFINES__
#define __OSG4WEB_SHELLBASE_DEFINES__ 1

//ShellBase
//Default Values
#define		SHELL_VERSION_STRING		"OSG4Web - Shell Version: 1.0.0.0"
#define		LOAD_CORE_DEFAULT_NAME		"LoadCore"
#define		ADVANCED_CORE_DEFAULT_NAME	"dummy_core_run"
#define		CORE_DEPENDANCIES_DIR		"coredeps"

//Init Options Defines
#define		INIT_OPTION_PROXYHNAME		"ProxyHName"
#define		INIT_OPTION_PROXYHPORT		"ProxyHPort"
#define		INIT_OPTION_INSTALLDIR		"InstallDir"
#define		INIT_OPTION_LOCALINSTDIR	"LocalInstDir"
#define		INIT_OPTION_TEMPDIR			"TempDir"
#define		INIT_OPTION_COREINSTDIR		"CoreInstallDir"

//Cores General Options
#define		LOAD_CORE_CORENAME			"LoadCoreName"
#define		LOAD_CORE_COREDIR			"LoadCoreDir"
#define		ADV_CORE_CORENAME			"AdvCoreName"
#define		ADV_CORE_MAINCOREDIR		"AdvCoreMainDir"
#define		ADV_CORE_DEPCOREDIR			"AdvCoreDepDir"

//Object Options Defines
#define		OBJECT_OPTION_ADVCORE			"ADVCore"
#define		OBJECT_OPTION_ADVCSTARTOPT		"ADVCoreStartOptions"
#define		OBJECT_OPTION_ADVCINITOPT		"ADVCoreInitOptions"
#define		OBJECT_OPTION_ADVCORESHA1		"ADVCoreSHA1"
#define		OBJECT_OPTION_ADVCOREDEP		"ADVCoreDep"
#define		OBJECT_OPTION_ADVCOREDEPSHA1	"ADVCoreDepSHA1"
#define		OBJECT_OPTION_LOADCOREOPT		"LoadCoreOptions"
#define		OBJECT_OPTION_ENABLELOGS		"EnableDiskLogs"


//Loading Core Options
#if defined( _DEBUG )
#define		DEBUGAPPEND						"d"   
#else
#define		DEBUGAPPEND						""
#endif

//Loading Core Messages Delay
#define LOADING_CORE_MESSAGES_MIN			500
#define LOADING_CORE_MESSAGES_MID			2000
#define LOADING_CORE_MESSAGES_MAX			10000

//Rendering Sleep Time
#define		RENDER_DELAY					10

//ShellThread Defines
#define		JOIN_THREAD_DELAY				1000

#endif //__OSG4WEB_SHELLBASE_DEFINES__