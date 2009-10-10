
#include <CoreAnimateView.h>


#include <osg/Material>
#include <osg/PolygonOffset>
#include <osg/PolygonMode>

#include <osgDB/ReadFile>

#include <osgUtil/Optimizer>

using namespace OSG4WebCC;

//CoreAnimateView Costruttore
CoreAnimateView::CoreAnimateView(std::string corename) : CoreBase(corename),
	_AnimateHandler(NULL)
{
	this->sendNotifyMessage("CoreAnimateView -> Costructing CoreExampleScribe Instance.");
}

//CoreAnimateView Distruttore
CoreAnimateView::~CoreAnimateView()
{
	this->sendNotifyMessage("~CoreAnimateView -> Destructing CoreExampleScribe Instance.");

	if(_AnimateHandler.valid())
		_AnimateHandler = NULL;
}

//Ridefinizione dell'albero di scena
bool CoreAnimateView::initSceneData()
{
	_AnimateHandler = new SceneHandlers::AnimateViewHandler(_Viewer.get());

	_Viewer->addEventHandler(_AnimateHandler.get());

	this->addCommandSchedule((CommandSchedule*) _AnimateHandler.get());

	_Viewer->setSceneData(_LocalSceneGraph.get());

	return true;
}
