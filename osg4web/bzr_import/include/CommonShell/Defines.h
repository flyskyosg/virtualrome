#ifndef __OSG4WEB_SHELLBASE_DEFINES__
#define __OSG4WEB_SHELLBASE_DEFINES__ 1

#define		SHELL_VERSION_STRING		"OSG4Web - Shell Version: 1.0.0.0"

#define		OSG4WEB_LOADCORE_NAME		"LoadCore"

#if defined( _DEBUG )
#define		DEBUGAPPEND					"d"   
#else
#define		DEBUGAPPEND					""
#endif



//Init Options Defines
#define		INIT_OPTION_PROXYHNAME		"ProxyHName"
#define		INIT_OPTION_PROXYHPORT		"ProxyHPort"
#define		INIT_OPTION_INSTALLDIR		"InstallDir"
#define		INIT_OPTION_LOCALINSTDIR	"LocalInstDir"
#define		INIT_OPTION_TEMPDIR			"TempDir"
#define		INIT_OPTION_COREINSTDIR		"CoreInstallDir"


//Object Options Defines
#define		OBJECT_OPTION_ADVCORE			"ADVCore"
#define		OBJECT_OPTION_ADVCSTARTOPT		"ADVCoreStartOptions"
#define		OBJECT_OPTION_ADVCINITOPT		"ADVCoreInitOptions"
#define		OBJECT_OPTION_ADVCORESHA1		"ADVCoreSHA1"
#define		OBJECT_OPTION_ADVCOREDEP		"ADVCoreDep"
#define		OBJECT_OPTION_ADVCOREDEPSHA1	"ADVCoreDepSHA1"
#define		OBJECT_OPTION_LOADCOREOPT		"LoadCoreOptions"
#define		OBJECT_OPTION_ENABLELOGS		"EnableDiskLogs"

#endif //__OSG4WEB_SHELLBASE_DEFINES__