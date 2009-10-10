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
		LogInterface(std::string logname) : _LogName(logname),
			_coutbuf(NULL),
			_cerrbuf(NULL),
			_fout(NULL)
		{
		
		}

		~LogInterface()
		{
			restoreLogMessages();

			_fout = NULL;
			_coutbuf = NULL;
			_cerrbuf = NULL;
		}

		//Messaggi di Warning
		virtual void sendWarnMessage(std::string warnmessage)
		{
			if(_fout != NULL)
				std::cerr.rdbuf( _fout->rdbuf() );
	
			std::cerr << "ERROR: "<< _LogName << "::" << warnmessage << std::endl;
		}

		//Messaggi di Notify
		virtual void sendNotifyMessage(std::string notifymessage)
		{
			if(_fout != NULL)
				std::cout.rdbuf( _fout->rdbuf() );
	
			std::cout << "NOTIFY: " << _LogName << "::" << notifymessage << std::endl;
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
	
			std::string fname(this->getLogName() + "_log_");

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

				_fout = NULL;
				_coutbuf = NULL;
				_cerrbuf = NULL;

				return true;
			}

			return false;
		}

		virtual bool isLogMessagesInitialized()	{ return (_coutbuf || _cerrbuf); }

		virtual std::string getLogName() { return _LogName; }

	private:
		//Strutture per il log
		std::streambuf* _coutbuf;
		std::streambuf* _cerrbuf;
		std::ofstream* _fout;
		
		//Init dei Log
		std::string _LogName;
	
	};
}

#endif //__OSG4WEBCORE_LOGINTERFACE__