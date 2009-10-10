#ifndef __OSG4WEB_SHELLBASE__
#define __OSG4WEB_SHELLBASE__ 1


#include <CommonCore/DynamicLoad.h>
#include <CommonCore/CoreInterface.h>
#include <Utilities/EnvUtils.h>

#include <CommonShell/Defines.h>

#include <unrarlib.h>

#include <iostream>
#include <map>

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

	bool startLoadingBaseCore();
	bool startLoadingAdvancedCore();
	bool initializeAdvancedCore();

	bool closeAllLibraries();

	bool isRunning(){ return m_coreInit; }
	
	bool doRendering();

	std::string getErrorString();
	std::string getErrorString(unsigned int no);

	bool isThereErrors(){ return m_errorcode != 0; }
	void setErrorCode(unsigned int no) { m_errorcode = no; }

	std::string getEnvironmentTempDirectory();
	std::string getEnvironmentAppDirectory();
 
	std::string	getAdvancedCore() { return m_advancedcore; };
	std::string getAdvancedCoreAddress();
	std::string getAdvancedCoreFileName();
	std::string getAdvancedCoreSHA1();
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

	//Settaggio dei parametri di configurazione
	typedef std::pair<std::string, std::string> OptionPair;
	
	//Settaggio parametri iniziali
	void setInitOption(OptionPair opair) { m_initOptionsMap.insert(opair); }
	void setInitOption(std::string name, std::string value) { this->setInitOption(OptionPair(name, value)); }
	bool getInitOption(std::string name, std::string& value);

	//Funzione di configurazione dei parametri iniziali
	bool configuringInitialOptions();

	//Settaggio parametri iniziali
	void setObjectShellOption(OptionPair opair) { m_objectShellOptionsMap.insert(opair); }
	void setObjectShellOption(std::string name, std::string value) { this->setObjectShellOption(OptionPair(name, value)); }
	bool getObjectShellOption(std::string name, std::string& value);

	//Funzione di configurazione dei parametri provenienti da Object
	bool configuringObjectOptions();
	
protected:
	bool initializeLogMessages(std::string logname);
	void initializeErrorMessages();
	bool restoreLogMessages();
	bool isLogMessagesInitialized()	{ return (m_coutbuf || m_cerrbuf); }

	bool loadDynamicCore(std::string);
	bool deleteCurrentCore();

	bool startRendering();
	bool stopRendering();

	bool checkLoadCorePresence();

	std::string getInitLoadCoreOptions();
	
	std::string getInitAdvancedCoreOptions();
	std::string generateAdvancedCoreDirectory();

	void setAdvancedCoreDirectory(std::string dir);

	//Return Shell Version
	std::string getShellVersion();

	//Caricamento RunTime
	DynamicLoad* m_DynLoad;

	//Core Interface
	CommonCore::CoreInterface* m_CoreInterface;

	Utilities::EnvUtils::Environment m_Environm;

	//Parametri del LoadCore
	std::string	m_loadcorename;
	std::string	m_loadcoredir;

	//Parametri di Advanced Core
	std::string	m_advancedcore;
	std::string	m_advancedcoredir;

	//Mappa delle opzioni iniziali
	std::map<std::string,std::string> m_initOptionsMap;

	//Mappa delle opzioni passate tramite gli argomenti di HTML Object o Embedded
	std::map<std::string,std::string> m_objectShellOptionsMap;

	//Controllo di inizializzazione
	bool m_coreInit;

	//Stato di errore
	unsigned int m_errorcode;
	std::vector<std::string> m_errormessage;
	
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