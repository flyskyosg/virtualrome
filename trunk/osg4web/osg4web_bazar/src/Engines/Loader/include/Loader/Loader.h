#ifndef __OSG4WEB_LOADER__
#define __OSG4WEB_LOADER__ 1


#include <CoreBase/CoreBase.h>
#include <Loader/Defines.h>
#include <CommonCore/Manipulators/NullManipulator.h>

#include <osg/ShapeDrawable>

using namespace CommonCore;

/***********************************************************************
 *
 * Loader example class
 *
 ***********************************************************************/
class Loader : public CoreBase
{
public:

	enum LoaderActions
	{
		UNKNOWN_ACTION = 0,
		LOAD_MODEL,
		STATUSBAR_VALUE,
		STATUSBAR_COLOR,
		STATUSBAR_VISIBILITY,
		SETMESSAGE,
		SETMESSAGE_COLOR
	};

	//Costruttore/Distruttore
	Loader(std::string corename = OSG4WEB_LOADER_NAME);
	~Loader();
	
	//Return Class Name
	virtual const char* className() { return OSG4WEB_LOADER_LIBNAME; };
	//Return Class Version
	virtual const char* classVersion() { return OSG4WEB_LOADER_VERSION; };

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

	osg::ref_ptr<osg::Camera> _CameraNode;

	osg::ref_ptr<osg::Node> _LoadedNode;
	osg::ref_ptr<osg::ShapeDrawable> _CapsuleBarSD;
	osg::ref_ptr<osg::Switch> _CapsuleBarSwitch;

	osg::ref_ptr<osgText::Text> _TextMessage;
	osg::ref_ptr<osg::Switch> _SwitchMessage;

	//Null Manipolator
	osg::ref_ptr<Manipulators::NullManipulator> _NullManipulator;

};

#endif //__OSG4WEB_LOADER__



