#ifndef __OSG4WEB_COREJSCALL__
#define __OSG4WEB_COREJSCALL__ 1


#include <CoreBase/CoreBase.h>
#include <Defines.h>


using namespace OSG4WebCC;

#include <osgText/Text>

/***********************************************************************
 *
 * CoreJSCall example class
 *
 ***********************************************************************/
class CoreJSCall : public CoreBase
{
public:
	//Costruttore/Distruttore
	CoreJSCall(std::string corename = OSG4WEB_COREJSCALL_NAME);
	~CoreJSCall();
	
	//Return Class Name
	virtual const char* className() { return OSG4WEB_COREJSCALL_LIBNAME; };
	//Return Class Version
	virtual const char* classVersion() { return OSG4WEB_COREJSCALL_VERSION; };

	//TIPS: in questo caso lascio inalterato il caricamento e la registrazione dei comandi
	
protected:
	//Ridefinizione dell'albero di scena
	bool initSceneData();

};

#endif //__OSG4WEB_COREJSCALL__




