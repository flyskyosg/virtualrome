#ifndef __OSG4WEBCORE_LOGINTERFACE__
#define __OSG4WEBCORE_LOGINTERFACE__ 1


#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>

namespace CommonCore
{
	/************************************************
	 *
	 * OSG4Web LogInterface Class.
	 *
	 ************************************************/

	class LogInterface
	{
	public:
		LogInterface(std::string logname) : _LogName(logname), _active(false) { }

		~LogInterface() { }

		//Messaggi di Warning
		virtual void sendWarnMessage(std::string warnmessage) { if(_active) std::cerr << "ERROR: "<< _LogName << "::" << warnmessage << std::endl; }

		//Messaggi di Notify
		virtual void sendNotifyMessage(std::string notifymessage) { if(_active) std::cout << "NOTIFY: " << _LogName << "::" << notifymessage << std::endl; }

		virtual bool forcingLogMessages() { return true; }

	protected:
		//Inizializza il salvatagggio dei Log
		virtual bool initializeLogMessages() { _active = true; return true; }
		
		//Conclude i messaggi di Log
		virtual bool restoreLogMessages() {	_active = false; return true; }

		virtual bool isLogMessagesInitialized()	{ return _active; }

		virtual std::string getLogName() { return _LogName; }

	private:
		//Activate Logs
		bool _active;
		//Init dei Log
		std::string _LogName;
	};
}

#endif //__OSG4WEBCORE_LOGINTERFACE__