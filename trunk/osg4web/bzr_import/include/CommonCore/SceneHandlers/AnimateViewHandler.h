#ifndef __OSG4WEB_ANIMATEVIEWHANDLER__
#define __OSG4WEB_ANIMATEVIEWHANDLER__ 1

#include <sstream>

#include <osgViewer/Viewer>
#include <osg/MatrixTransform>

#include <CommonCore/CommandSchedule.h>

namespace SceneHandlers
{
	class AnimateViewHandler : public osgGA::GUIEventHandler , public CommandSchedule
	{
	public:

		enum CAV_Actions{
			CAV_GET_ACTION = 0,
			CAV_SET_ACTION,
		};

		AnimateViewHandler(osgViewer::Viewer* viewer);
	        
		~AnimateViewHandler();

		void setLoadingOptions(osgDB::ReaderWriter::Options* opt) { _options = opt; }

		void setAnimationTime(double animtime) { _animationTime = animtime; }
	    
		bool handle(const osgGA::GUIEventAdapter& ea,osgGA::GUIActionAdapter& aa);

		//Ridefinizioni del Gestore dei Comandi
		virtual std::string handleAction(std::string action, std::string argument);
	    
	protected:

		std::string getViewMatrix();

		bool setViewMatrix( std::string viewstring );


		osg::Timer_t _startTime;
		osg::Matrix _transitionMatrix, _oldMatrix;
		double _animationTime;

		osg::ref_ptr<osgViewer::Viewer>  _mainViewer;

		osg::ref_ptr<osgDB::ReaderWriter::Options> _options;

		bool _activateTransition;
	};

};

#endif //__OSG4WEB_ANIMATEVIEWHANDLER__