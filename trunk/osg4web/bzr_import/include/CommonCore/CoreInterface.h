#ifndef __OSG4WEBCORE_COREINTERFACE__
#define __OSG4WEBCORE_COREINTERFACE__ 1


#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
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


namespace OSG4WebCC
{

	/************************************************
	*
	* OSG4Web CoreInterface Class.
	*
	************************************************/

	class CoreInterface
	{
	public:
		CoreInterface(std::string corename = OSG4WEB_COREINTERFACE_NAME) : _CoreName(corename),
			_coutbuf(NULL),
			_cerrbuf(NULL),
			_fout(NULL),
			_EventFPtr(NULL),
			_ClassEPtr(NULL)
		{ }

		//Distruttore dell Core Interface
		~CoreInterface()
		{
			if(_EventFPtr)
				_EventFPtr = NULL;

			if(_ClassEPtr)
				_ClassEPtr = NULL;
		}

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
		
		//Inizializza la funzione di Raise a Javascript
		void setEventBridge(void* classptr, void* eventptr) 
		{ 
			_ClassEPtr = classptr;  
			_EventFPtr = eventptr; 
		}

		//Messaggi di Warning
		virtual void sendWarnMessage(std::string warnmessage)
		{
			if(_fout != NULL)
				std::cerr.rdbuf( _fout->rdbuf() );
	
			std::cerr << "ERROR: "<< _CoreName << "::" << warnmessage << std::endl;
		}

		//Messaggi di Notify
		virtual void sendNotifyMessage(std::string notifymessage)
		{
			if(_fout != NULL)
				std::cout.rdbuf( _fout->rdbuf() );
	
			std::cout << "NOTIFY: " << _CoreName << "::" << notifymessage << std::endl;
		}

		bool forcingLogMessages()
		{
			if(!this->isLogMessagesInitialized())
				return this->initializeLogMessages();
			return false;
		}

	protected:
		//Inizializza il salvatagggio dei Log
		virtual bool initializeLogMessages()
		{
			_coutbuf = std::cout.rdbuf();
			_cerrbuf = std::cerr.rdbuf();
	
			std::string fname(this->getCoreName() + "_log_");

			std::ostringstream convstream;
			convstream << this << std::flush;
			fname += convstream.str();

			fname += "_ID.txt";
    
			_fout = new std::ofstream( fname.c_str() );
	
			if(_fout->fail())
			{
				delete _fout;
				_fout = NULL;
				return false;
			}

			std::cout.rdbuf( _fout->rdbuf() );
			std::cerr.rdbuf( _fout->rdbuf() );

			return true;
		}
		
		//Conclude i messaggi di Log
		virtual bool restoreLogMessages()
		{
			if(_fout)
			{
				_fout->close();
	
				delete _fout;

				std::cout.rdbuf( _coutbuf );
				std::cout.rdbuf( _cerrbuf );

				return true;
			}

			return false;
		}

		virtual bool isLogMessagesInitialized()	{ return (_coutbuf || _cerrbuf); }

		//Trasporta un messaggio a Javascript
		bool raiseCommand(std::string eventmessage)
		{
			if( (_EventFPtr == NULL) || (_ClassEPtr == NULL))
				return false;

			typedef bool (sendEventDefinition)(void*, std::string);
	
			sendEventDefinition* sendEvent;
			sendEvent = (sendEventDefinition*) _EventFPtr;

			return sendEvent(_ClassEPtr, eventmessage);
		}

		virtual std::string getCoreName() { return _CoreName; }

	private:

		//Puntatore alla funzione di Raise
		void* _EventFPtr;
		//Puntatore alla istanza della shell
		void* _ClassEPtr;

		//Strutture per il log
		std::streambuf* _coutbuf;
		std::streambuf* _cerrbuf;
		std::ofstream* _fout;

		//Nome del core
		std::string _CoreName;
	};

	//Definizioni
	typedef CoreInterface* (createCoreInterfaceFunction)(void);
	typedef void (deleteCoreInterfaceFunction)(CoreInterface*);
}

#endif //__OSG4WEBCORE_COREINTERFACE__