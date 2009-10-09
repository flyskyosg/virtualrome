/*
	ViRo HUD Header
	by Bruno Fanini ( http://www.brunofanini.tk )
========================================================================*/

#ifndef __VIROHUD__
#define __VIROHUD__

#include <osg/io_utils>
#include <sstream>

#include <CommonCore/Manipulators/ViroManipulator.h>
//#include <CommonCore/Manipulators/Mixer.h>

#include <osgDB/ReadFile>
#include <osgGA/MatrixManipulator>
#include <osgViewer/Viewer>
#include <osg/Referenced>
#include <osg/MatrixTransform>
#include <osg/Geometry>
#include <osg/PositionAttitudeTransform>
#include <osgText/Font>
#include <osgText/Text>
#include <osgText/String>
#include <osg/ShapeDrawable>
#include <osg/AlphaFunc>
#include <osg/BlendFunc>
#include <osg/TexGenNode>
#include <osg/Texture>

#include <osg/NodeCallback>

#define VIROHUD_FONT				"arial.ttf"
#define VIROHUD_SIDECOLOR			0.94,0.91,0.78,0.6
#define VIROHUD_HUDCOLOR			0.94,0.91,0.78,0.2	//1.0,1.0,0.0,0.4
#define VIROHUD_COMPASSNORTHCOLOR	0.94,0.91,0.78,1.0
#define VIROHUD_COMPASSNODE			"axes.osg"	//"compass.ive"

using namespace osg;
using namespace Manipulators;

//////////////////////////////////////////////////////////////////////////

class ViroHud : public Referenced {

	public:
		ViroHud();
		ViroHud( osgViewer::Viewer* v );	// using a generic OSG Viewer
		ViroHud( ViroManipulator* vm );		// using ViRo manipulator

		const char* className() const { return "ViRoHUD"; }

		void setViroManipulator( ViroManipulator* vm ){
			_vm     = vm;
			_viewer = vm->getViewer();
			};

		osg::Projection* getHUD(){ return _HUD.get(); };
		void Update();
		void Realize();

		void CreateCompass(float px,float py, float size);
		void CreateLoadingBar();
		void UpdateLoadingBar();

		void setServerPrefix( std::string s){ _serverPrefix = s; };
		std::string getServerPrefix(){ return _serverPrefix; };

		void setViewportSize(int w, int h, float percent=1.0f){
			if (w>0 && h>0){
				_vp_width = w * percent;
				_vp_height = h * percent;
				}
			};

		void setCurrentLoad(unsigned int l){ _currentLoad = l; };
		unsigned int getCurrentLoad(){ return _currentLoad; };

	private:
		osg::ref_ptr<Manipulators::ViroManipulator> _vm;			// using ViRo manipulator
		osg::ref_ptr<osgViewer::Viewer> _viewer;					// using a generic OSG Viewer
		osg::ref_ptr<osg::Projection> _HUD;							// The HUD node
		osg::ref_ptr<osgText::Text> _HUDlabel;
		osg::ref_ptr<osg::PositionAttitudeTransform> _CompassPAT;	// Compass transform-node
		osg::ref_ptr<osg::Geode> _Compass;							// the Compass
		osg::ref_ptr<osg::Geometry> _LoadingBarGeom;

		double _vp_height, _vp_width;
		std::ostringstream _HUDstring;
		//char _HUDstring[512];	// Piu' performante
		std::string _serverPrefix;
		unsigned int _currentLoad;
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////

class ViroHudUpdater : public osg::NodeCallback {
	public:
		ViroHudUpdater( ViroHud* vhud){ _vhud = vhud; };

		virtual void operator()(osg::Node* node, osg::NodeVisitor* nv){
			_vhud->Update();

			// must traverse the Node's subgraph            
            traverse(node,nv);
			};

	private:
		osg::ref_ptr<ViroHud> _vhud;
};

#endif	// ViroHUD