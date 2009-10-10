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
		RaiseEventInterface() : _EventFPtr(NULL), _ClassEPtr(NULL) { }

		RaiseEventInterface(void* efptr,void* ceptr) : _EventFPtr(efptr), _ClassEPtr(ceptr) { }
		
		~RaiseEventInterface()
		{
			if(_EventFPtr)
				_EventFPtr = NULL;

			if(_ClassEPtr)
				_ClassEPtr = NULL;
		}

		//Inizializza la funzione di Raise a Javascript
		void setEventBridge(void* classptr, void* eventptr) 
		{ 
			_ClassEPtr = classptr;  
			_EventFPtr = eventptr; 
		}

		//Inizializza la funzione di Raise a Javascript
		bool getEventBridge(void* classptr, void* eventptr) 
		{
			if(_ClassEPtr && _EventFPtr)
			{
				classptr = _ClassEPtr;
				eventptr = _EventFPtr; 

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

	private:
		//Puntatore alla funzione di Raise
		void* _EventFPtr;
		//Puntatore alla istanza della shell
		void* _ClassEPtr;
	
	};
}

#endif //__OSG4WEBCORE_RAISEEVENTINTERFACE__