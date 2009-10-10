
#include <FunCore/FunCore.h>


#include <osg/MatrixTransform>

#include <osgDB/ReadFile>

#include <osgUtil/Optimizer>

using namespace OSG4WebCC;


//FunCore Costruttore
FunCore::FunCore(std::string corename) : CoreBase(corename)
{
	this->sendNotifyMessage("FunCore -> Costructing FunCore Instance.");
}

//FunCore Distruttore
FunCore::~FunCore()
{
	this->sendNotifyMessage("~FunCore -> Destructing FunCore Instance.");
}

//Setta le opzioni del core successivamente alla inizializzazione
void FunCore::AddStartOptions(std::string str, bool erase)
{
	this->sendNotifyMessage("AddStartOptions -> Adding Starting Options.");
/*
	this->setCommandAction("LC_LOAD_MODEL");
	this->setCommandAction("LC_STATUSBAR_VALUE");
	this->setCommandAction("LC_STATUSBAR_COLOR");
	this->setCommandAction("LC_STATUSBAR_VISIBILITY");
	this->setCommandAction("LC_SETMESSAGE");
	this->setCommandAction("LC_SETMESSAGE_COLOR");

	this->addCommandSchedule((CommandSchedule*) this);
*/
}

/** Ridefinizione della funzione di Gestione Comandi per CommandSchedule "this" */
/*
std::string LoadCore::handleAction(std::string action, std::string argument)
{
	std::string retstr = "CORE_DONE";

	this->sendNotifyMessage("handleAction -> Command Found");

	switch(this->getCommandActionIndex( action ))
	{
	case LOAD_MODEL: //LC_LOAD_MODEL
		if( !this->loadModel( argument, false ) )
			retstr = "CORE_FAILED";
		break;
	case STATUSBAR_VALUE: //LC_STATUSBAR_VALUE
		if( !this->refreshStatusBarValue( argument ) )
			retstr = "CORE_FAILED";
		break;
	case STATUSBAR_COLOR: //LC_STATUSBAR_COLOR
		if( !this->setStatusBarColor( argument ) )
			retstr = "CORE_FAILED";
		break;
	case STATUSBAR_VISIBILITY: //LC_STATUSBAR_VISIBILITY
		if( !this->setStatusBarVisibility( argument ) )
			retstr = "CORE_FAILED";
		break;
	case SETMESSAGE: //LC_SETMESSAGE
		if( !this->setHUDMessage( argument ) )
			retstr = "CORE_FAILED";
		break;
	case SETMESSAGE_COLOR: //LC_SETMESSAGE_COLOR
		if( !this->setHUDMessageColor( argument ) )
			retstr = "CORE_FAILED";
		break;
	default: //UNKNOWN_CORE_COMMAND
		retstr = "UNKNOWN_CORE_COMMAND";
		break;
	}
	
	return retstr;
}
*/
