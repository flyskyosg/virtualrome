#ifndef __OSG4WEB_SHELLBASE__
#define __OSG4WEB_SHELLBASE__ 1


#include <CommonCore/DynamicLoad.h>
#include <CommonCore/CoreInterface.h>
#include <Utilities/EnvUtils.h>

#include <CommonShell/Defines.h>
#include <CommonShell/ShellOption.h>
#include <CommonShell/ShellRenderer.h>
#include <CommonShell/ShellDownloader.h>

#include <iostream>

class ShellBase
{
public:
	ShellBase();
	~ShellBase();

	//Return Shell Version
	std::string ShellBase::getShellVersion() { return std::string(SHELL_VERSION_STRING); }

	//Core / Shell Command handlers
	std::string execShellCommand(std::string);
	std::string execCoreCommand(std::string);
	
	//Set IE / PLugin Instance
	void setInstanceHandler(void* classptr) { m_instanceclassptr = classptr; };
	//Transport Event: allow the shellbase to call plugins functions
	void setTransportEventHandler(void* eventfptr) { m_eventfuncptr = eventfptr; };
	//Funzione di reset della window id
	void setResetWindowHandler(void* resetwhptr) { m_resetwclassptr = resetwhptr; }; //FIXME
	
	//Handle main loading and advanced cores
	bool startLoadingBaseCore();
	bool initializeAdvancedCore();
	bool closeAllLibraries();

	//Carica il core Downloadato
	bool checkAdvCorePresence();
	bool startDownloadedCore(bool timing = true);

	//Debugging redirection
	void initializeLog(std::string logname);
	void sendWarnMessage(std::string warnmessage);
	void sendNotifyMessage(std::string notifymessage);

	//Settaggio dei parametri di configurazione
	typedef std::pair<std::string, std::string> OptionPair;

	//Settaggio opzioni da plugin
	void setInitOption(std::string name, std::string value) { m_initOption.setShellOption(name, value); }
	void setObjectOption(std::string name, std::string value) { m_objectOption.setShellOption(name, value); }

	//Funzione di configurazione dei parametri iniziali
	bool configuringInitialOptions();
	//Funzione di configurazione dei parametri provenienti da Object
	bool configuringObjectOptions();

	//Handle error functions
	std::string getErrorString();
	std::string getErrorString(unsigned int no);

	bool isThereErrors(){ return m_errorcode != 0; }
	void setErrorCode(unsigned int no) { m_errorcode = no; }

	//Rendering Functions
	bool isRunning(){ return m_coreInit; }
	bool prepareRendering();
	bool closeRendering();
	bool doRenderingBridge();
	bool setDoneBridge();

	//IE
	std::string getEnvironmentTempDirectory();
	std::string getEnvironmentAppDirectory();

	//TODO: nuove funzioni
	int doProgressDLStatus(double downtot, double downnow, double ultotal, double ulnow);
	int doProgressUnpackStatus(unsigned int cicleno, int filenumber);

	void startDownloading(bool timing);
	void checkSecurityString(bool timing);
	void startUnPackSession(bool timing);
	void downloadFinished(bool timing);
	void downloadError(int error);
	void securityStringError();
	void unpackError(int error);

	void requestExplicitRendering();  //FIXME: finire questa parte ... si inkioda se attivo 

//Sets Windows Handler
#if defined (WIN32)
	void setWindowHandler( HWND hwnd ) { m_hWnd = hwnd; }
#else
	 //TODO Linux
#endif

protected:
	//Log messages functions
	bool initializeLogMessages(std::string logname);
	void initializeErrorMessages();
	bool restoreLogMessages();
	bool isLogMessagesInitialized()	{ return (m_coutbuf || m_cerrbuf); }


	//Files functions
	bool checkFileExistance(std::string fname);
	bool removingFile(std::string fname);
	bool getOrCreateDirectory(std::string dir);
	
	//Starts loading the advanced core	
	bool startLoadingAdvancedCore();

	//Handles Dynamic Libraries
	bool loadDynamicCore(std::string);
	bool deleteCurrentCore();

	//Rendering functions
	bool startRendering();
	bool stopRendering();

	bool checkLoadCorePresence();
	bool checkAdvCoreDependaciesPresence();

	std::string getInitLoadCoreOptions();
	std::string getInitAdvancedCoreOptions();



private:
	//Controllo di inizializzazione del Core caricato 
	bool m_coreInit;
	//Controllo sul ciclo di rendering del core
	bool m_doRender;

	//RunTime Loaded Library
	DynamicLoad* m_DynLoad;
	//Core Interface (Handle loaded Core)
	CommonCore::CoreInterface* m_CoreInterface;

	//System Environment
	Utilities::EnvUtils::Environment m_Environm;

	//ShellBase Options
	ShellOption m_initOption, m_objectOption, m_generalOption;

	//Error Messages Status
	unsigned int m_errorcode;
	std::vector<std::string> m_errormessage;

	//Redirect dei log
	std::streambuf* m_coutbuf;
	std::streambuf* m_cerrbuf;
	std::ofstream* m_fout;

	//Plugin / IE instance
	void* m_instanceclassptr;
	//Event callback pointer for Core -> JS messages
	void* m_eventfuncptr;
	//Reset window handler callback pointer
	void* m_resetwclassptr; //FIXME: da rifare 

	//Rendering Thread
	ShellRenderer* m_Renderer;
	//Downloader Thread
	ShellDownloader* m_Downloader;

#if defined(WIN32)
	//ID di finestra di Windows
	HWND m_hWnd;
	HANDLE m_ThreadHandle;
#else

#endif
};


#endif //__OSG4WEB_SHELLBASE__