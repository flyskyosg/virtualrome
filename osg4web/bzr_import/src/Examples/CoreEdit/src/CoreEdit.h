#ifndef __OSG4WEB_COREEDIT__
#define __OSG4WEB_COREEDIT__ 1


#include <CoreBase/CoreBase.h>
#include <Defines.h>
#include <CommonCore/SceneHandlers/PickEditHandler.h>

using namespace OSG4WebCC;


/***********************************************************************
 *
 * EditManipSchedule class
 *
 ***********************************************************************/
/*
class EditManipSchedule : public PickEditHandler, public CommandSchedule
{
public:
	//Costruttore
	EditManipSchedule();

	//Ridefinizioni del Gestore dei Comandi
	virtual std::string handleAction(std::string action, std::string argument);

protected:

};

*/


/***********************************************************************
 *
 * CoreEdit example class
 *
 ***********************************************************************/
class CoreEdit : public CoreBase
{
public:
	//Costruttore/Distruttore
	CoreEdit(std::string corename = OSG4WEB_COREEDIT_NAME);
	~CoreEdit();
	
	//Return Class Name
	virtual const char* className() { return OSG4WEB_COREEDIT_LIBNAME; };
	//Return Class Version
	virtual const char* classVersion() { return OSG4WEB_COREEDIT_VERSION; };

	virtual void AddStartOptions(std::string str, bool erase = true);

	//Ridefinisco l'handler per le azioni di loading
	virtual std::string handleAction(std::string argument);
	
protected:
	//Carica un modello nella scena
	bool loadModel(std::string nodename, bool erase = true) { return false; }

	//Terrain Loading
	bool loadTerrain(std::string address);

	//Gestisco il LoaderThreadHandler
	virtual void preFrameUpdate(void);

	//bool initCameraConfig(void);
	virtual bool initSceneData(void);


	osg::Group* createHUD();

	osg::ref_ptr<osg::Group> _mainterrain;
	osg::ref_ptr<SceneHandlers::PickEditHandler> _picker;

};

#endif //__OSG4WEB_COREEDIT__




