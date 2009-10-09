#ifndef __OSG4WEB_COREPICK__
#define __OSG4WEB_COREPICK__ 1


#include <CoreBase/CoreBase.h>
#include <Defines.h>


using namespace CommonCore;

#include <osgText/Text>

/***********************************************************************
 *
 * CorePick example class
 *
 ***********************************************************************/
class CorePick : public CoreBase
{
public:
	//Costruttore/Distruttore
	CorePick(std::string corename = OSG4WEB_COREPICK_NAME);
	~CorePick();
	
	//Return Class Name
	virtual const char* className() { return OSG4WEB_COREPICK_LIBNAME; };
	//Return Class Version
	virtual const char* classVersion() { return OSG4WEB_COREPICK_VERSION; };

	//TIPS: in questo caso lascio inalterato il caricamento e la registrazione dei comandi
	
protected:
	//Ridefinizione dell'albero di scena
	bool initSceneData();

	osg::Node* createHUD(osgText::Text* updateText);
};

#endif //__OSG4WEB_COREPICK__




