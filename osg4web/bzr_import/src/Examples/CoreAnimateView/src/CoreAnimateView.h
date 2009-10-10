#ifndef __OSG4WEB_COREANIMATEVIEW__
#define __OSG4WEB_COREANIMATEVIEW__ 1


#include <CoreBase/CoreBase.h>
#include <Defines.h>
#include <CommonCore/SceneHandlers/AnimateViewHandler.h>

using namespace OSG4WebCC;


/***********************************************************************
 *
 * CoreAnimateView example class
 *
 ***********************************************************************/
class CoreAnimateView : public CoreBase
{
public:
	//Costruttore
	CoreAnimateView(std::string corename = OSG4WEB_COREANIMATEVIEW_NAME);
	//Distruttore
	~CoreAnimateView();
	
	//Return Class Name
	virtual const char* className() { return OSG4WEB_COREANIMATEVIEW_LIBNAME; };
	//Return Class Version
	virtual const char* classVersion() { return OSG4WEB_COREANIMATEVIEW_VERSION; };

protected:
	//Ridefinisco l'albero di scena
	bool initSceneData();

	//Animate Handler
	osg::ref_ptr<SceneHandlers::AnimateViewHandler> _AnimateHandler;
};

#endif //__OSG4WEB_COREANIMATEVIEW__




