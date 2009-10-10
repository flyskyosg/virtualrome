/*=========================================================================
Program:   Multimod Application Framework
Module:    $RCSfile: walkManipulator.h,v $
Language:  C++
Date:      $Date: 2007/12/18 19:09:58 $
Version:   $Revision: 1.4 $
Authors:   Tiziano Diamanti
==========================================================================
Copyright (c) 2001/2005 
CINECA - Interuniversity Consortium (www.cineca.it)
=========================================================================*/

#ifndef __walkManipulator__
#define __walkManipulator__ 1


#include <CommonCore/CommandSchedule.h>
#include <CommonCore/Manipulators/CameraPoints.h>

#define		_USE_VISMAN_	0 //Disable visman API:w

#define NUM_LIGHTS 5

#include <osgGA/MatrixManipulator>
#include <osg/Quat>
#include <osg/AnimationPath>
#include <osg/MatrixTransform>
#include <osgText/Text>
#include <osg/ShapeDrawable>
#include <osgViewer/Viewer>


#if _USE_VISMAN_
  #include "vismanPicker.h"
  #include "vismanMain.h"
  class vismanMain;
#endif

using namespace osgGA;

namespace Manipulators
{

	const static double _walk_smoothing_factor = 5.0;
	const static double _angle_step = 0.5;

	//---------------------------------------------------
	class walkManipulator : public MatrixManipulator, public CommandSchedule
	//---------------------------------------------------
	{
	  public:

	#if _USE_VISMAN_
	    void assignVisman(vismanMain *vs);
	    inline vismanPicker *getPicker(void) { return _picker; };
	#endif

		walkManipulator();
		virtual const char* className() const { return "walkManipulator"; }

		// chiamati da fuori
		virtual void setByMatrix(const osg::Matrixd& matrix);
		virtual void setByInverseMatrix(const osg::Matrixd& matrix) { setByMatrix(osg::Matrixd::inverse(matrix)); }
		virtual osg::Matrixd getMatrix() const;
		virtual osg::Matrixd getInverseMatrix() const;

		// funzioni virtuali della classe base
		// nodo usato per impostare la HomePosition ed anche per fare l'Intersect
		void setNode(osg::Node* node); 
		osg::Node* getNode() {return _node.get();}; 

		virtual void searchDefaultPos();
		virtual void searchPosUsingSubNode(osg::Node* subnode, double ydistfromcenter = 3.0);
		virtual bool handle(const GUIEventAdapter& ea,GUIActionAdapter& us);

		void Render(void);

		void setPosition(CameraData Point, bool default_pos);
		void getPosition(CameraData *Point);
		void rotateHead(double yaw, double pitch);
		void setStepAmount(double step_h, double step_v);
		void setGroundDistance(double gd);
    void setNearFar(double Near, double Far);
		void setGroundCollisionOnOff(bool on);
		void setViewer(osgViewer::Viewer *viewer, std::string db_prefix, std::string exp_prefix);
		void flytoScenePick(float mouse_x, float mouse_y);
		osg::Vec3d getPosition(void);
		void getYawnPitch(double *yaw, double *pitch);
		void getSteps(double *step_h, double *step_v);
		void go_left(bool keep_on);
		void go_right(bool keep_on);
		void go_upward(bool keep_on);
		void go_backward(bool keep_on);
		void go_lift(bool keep_on);
		void go_lower(bool keep_on);
		void stop_walking(void);
		void rotate_left(bool keep_on);
		void rotate_right(bool keep_on);
		void rotate_up(bool keep_on);
		void rotate_down(bool keep_on);
		void stop_rotating(void);
		void adjustGroundDistance(void);
		void loopCameraPoints(bool *go_on, CameraPoints *CP);
		void moveLights(osg::Light **Sun, osg::LightSource **Sun_ls, float *scene_radius);
		unsigned int get_context_id(void);

		std::string ExecCommand(std::string lcommand, std::string rcommand);

		//Ridefinizioni del Gestore dei Comandi
		virtual std::string handleAction(std::string argument);

		void setIntersectSegmenteMultiplier(double one, double two) { _segmult1 = one; _segmult2 = two; };

	  protected:
		virtual ~walkManipulator();
		bool calcMovement();

		void IntersectTerrain(void);
		bool Intersect( osg::Vec3d p1, osg::Vec3d p2, osg::Vec3d& result);
		void moveHorizzontally(double amount, bool move_forward, bool move_left);
		void moveVertically(double amount);
		osg::Vec3d calcNextPosition(double step_x, double dist_y, double dist_z); 
		double calcGroundDistance(void);
		void handleWalk(void);
		void handleRotation(void);
		void accelerateHor(double value);
		void accelerateVer(double value);
		double calcPointsDistance(osg::Vec3d p1, osg::Vec3d p2);
		double calcNextAngle(double StartingAngle, double *phi, double step);

		osg::ref_ptr<osg::Node> _node;
		osgViewer::Viewer *_viewer;
	#if _USE_VISMAN_
		vismanPicker *_picker;
		vismanMain *_visman;
	#endif
		osg::Vec3d   _position; // posizione camera
		double       _yaw;      // angolo di rotazione della camera intorno a z [0..360]
		double       _pitch;    // angolo di rotazione della camera intorno a x [0..90]
		double       _roll;     // angolo di rotazione della camera intorno a y
		double       _ground_distance;   // altezza dal terreno

		double       _step_h;  // lunghezza di un passo orizzontale
		double       _step_v;  // lunghezza di un passo verticale
		double       _rstep;   // passo di rotazione

		double       _x0,_y0;  // previous mouse position
		osg::Vec3d   _initial_position;
		osg::Vec3d   _initial_angles;
		bool         _default_pos_set;
		double       _H_Speed, _V_Speed;
		int          _going_left, _going_up;
		bool         _calc_ground_distance;
		bool         _keep_on_walking;
		bool         _keep_on_rotating_left, _keep_on_rotating_right, _keep_on_rotating_up, _keep_on_rotating_down;
		bool         _ground_collision_on;   

		double		_segmult1;
		double		_segmult2;
	};

};

#endif



