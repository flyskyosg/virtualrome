

#ifndef __NPOSG4WEB_PLUGIN_H__
#define __NPOSG4WEB_PLUGIN_H__ 1

#include "nspr.h"
#include "prlock.h"
#include "prthread.h"
#include <npOSG4Web/npOSG4Web_pluginbase.h>
#include <npOSG4Web/npOSG4Web_nsScriptablePeer.h>
#include <npOSG4Web/npOSG4Web_Defines.h>
#include <CommonShell/ShellBase.h>

#include <iostream>

#if defined(XP_UNIX)
	#include <X11/Xlib.h>
	#include <X11/Intrinsic.h>
	#include <X11/cursorfont.h>
#endif

/*		Classe Plug-In Instance
 *
 *		Questa classe si occupa di creare un istanza del plug-In all'interno di una pagina WEB. 
 *		Esiste una istanza per ogni tab e pagina di Firefox.
 *		Esite uno Scriptable Peer associato alla istanza che lo interfaccia come oggetto JavaScript.+
 *		La Plug-In instance gestisce il ciclo di rendering del core e l'interazione con i modelli nello scenegraph.
 */
class nsPluginInstance : public nsPluginInstanceBase
{
public:
	//costruttore dell'istanza Plug-In
	nsPluginInstance(NPP aInstance);
	//distruttore dell'istanza Plug-In
	virtual ~nsPluginInstance();

	//permette il passaggio dall'involucro all'istanza dei parametri di inizializzazione
	NPError SetInitialData(PRUint16 argc, char* argn[], char* argv[]);
	//inizializza l'istanza del Plug-In
	NPBool init(NPWindow* aWindow);
	//inizializza l'istanza del LoadingCore
	NPBool initLoadCore();
	//chiude l'istanza del Plug-In
	void shut();
	//check se l'istanza è inizializzata
	NPBool isInitialized();

	//uint16 HandleEvent(void* aEvent);

	//Get Main Rendering Thread
	const PRThread* getThread() { return mThread; };

	//Get Downloading Core Thread
	const PRThread* getDlCoreThread() { return mDlCoreThread; };

	// we need to provide implementation of this method as it will be
	// used by Mozilla to retrive the scriptable peer
	NPError	GetValue(NPPVariable variable, void *value);

	//restituisce lo ScriptablePeer
	nsScriptablePeer* getScriptablePeer();

	//invia evento a javascript tramite metodo NPN_GetUrl
	NPError SendGetEvent(std::string evnt);
	
	//Trasporta dal core al plugin gli eventi di js
	static bool TransportEvent(void*, std::string);

	//reset Window Handler
	static bool callResetWindowHandler(void*);
	bool resetWindowHandler();

	//Funzioni di Render
	static void callRender(void*);

	void doRender();
	void doRenderFrame();
	//Controlla se il core è in stato di rendering
	bool checkRunning();
	//Controlla che la shell non sia in stato di errore
	bool checkErrorPresence();

	static bool callPrepareRendering(void*);
	bool prepareRendering();
	
	static bool callCloseRendering(void*);
	bool closeRendering();

	//Download Core
	static bool callRequestCoreDownloading(void*);
	bool requestCoreDownloading();

	static void callDownloadCore(void*);
	void doDownloadCore();

	static int callProgressDLStatus(void*, double , double, double, double);
	int doProgressDLStatus(double , double, double, double);

	static size_t writeDLStream(void*, size_t, size_t, FILE*);
	static size_t readDLStream(void*, size_t, size_t, FILE*);

	// Metodi Richiamabili dallo ScriptablePeer

	//Funzione di trasporto dei comandi da JS al Core
	std::string execCoreCommand(std::string line);
	//Funzione di trasporto dei comandi da JS alla Shell
	std::string execShellCommand(std::string line);

	//NPError SetWindow(NPWindow* aWindow);

	void sendWarnMessage(std::string message);
	
//TODO: controllare che se è possibile metterlo private
#if defined(XP_UNIX)
	//set X11 windows id
	NPError SetWindow(NPWindow* aWindow);
	//get X11 windows id
	Widget GetWidget() { return mXtwidget; }
	//get X11 windows process id
	XtWorkProcId GetWorkProcId() { return mXtprocid; }
	//set X11 windows process id
	void SetWorkProcId( XtWorkProcId procid ) { mXtprocid = procid; }
	//restituisce la frequenza di clock del timer
	XtIntervalId GetTimerId() { return mXttimeid; }
	//setta la frequenza di clock del timer
	void SetTimerId( XtIntervalId timeid ) { mXttimeid = timeid; }
#endif

private:
	//restituisce la directory dove è presente il plugin
	nsresult getCurrPlugDir(std::string* dir, const char* diropt);
	//restituisce i settaggi del proxy 
	nsresult getProxySettings(std::string &proxyhostname, std::string &port);

	//segnale di terminazione del main thread di rendering
	NPBool getThreadShutDown() { return mShutdownThread; }
	//segnale di terminazione del thread di download dei core avanzati
	NPBool getDlCoreThreadShutDown() { return mDlCoreShutdownThread; }
	//Rilascia il Thread di Download
	bool releaseDownloadCore();
	//Direttive interne al LoadCore
	std::string loadingCoreCommand(std::string line);
	//Download / Unpack Adv Core Package
	bool downloadUnpackPackage(std::string packagename);

	//istanza del plugin
	NPP mInstance;
	//controllo di inizializzazione
	NPBool mInitialized;
	
	//Installation Directory
	std::string *mInstDir;

	//Local Installation Directory
	std::string *mLocalInstDir;

	//Temporary Directory
	std::string *mTempDir;

	//Default Core Install Directory
	std::string *mCoreInstallDir;
	
	//AdvancedCore Downloading Address
	std::string *mADVCore;
	//AdvancedCore Hash String
	std::string *mADVCoreSHA1Hash;
	//AdvancedCore Dependacies Address
	std::string *mADVCoreDep;
	//AdvancedCore Dependacies Hash String
	std::string *mADVCoreDepSHA1Hash;
	//AdvancedCore Initialization Options
	std::string *mADVInitOptions;
	//AdvancedCore Start Options
	std::string *mADVStartOptions;
	//LoadCore Init Option String
	std::string *mLoadCBOptions;

	//Puntatore allo Scriptable Peer
	nsScriptablePeer * mScriptablePeer;

	//Proxy HostName
	std::string mProxyHName;
	//Proxy Port
	std::string mProxyPort;

	PRThread* mThread;
	NPBool mShutdownThread;

	PRThread* mDlCoreThread;
	NPBool mDlCoreShutdownThread;

	ShellBase mShellBase;

	NPWindow* mWindow;

	NPBool mLoading;

#if defined(WIN32)
	WNDPROC lpOldProc;
	//ID di finestra di Windows
	HWND mhWnd;

#if defined(RENDER_WITH_TIMER_EVENT)
	//Timer Delay variable
	unsigned int mTimer;
#endif
#else
	//ID di finestra di X11
	Window mhWnd;
	//ID del display di X11
	Display *mDisplay;
	//coordinate x,y di finestra
	int mX, mY;
	//altezza e larghezza della finestra
	int mWidth, mHeight;
	Visual* mVisual;
	//colormap
	Colormap mColormap;
	//profondità
	unsigned int mDepth;
	XFontStruct *mFontInfo;

	//X11 widget
	Widget mXtwidget;
	//X11 timer
	XtWorkProcId mXtprocid;
	//Intervallo di ripetizione del timer
	XtIntervalId mXttimeid;
#endif
};



#endif //__NPOSG4WEB_PLUGIN_H__
