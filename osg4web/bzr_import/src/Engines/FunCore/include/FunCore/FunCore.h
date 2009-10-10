#ifndef __OSG4WEB_FUNCORE__
#define __OSG4WEB_FUNCORE__ 1


#include <CoreBase/CoreBase.h>
#include <FunCore/Defines.h>


#include <CommonCore/SceneHandlers/AnimateViewHandler.h>
#include <CommonCore/SceneHandlers/SceneModifier.h>
#include <CommonCore/SceneHandlers/LoadThreadsHandler.h>
#include <CommonCore/SceneHandlers/TooltipsSceneModifier.h>
#include <CommonCore/SceneHandlers/GetBackHandler.h>

#include <CommonCore/Manipulators/walkManipulator.h>
#include <CommonCore/Manipulators/ViroManipulator.h>

using namespace CommonCore;

/***********************************************************************
 *
 * FunCore example class
 *
 ***********************************************************************/
class FunCore : public CoreBase
{
public:

	enum FunCoreActions	{
		UNKNOWN_ACTION = 0,
		SETMAINSCENEGRAPH,
		ADDFILETONODE,
		CHECKNODEPRESENCE,
		SETOPTIMIZATION,
		SWITCH_MANIPULATORS,
		RESET_SCENE,
		ACTIVATE_JSMAP_MESSAGES,
		DEACTIVATE_JSMAP_MESSAGES,
		GET_JSMAP_COORDINATES
	};

	//Costruttore/Distruttore
	FunCore(std::string corename = OSG4WEB_FUNCORE_NAME);
	~FunCore();
	
	//Return Class Name
	virtual const char* className() { return OSG4WEB_FUNCORE_LIBNAME; };
	//Return Class Version
	virtual const char* classVersion() { return OSG4WEB_FUNCORE_VERSION; };

	//Inizializza le Opzioni del core successivamente all'inizializzazione
	virtual void AddStartOptions(std::string str, bool erase = true);

	//Attacco gli Handler di Scena
	virtual bool initSceneHandlers();
	
	//Ridefinizioni del Gestore dei Comandi
	virtual std::string handleAction(std::string argument);

protected:
	bool initSceneData();
	bool initManipulators(void);

	bool buildMainScene();

	//Carica un modello nella scena
	virtual bool loadModel(std::string nodename, bool erase = true);
	virtual bool loadModelToNode(std::string arguments);

	virtual void preFrameUpdate();
	
	void handleLoadingThreads();
	void handleEnvironment();
	void handleTooltips();

	void sendLookAtToJS();

	std::string getJSMapCoordinates();

	//Manipolatori
	osg::ref_ptr<Manipulators::walkManipulator> _WalkManip;
	osg::ref_ptr<Manipulators::ViroManipulator> _ViRoMan;

	//Scene Handlers
	osg::ref_ptr<SceneHandlers::GetBackHandler> _GetBackHandler;
	osg::ref_ptr<SceneHandlers::AnimateViewHandler> _AnimateHandler;
	osg::ref_ptr<SceneHandlers::SceneModifier> _SceneModifier;
	osg::ref_ptr<SceneHandlers::LoadThreadsHandler> _LoaderThreadHandler;
	osg::ref_ptr<SceneHandlers::TooltipsSceneModifier> _TooltipsSceneModifier;
	osg::ref_ptr<SceneHandlers::NodeParserHandler> _TooltipsParserHandler;
	osg::ref_ptr<SceneHandlers::NodeParserHandler> _StandardNodeParserHandler;

	//Main Node
	osg::ref_ptr<osg::Group> _MainNode;
	//SceneGraph Modificabile
	osg::ref_ptr<osg::Group> _ModiSceneGraph;
	//Nodo di supporto non linkato nello SceneGraph di View
	osg::ref_ptr<osg::Group> _SupportNode;

	//Tick di invio eventi ciclici a JS
	osg::Timer_t _sendCEventToJSTime;

	//Attivazione messaggi a mappa JS
	bool _jsMapActive;

	//Inizializzazione del Main SceneGraph
	bool _maininit;

	//Environment
	bool _envdone;
};

#endif //__OSG4WEB_FUNCORE__




