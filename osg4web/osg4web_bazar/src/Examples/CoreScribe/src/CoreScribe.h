#ifndef __OSG4WEB_CORESCRIBE__
#define __OSG4WEB_CORESCRIBE__ 1


#include <CoreBase/CoreBase.h>
#include <Defines.h>

using namespace CommonCore;


/***********************************************************************
 *
 * CoreScribe example class
 *
 ***********************************************************************/
class CoreScribe : public CoreBase
{
public:
	//Costruttore
	CoreScribe(std::string corename = OSG4WEB_CORESCRIBE_NAME);
	//Distruttore
	~CoreScribe();
	
	//Return Class Name
	virtual const char* className() { return OSG4WEB_CORESCRIBE_LIBNAME; };
	//Return Class Version
	virtual const char* classVersion() { return OSG4WEB_CORESCRIBE_VERSION; };

protected:
	//Carica un modello nella scena
	virtual bool loadModel(std::string nodename, bool erase = true);
	
};

#endif //__OSG4WEB_CORESCRIBE__




