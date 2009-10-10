#ifndef __OSG4WEB_FUNCORE__
#define __OSG4WEB_FUNCORE__ 1


#include <CoreBase/CoreBase.h>
#include <FunCore/Defines.h>


using namespace OSG4WebCC;

/***********************************************************************
 *
 * FunCore example class
 *
 ***********************************************************************/
class FunCore : public CoreBase
{
public:

	//Costruttore/Distruttore
	FunCore(std::string corename = OSG4WEB_FUNCORE_NAME);
	~FunCore();
	
	//Return Class Name
	virtual const char* className() { return OSG4WEB_FUNCORE_LIBNAME; };
	//Return Class Version
	virtual const char* classVersion() { return OSG4WEB_FUNCORE_VERSION; };

	//Inizializza le Opzioni del core successivamente all'inizializzazione
	virtual void AddStartOptions(std::string str, bool erase = true);
	//Ridefinizioni del Gestore dei Comandi
	//virtual std::string handleAction(std::string action, std::string argument);

protected:

};

#endif //__OSG4WEB_FUNCORE__




