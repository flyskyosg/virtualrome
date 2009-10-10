#ifndef __OSG4WEB_COREFLY__
#define __OSG4WEB_COREFLY__ 1


#include <CoreBase/CoreBase.h>
#include <Defines.h>
#include <CommonCore/Manipulators/FlyManipulator.h>

using namespace OSG4WebCC;

/***********************************************************************
 *
 * CoreFly example class
 *
 ***********************************************************************/
class CoreFly : public CoreBase
{
public:
	//Costruttore/Distruttore
	CoreFly(std::string corename = OSG4WEB_COREFLY_NAME);
	~CoreFly();
	
	//Return Class Name
	virtual const char* className() { return OSG4WEB_COREFLY_LIBNAME; };
	//Return Class Version
	virtual const char* classVersion() { return OSG4WEB_COREFLY_VERSION; };
	
protected:
	//Ridefinizione del Manipolatore
	bool initManipulators();

	//Fly Manipolator
	osg::ref_ptr<Manipulators::FlyManipulator> _FlyManip;
};

#endif //__OSG4WEB_COREFLY__




