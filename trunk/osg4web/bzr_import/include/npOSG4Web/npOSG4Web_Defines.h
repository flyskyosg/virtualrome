#ifndef __NPOSG4WEB_DEFINES__
#define __NPOSG4WEB_DEFINES__

//Plugin Defines
#define MIME_TYPES_HANDLED  "application/osg4web-plugin" 
#define PLUGIN_NAME         "OSG4Web: OpenSceneGraph Plug-in 4 Firefox"
#define MIME_TYPES_DESCRIPTION  MIME_TYPES_HANDLED":scr:"PLUGIN_NAME
#define PLUGIN_DESCRIPTION  PLUGIN_NAME " (OpenSceneGraph Plug-in 4 Firefox)"


//OSG4Web name
#define		OSG4WEB_SHELL_NAME		"npOSG4Web"

#if defined (WIN32) //Windows DEBUG

#if defined (_DEBUG)
#define		NPOSG4WEB_DIRECTORY		NS_OS_CURRENT_PROCESS_DIR //FIXME: ripristinare
#else
#define		NPOSG4WEB_DIRECTORY		"ProfD"
#endif

#define		APPEND_NPOSG4WEB_DIRECTORY	"\\plugins"
#define		APPEND_NPOSG4WEB_EXTINSTALLDIR	"\\plugins"
#define		APPEND_TEMP_DIRECTORY	"\\osg4web"
#define		COREINSTALL_DIRECTORY	NS_WIN_APPDATA_DIR //Controllare
#define		APPEND_CORES_DIRECTORY	"\\osg4web"

#else //Linux

#define		NPOSG4WEB_DIRECTORY		NS_OS_HOME_DIR //FIXME: controllare
#define		APPEND_NPOSG4WEB_DIRECTORY	"/.mozilla/plugins"
#define		APPEND_TEMP_DIRECTORY	"/osg4web"
#define		COREINSTALL_DIRECTORY	NS_OS_HOME_DIR
#define		APPEND_CORES_DIRECTORY	"/.osg4web"

#endif 


//Define di Delay dei Threads
#define		RENDER_DELAY				10
#define		EXIT_DELAY					1000

#define		DLCORE_DELAY				10
#define		EXITDLCORE_DELAY			1000

//Gestisce il Rendering tramite Timer o Threads
//#define		RENDER_WITH_TIMER_EVENT	1

#endif //__NPOSG4WEB_DEFINES__

