#ifndef __OSG4WEB_ANIMATEVIEWHANDLER__
#define __OSG4WEB_ANIMATEVIEWHANDLER__ 1

#include <sstream>
#include <queue>

#include <osgViewer/Viewer>
#include <osg/MatrixTransform>

#include <CommonCore/CommandSchedule.h>

namespace SceneHandlers
{

#define		DEFAULT_ANIMATION_TIME			2500.0

	class AnimateViewHandler : public osgGA::GUIEventHandler , public CommandSchedule
	{
	public:

		enum AnimateViewActions{
			UNKNOWN_ACTION = 0,
			GET_CURRENT_MATRIX,
			GO_TO_MATRIX_DIRECTLY,
			GET_ANIMATION_TIME,
			SET_ANIMATION_TIME,
			SET_ANIMATION_KEY,
			START_ANIMATION,
			CONTINUE_ANIMATION,
			STOP_ANIMATION,
			RESET_DEFAULT
		};

		AnimateViewHandler(osgViewer::Viewer* viewer);
	        
		~AnimateViewHandler();

		void setLoadingOptions(osgDB::ReaderWriter::Options* opt) { _options = opt; }

		void setAnimationTime(double animtime) { _animationTime = animtime; }

		double getAnimationTime() { return _animationTime; }

		void setAnimationPathSmoothness(double s){ _animationPathSmoothness = s; }
		double getAnimationPathSmoothness(){ return _animationPathSmoothness; }

		bool handle(const osgGA::GUIEventAdapter& ea,osgGA::GUIActionAdapter& aa);

		// Catmull-Rom Interpolation (p1,p2)
		osg::Vec3 interpolateCR(float t, osg::Vec3 p0, osg::Vec3 p1, osg::Vec3 p2, osg::Vec3 p3);

		//Ridefinizioni del Gestore dei Comandi
		virtual std::string handleAction(std::string argument);
	    
	protected:

		std::string getViewMatrix();
		bool setViewMatrixAndGoTo( std::string viewstring );

		bool setAnimationKey( std::string key );

		std::string getViewAnimationTime();
		bool setViewAnimationTime(std::string animtime);

		bool startAnimation();
		bool continueAnimation();
		bool stopAnimation();

		void resetSettings();

		bool doTransition(osg::Matrix animMatrix, double animTime);

		osg::ref_ptr<osgViewer::Viewer>  _mainViewer;
		osg::ref_ptr<osgDB::ReaderWriter::Options> _options;

		std::queue<osg::Matrix> _animationMatrixArray;
		std::vector<osg::Matrix> _animationMatrixVector;
		std::queue<double> _animationTimeArray;
		std::vector<double> _animationTimeVector;
		int _currentKey;

		osg::Timer_t _startTime;
		osg::Matrix _transitionMatrix, _oldMatrix;
		double _animationTime;

		osg::Matrix _sequenceTransitionMatrix;
		double _sequenceAnimationTime;
		osg::Matrix _prevTensor, _nextTensor;

		bool _activateTransition;
		bool _duringAnimation;
		double _animationPathSmoothness;		// Piu tende a 0, piu il percorso di animazione tende ad una spezzata. (default = 1.0)
	};

};

#endif //__OSG4WEB_ANIMATEVIEWHANDLER__