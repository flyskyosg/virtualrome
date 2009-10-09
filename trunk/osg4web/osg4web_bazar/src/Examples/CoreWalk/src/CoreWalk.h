#ifndef __OSG4WEB_COREWALK__
#define __OSG4WEB_COREWALK__ 1


#include <CoreBase/CoreBase.h>
#include <CommonCore/Manipulators/walkManipulator.h>
#include <Defines.h>

using namespace CommonCore;


/***********************************************************************
 *
 * CoreWalk example class
 *
 ***********************************************************************/
class CoreWalk : public CoreBase
{
public:
	//Costruttore/Distruttore
	CoreWalk(std::string corename = OSG4WEB_COREWALK_NAME);
	~CoreWalk();
	
	//Return Class Name
	virtual const char* className() { return OSG4WEB_COREWALK_LIBNAME; };
	//Return Class Version
	virtual const char* classVersion() { return OSG4WEB_COREWALK_VERSION; };

protected:
	//Carica un modello nella scena
	virtual bool loadModel(std::string nodename, bool erase = true);

	//Ridefinizione del Manipolatore
	bool initManipulators(void);

	//Walk Manipolator
	osg::ref_ptr<Manipulators::walkManipulator> _WalkManip;

};

#endif //__OSG4WEB_COREWALK__




