#ifndef __OSG4WEB_LOADCORE__
#define __OSG4WEB_LOADCORE__ 1


#include <CoreBase/CoreBase.h>
#include <LoadCore/Defines.h>
#include <CommonCore/Manipulators/NullManipulator.h>

#include <osg/ShapeDrawable>

using namespace CommonCore;

/***********************************************************************
 *
 * LoadCore example class
 *
 ***********************************************************************/
class LoadCore : public CoreBase
{
public:

	enum LoadCoreActions
	{
		LOAD_MODEL = 0,
		STATUSBAR_VALUE,
		STATUSBAR_COLOR,
		STATUSBAR_VISIBILITY,
		SETMESSAGE,
		SETMESSAGE_COLOR
	};

	//Costruttore/Distruttore
	LoadCore(std::string corename = OSG4WEB_LOADCORE_NAME);
	~LoadCore();
	
	//Return Class Name
	virtual const char* className() { return OSG4WEB_LOADCORE_LIBNAME; };
	//Return Class Version
	virtual const char* classVersion() { return OSG4WEB_LOADCORE_VERSION; };

	//Inizializza le Opzioni del core successivamente all'inizializzazione
	virtual void AddStartOptions(std::string str, bool erase = true) { }

	//Ridefinizioni del Gestore dei Comandi
	virtual std::string handleAction(std::string argument);

protected:
	//Ridefinizione del Manipolatore
	bool initManipulators();
	bool initSceneData();
	bool loadModel(std::string, bool erase = true);
	void preFrameUpdate(void);

	bool refreshStatusBarValue(std::string value);
	bool setStatusBarColor(std::string colorname);
	bool setStatusBarVisibility(std::string value = std::string());

	osg::Node* createOSG4WebLogoText();
	osg::Node* createOSG4WebMessages();
	bool createOSG4WebLogo();

	bool setHUDMessage(std::string msg = std::string());
	bool setHUDMessageColor(std::string);


	osg::Timer* _MessageTimer;

	osg::ref_ptr<osg::Node> _LoadedNode;
	osg::ref_ptr<osg::ShapeDrawable> _CapsuleBarSD;
	osg::ref_ptr<osg::Switch> _CapsuleBarSwitch;

	osg::ref_ptr<osgText::Text> _TextMessage;
	osg::ref_ptr<osg::Switch> _SwitchMessage;

	//Null Manipolator
	osg::ref_ptr<Manipulators::NullManipulator> _NullManipulator;

};

#endif //__OSG4WEB_LOADCORE__




