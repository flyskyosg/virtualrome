#ifndef __OSG4WEBCORE_RAISEEVENTINTERFACE__
#define __OSG4WEBCORE_RAISEEVENTINTERFACE__ 1

#include <string>

namespace CommonCore
{

	/************************************************
	 *
	 * OSG4Web RaiseEventInterface Class.
	 *
	 ************************************************/

	class RaiseEventInterface
	{
	public:
		RaiseEventInterface() : _EventFPtr(NULL), _DownlFPtr(NULL), _ClassEPtr(NULL) { }

		RaiseEventInterface(void* efptr,void* ceptr) : _EventFPtr(efptr), _DownlFPtr(NULL), _ClassEPtr(ceptr) { }
		
		~RaiseEventInterface()
		{
			if(_EventFPtr)
				_EventFPtr = NULL;

			if(_DownlFPtr)
				_DownlFPtr = NULL;

			if(_ClassEPtr)
				_ClassEPtr = NULL;
		}

		//Inizializza la funzione di Raise a Javascript
		void setEventBridge(void* classptr, void* eventptr, void* downlptr) 
		{ 
			_ClassEPtr = classptr;  
			_EventFPtr = eventptr; 
			_DownlFPtr = downlptr;
		}

		//Inizializza la funzione di Raise a Javascript
		bool getEventBridge(void* classptr, void* eventptr, void* downlptr) 
		{
			if(_ClassEPtr && _EventFPtr)
			{
				classptr = _ClassEPtr;
				eventptr = _EventFPtr; 
				downlptr = _DownlFPtr;

				return true;
			}
			else
				return false;
		}

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

		//Trasporta un messaggio a Javascript
		bool raiseDownload(std::string downloadurl)
		{
			if( (_DownlFPtr == NULL) || (_ClassEPtr == NULL))
				return false;

			typedef bool (sendDownlDefinition)(void*, std::string);
	
			sendDownlDefinition* sendDownload;
			sendDownload = (sendDownlDefinition*) _DownlFPtr;

			return sendDownload(_ClassEPtr, downloadurl);
		}

	private:
		//Puntatore alla funzione di Raise Event
		void* _EventFPtr;
		//Puntatore alla funzione di Raise Download
		void* _DownlFPtr;
		//Puntatore alla istanza della shell
		void* _ClassEPtr;
	
	};
}

#endif //__OSG4WEBCORE_RAISEEVENTINTERFACE__