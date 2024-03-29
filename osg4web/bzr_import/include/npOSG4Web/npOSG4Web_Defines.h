#ifndef __NPOSG4WEB_DEFINES__
#define __NPOSG4WEB_DEFINES__

//Plugin Defines
#define		MIME_TYPES_HANDLED						"application/osg4web-plugin" 
#define		PLUGIN_NAME								"OSG4Web: OpenSceneGraph Plug-in 4 Firefox"
#define		MIME_TYPES_DESCRIPTION					MIME_TYPES_HANDLED":scr:"PLUGIN_NAME
#define		PLUGIN_DESCRIPTION  PLUGIN_NAME			" (OpenSceneGraph Plug-in 4 Firefox)"

//OSG4Web name
#define		OSG4WEB_SHELL_NAME						"npOSG4Web"



#if defined (WIN32) //Windows DEBUG

//#define	_WIN32_WINNT							0x0400

#define		NPOSG4WEB_DIRECTORY						NS_OS_CURRENT_PROCESS_DIR //FIXME: ripristinare
#define		NPOSG4WEB_DIRECTORY_LOCAL				"ProfD"

#define		APPEND_NPOSG4WEB_DIRECTORY				"\\plugins"
#define		APPEND_NPOSG4WEB_EXTINSTALLDIR			"\\plugins"
#define		APPEND_TEMP_DIRECTORY					"\\osg4web"
#define		COREINSTALL_DIRECTORY					NS_WIN_APPDATA_DIR //Controllare
#define		APPEND_CORES_DIRECTORY					"\\osg4web"

#else //Linux

#define		NPOSG4WEB_DIRECTORY						NS_OS_HOME_DIR //FIXME: controllare
#define		APPEND_NPOSG4WEB_DIRECTORY				"/.mozilla/plugins"
#define		APPEND_TEMP_DIRECTORY					"/osg4web"
#define		COREINSTALL_DIRECTORY					NS_OS_HOME_DIR
#define		APPEND_CORES_DIRECTORY					"/.osg4web"

#endif 



//Object Options Defines
#define		OBJECT_OPTION_ADVCORE					"ADVCore"
#define		OBJECT_OPTION_ADVCSTARTOPT				"ADVCoreStartOptions"
#define		OBJECT_OPTION_ADVCINITOPT				"ADVCoreInitOptions"
#define		OBJECT_OPTION_ADVCORESHA1				"ADVCoreSHA1"
#define		OBJECT_OPTION_ADVCOREDEP				"ADVCoreDep"
#define		OBJECT_OPTION_ADVCOREDEPSHA1			"ADVCoreDepSHA1"
#define		OBJECT_OPTION_LOADEROPT					"LoaderOptions"
#define		OBJECT_OPTION_ENABLELOGS				"EnableDiskLogs"



#endif //__NPOSG4WEB_DEFINES__

