#ifndef __OSG4WEBCORE_COMMANDSCHEDULE__
#define __OSG4WEBCORE_COMMANDSCHEDULE__ 1

#include <string>
#include <vector>

/************************************************
 *
 * OSG4WEB CommandSchedule Class.
 *
 ************************************************/

class CommandSchedule
{
public:
	//Costruttore
	CommandSchedule(std::string name) : _cschedulename(name) {  }

	//Set Get CommandSchedule Name
	std::string getCommandScheduleName() { return _cschedulename; }
	void setCommandScheduleName(std::string name) { _cschedulename = name; }

	//Definizione di CommandActions
	typedef std::vector<std::string> CommandActions;

	//Ritorna la lista dei comandi accettati
	CommandActions getCommandActions() { return _commandactions; }
	
	//Aggiunge un azione a questo Schedule
	void setCommandAction(std::string caction) { _commandactions.push_back(caction); }

	//Clear della Command Action List
	void clearCommandActions() { _commandactions.clear(); }

	//Ritorna l'indice del vettore di comando
	unsigned int getCommandActionIndex(std::string caction)
	{
		unsigned int i;
		for(i = 0; i < _commandactions.size(); i++)
			if( _commandactions.at(i) == caction )
				return i;

		return 0; //0 deve esser sempre UNKONOWN_COMMAND per ogni handler
	}

	//Controlla se il comando passato fa parte di questo Schedule
	bool isMineCommandAction(std::string caction)
	{
		CommandActions::iterator itr = _commandactions.begin();
		for( ; itr != _commandactions.end(); itr++)
			if( (*itr) == caction )
				return true;

		return false;
	}

	//Divide il comando in command e argument
	virtual void splitActionCommand(std::string command, std::string& largs, std::string& rargs, std::string delimiter = " ")
	{
		std::string::size_type pos = command.find( delimiter );

		largs = command.substr(0, pos);

		if( pos != std::string::npos )
			rargs = command.substr(pos + 1, command.size() -1);
		else
			rargs.clear();
	}

	//Funzione virtuale di gestione dei comandi accettati
	virtual std::string handleAction(std::string argument) = 0;

private:

	//Contenitore dei Command Actions accettati dallo Scheduler
	CommandActions _commandactions;
	//Nome della classe
	std::string _cschedulename;
};





#endif //__OSG4WEBCORE_COMMANDSCHEDULE__