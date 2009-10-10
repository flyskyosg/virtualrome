#ifndef __OSG4WEB_CORETOOLTIPS__
#define __OSG4WEB_CORETOOLTIPS__ 1

#include <CoreBase/CoreBase.h>
#include <CommonCore/SceneHandlers/TooltipHandler.h>
#include <Defines.h>

using namespace OSG4WebCC;

/***********************************************************************
 *
 * CoreTooltips example class
 *
 ***********************************************************************/
class CoreTooltips : public CoreBase
{
public:
	//Costruttore
	CoreTooltips(std::string corename = OSG4WEB_CORETOOLTIPS_NAME);
	//Distruttore
	~CoreTooltips();
	
	//Return Class Name
	virtual const char* className() { return OSG4WEB_CORETOOLTIPS_LIBNAME; };
	//Return Class Version
	virtual const char* classVersion() { return OSG4WEB_CORETOOLTIPS_VERSION; };
	
	//TIPS: Ridefinisco questa funzione per non far attaccare il CommandSchedule "This" al Registry dei comandi
	virtual void AddStartOptions(std::string str, bool erase = true) { }
		
protected:
	//Inizializzo l'albero di scena
	bool initSceneData();

	//Crea lo SceneGraph per i Tooltip
	osg::Node* createScene();

	//Handler Tooltips
	osg::ref_ptr<SceneHandlers::TooltipHandler> _TooltipsHandler;
};

#endif //__OSG4WEB_CORETOOLTIPS__




