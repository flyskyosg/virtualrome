#ifndef __OSG4WEB_COREJSCALL__
#define __OSG4WEB_COREJSCALL__ 1


#include <CoreBase/CoreBase.h>
#include <CommonCore/SceneHandlers/GetBackHandler.h>
#include <Defines.h>


using namespace CommonCore;

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
	
protected:
	//Ridefinizione dell'albero di scena
	bool initSceneData();

	//Handler NodeParser per JS Call
	osg::ref_ptr<SceneHandlers::NodeParserHandler> _ParserHandler;

	//Handler per GetBack Calls
	osg::ref_ptr<SceneHandlers::GetBackHandler> _GetBackHandler;

};

#endif //__OSG4WEB_COREJSCALL__




