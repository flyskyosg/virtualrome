#ifndef __OSG4WEB_CORESCENEMODIFIER__
#define __OSG4WEB_CORESCENEMODIFIER__ 1


#include <CoreBase/CoreBase.h>
#include <Defines.h>
#include <CommonCore/SceneHandlers/SceneModifier.h>

using namespace OSG4WebCC;


/***********************************************************************
 *
 * CoreSceneModifier example class
 *
 ***********************************************************************/
class CoreSceneModifier : public CoreBase
{
public:
	//Costruttore
	CoreSceneModifier(std::string corename = OSG4WEB_CORESCENEMODIFIER_NAME);
	//Distruttore
	~CoreSceneModifier();
	
	//Return Class Name
	virtual const char* className() { return OSG4WEB_CORESCENEMODIFIER_LIBNAME; };
	//Return Class Version
	virtual const char* classVersion() { return OSG4WEB_CORESCENEMODIFIER_VERSION; };
	
protected:
	//Carica un modello nella scena
	virtual bool loadModel(std::string nodename, bool erase = true);

	//Costruttore di scena;
	bool buildMainScene();

	//Ridefinizione dell'albero di scena
	bool initSceneData(void);

	//Modificatore di scena
	osg::ref_ptr<SceneHandlers::SceneModifier> _SceneModifier;

	//Main Node
	osg::ref_ptr<osg::Group> _MainNode;
	
};

#endif //__OSG4WEB_CORESCENEMODIFIER__




