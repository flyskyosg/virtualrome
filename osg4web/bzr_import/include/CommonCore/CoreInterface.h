#ifndef __OSG4WEBCORE_COREINTERFACE__
#define __OSG4WEBCORE_COREINTERFACE__ 1


#include <string>
#include <vector>

#if defined(WIN32)
#include <windows.h>
#include <process.h>
#else
#include <unistd.h>
#include <X11/Xlib.h>
#endif

#include <CommonCore/Export.h>
#include <CommonCore/Defines.h>
#include <CommonCore/RaiseEventInterface.h>
#include <CommonCore/LogInterface.h>


namespace CommonCore
{
	/************************************************
	 *
	 * OSG4Web CoreInterface Class.
	 *
	 ************************************************/

	class CoreInterface : public RaiseEventInterface, public LogInterface
	{
	public:
		CoreInterface(std::string corename = OSG4WEB_COREINTERFACE_NAME) : LogInterface(corename)
		{ }

		//Distruttore dell Core Interface
		~CoreInterface() { }

		//Ritorna il nome della Classe
		virtual const char* className() const { return OSG4WEB_COREINTERFACE_LIBNAME; }

		//Ritorna la versione della Classe
		virtual const char* classVersion() const {return OSG4WEB_COREINTERFACE_VERSION; };
		
		//Funzione di inizializzazione del core
#if defined(WIN32)
		virtual bool InitCore(WINDOWIDTYPE, std::string, std::string) { return false; }; //Windows
#else 
		virtual bool InitCore(Display*, WINDOWIDTYPE, std::string, std::string) { return false; }; //Linux (other platforms)
#endif

		//Setta direttamente le Opzioni per il Core
		virtual void AddStartOptions(std::string, bool erase = true) {};

		//Ritorna lo stato di fine del rendering (true se il rendering è concluso correttamente)
		virtual bool isDone() { return false; };

		//Segnala al core di preparasi alla chiusura del rendering
		virtual void setDone() {};

		//Singola passata di rendering
		virtual bool RenderScene(){ return false; };

		//Funzione di gestione dai messaggi da Javascript
		virtual std::string DoCommand(std::string) { return std::string("IMPLEMENT ME"); } ;

	};

	//Definizioni
	typedef CoreInterface* (createCoreInterfaceFunction)(void);
	typedef void (deleteCoreInterfaceFunction)(CoreInterface*);
}

#endif //__OSG4WEBCORE_COREINTERFACE__