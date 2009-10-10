#ifndef __OSG4WEB_SHELLBASE__
#define __OSG4WEB_SHELLBASE__ 1


#include <CommonCore/DynamicLoad.h>
#include <CommonCore/CoreInterface.h>
#include <Utilities/EnvUtils.h>

#include <unrarlib.h>

class ShellBase
{
public:
	ShellBase();
	~ShellBase();

	std::string execShellCommand(std::string);
	std::string execCoreCommand(std::string);

	void setInstanceHandler(void* classptr) { m_instanceclassptr = classptr; };
	void setResetWindowHandler(void* resetwhptr) { m_resetwclassptr = resetwhptr; };
	void setTransportEventHandler(void* eventfptr) { m_eventfuncptr = eventfptr; };
	void setRenderingHandler(void* prender, void* crender) { m_prenderptr = prender; m_crenderptr = crender; };
	void setDownloadingCoreHandler(void* dcore) { m_dcoreptr = dcore; };

	void setWindowHandler(HWND hwnd);
	void setProxyParameters(std::string url, std::string port) { m_proxyport = port; m_proxyurl = url; }

	bool setInstallDirectory(std::string insdir);
	bool setLocalInstallDirectory(std::string insdir);
	bool setTempDirectory(std::string insdir);
	bool setCoreAppDirectory(std::string insdir);

	bool startLoadingBaseCore();
	bool startLoadingAdvancedCore();
	bool initializeAdvancedCore();

	bool closeAllLibraries();

	bool isRunning(){ return m_coreInit; }
	bool isThereErrors(){ return m_errorcode != 0; }

	bool doRendering();

	std::string getErrorString();

	std::string getEnvironmentTempDirectory();
	std::string getEnvironmentAppDirectory();
 
	std::string	getAdvancedCore() { return m_advancedcore; };
	std::string getAdvancedCoreAddress() { return m_advancedcoreaddress; };
	std::string getAdvancedCoreFileName();
	std::string	getAdvancedCoreDirectory(); //FIXME: cambiare in modo tale da usare anche la dir plugin
	
	bool checkFileExistance(std::string fname);
	bool removingFile(std::string fname);
	bool checkOrCreateDirectory(std::string dir);
	bool checkFileValidity(std::string fname, std::string hexhash);
	bool checkAdvCorePresence();	

	//UnPacking Functions
	int openCompressedCore(std::string filename);
	int unpackCompressedCoreFile();
	void freeCompressedCore();

	//Debugging redirection
	void initializeLog(std::string logname);
	void sendWarnMessage(std::string warnmessage);
	void sendNotifyMessage(std::string notifymessage);

protected:
	bool initializeLogMessages(std::string logname);
	bool restoreLogMessages();

	bool loadDynamicCore(std::string);
	bool deleteCurrentCore();

	bool startRendering();
	bool stopRendering();

	bool checkLoadCorePresence();

	std::string getInitLoadCoreOptions();
	
	std::string getInitAdvancedCoreOptions();
	std::string generateAdvancedCoreDirectory();

	//Return Shell Version
	std::string getShellVersion();

	//Caricamento RunTime
	DynamicLoad* m_DynLoad;

	//Core Interface
	OSG4WebCC::CoreInterface* m_CoreInterface;

	Utilities::EnvUtils::Environment m_Environm;

	//Parametri del LoadCore
	std::string	m_loadcorename;
	std::string	m_loadcoredir;
	std::string m_initloadcoreoptions;

	//Parametri di Advanced Core
	std::string	m_advancedcore;
	std::string m_advancedcoreaddress;
	std::string m_advancedcorehash;
	std::string m_advancedcoredepaddress;
	std::string m_advancedcoredephash;
	std::string m_advancedcoresha1;
	std::string	m_advancedcoredir;
	std::string m_initadvancedcoreinitoptions;
	std::string m_initadvancedcorestartoptions;

	bool m_coreInit;

	//Configurazione Directory
	std::string	m_installdir;
	std::string	m_localinstalldir;
	std::string	m_coreinstalldir;
	std::string	m_tempdir;

	//Configurazione Proxy
	std::string	m_proxyport;
	std::string	m_proxyurl;

	unsigned int m_errorcode;
	
#if defined(WIN32)
	//ID di finestra di Windows
	HWND m_hWnd;
	HANDLE m_ThreadHandle;
#else

#endif

	//Write Core File to Disk
	bool writeCoreFileToDisk(char* filename, char* data, unsigned long datasize);

	//UnRAR library Structure
	ArchiveList_struct* m_UnRARList;
	std::string m_TempArchive;

	void* m_instanceclassptr;
	void* m_eventfuncptr;

	void* m_prenderptr;
	void* m_crenderptr;

	void* m_dcoreptr;

	void* m_resetwclassptr;

	bool m_doRender;

	std::streambuf* m_coutbuf;
	std::streambuf* m_cerrbuf;
	std::ofstream* m_fout;
};

#endif //__OSG4WEB_SHELLBASE__