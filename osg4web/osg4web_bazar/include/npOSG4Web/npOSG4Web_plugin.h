#ifndef __NPOSG4WEB_PLUGIN_H__
#define __NPOSG4WEB_PLUGIN_H__ 1

#define GET_EVENT_THREADSAFE 1

#include "xpcom-config.h"
#include "nspr.h"
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

	//check se l'istanza è inizializzata
	NPBool isInitialized() { return mInitialized; }
	//send error messages at the ShellBase
	void sendWarnMessage(std::string message) {	mShellBase.sendWarnMessage(message); }

	//permette il passaggio dall'involucro all'istanza dei parametri di inizializzazione
	NPError SetInitialData(PRUint16 argc, char* argn[], char* argv[]);
	//inizializza l'istanza del Plug-In
	NPBool init(NPWindow* aWindow);
	//inizializza l'istanza del LoadingCore
	NPBool initLoader();
	//chiude l'istanza del Plug-In
	void shut();
	
	//uint16 HandleEvent(void* aEvent);

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
	void requestExplicitRendering();
	//Controlla se il core è in rendering
	bool checkRunning();
	//Controlla che la shell non sia in stato di errore
	bool showFatalCoreErrors();

	// Metodi Richiamabili dallo ScriptablePeer

	//Funzione di trasporto dei comandi da JS al Core
	std::string execCoreCommand(std::string line);
	//Funzione di trasporto dei comandi da JS alla Shell
	std::string execShellCommand(std::string line);

	//NPError SetWindow(NPWindow* aWindow);

	
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

#if defined(WIN32)
	LRESULT handleWindowEvents(HWND hWnd, UINT eventmsg, WPARAM wParam, LPARAM lParam);
#endif

private:
	//restituisce la directory dove è presente il plugin
	nsresult getCurrPlugDir(std::string* dir, const char* diropt);
	//restituisce i settaggi del proxy 
	nsresult getProxySettings(std::string &proxyhostname, std::string &port);

	//ShellBase
	ShellBase mShellBase;

	//istanza del plugin
	NPP mInstance;
	//controllo di inizializzazione
	NPBool mInitialized;

	//Puntatore allo Scriptable Peer
	nsScriptablePeer * mScriptablePeer;
	NPWindow* mWindow;
	NPBool mLoading;

	//Check di setting opzioni iniziali critiche
	bool mInitOptionsSet;
#if defined(GET_EVENT_THREADSAFE)
	std::string mGetEvent_Message;
	bool mGetEvent_do;
	PRLock* mGetEvent_Lock;
#endif
#if defined(WIN32)
	//Vecchio Window Procedure
	WNDPROC lpOldProc;
	//ID di finestra di Windows
	HWND mhWnd;
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
