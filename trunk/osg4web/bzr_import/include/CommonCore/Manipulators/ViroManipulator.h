/*
	ViRo Manipulator Header
	by Bruno Fanini ( http://www.brunofanini.tk )
========================================================================*/

#ifndef __VIROMANIP__
#define __VIROMANIP__

#include <osgGA/MatrixManipulator>
#include <osgViewer/Viewer>

//#include <CommonCore/Manipulators/BoostHotspot.h>
#include <CommonCore/Manipulators/Mixer.h>
#include <CommonCore/CommandSchedule.h>
#include <osg/Fog>

using namespace osgGA;

namespace Manipulators {

#define NOT_SOLID_MASK	0x5

class ViroManipulator : public MatrixManipulator, public CommandSchedule {

	public:
		
		ViroManipulator();

		const char* className() const { return "ViRoManipulator"; }

		/** Get the keyboard and mouse usage of this manipulator.*/
		void getUsage(osg::ApplicationUsage& usage) const;

		/** set the position of the matrix manipulator using a 4x4 Matrix.*/
		void setByMatrix(const osg::Matrixd& matrix);

		/** set the position of the matrix manipulator using a 4x4 Matrix.*/
		void setByInverseMatrix(const osg::Matrixd& matrix){ setByMatrix(osg::Matrixd::inverse(matrix)); }

		/** set the position of the matrix manipulator using Quat and Eye*/
		void setByOrientationEye(osg::Quat Q, osg::Vec3d E){
			_qRotation = Q;
			_vEye      = E;
			SyncData();
			};

		/** get the position of the manipulator as 4x4 Matrix.*/
		osg::Matrixd getMatrix() const;

		/** get the position of the manipulator as a inverse matrix of the manipulator, typically used as a model view matrix.*/
		osg::Matrixd getInverseMatrix() const;

		void home(const GUIEventAdapter& ea,GUIActionAdapter& us);
		//void computeHomePosition();
		void setHomeEye(Vec3d e){ _homeEye = e; };

		void requestFlyToHome();

		void init(const GUIEventAdapter& ea,GUIActionAdapter& us);
		// Event Listener
		bool handle(const GUIEventAdapter& ea,GUIActionAdapter& us);

		enum {
			STRAFE_LEFT,
			STRAFE_RIGHT,
			FORWARD,
			BACKWARD,
			UP,
			DOWN,
			BOOST
			};
		
		void boost(int mode = BOOST);
		void turn(double dx, double dy);

		// Sync some internal position/orientation data
		void SyncData( osg::Quat Q ){
			osg::Matrixd rMatrix;
			rMatrix.makeRotate( Q );
			osg::Vec3d vUp   = osg::Y_AXIS * rMatrix;
			osg::Vec3d vLook = -osg::Z_AXIS * rMatrix;
			osg::Vec3d vStrafe = vLook ^ vUp;
			vStrafe.normalize();
			vUp.normalize();
			//vLook.normalize();		// FIXME
			_vLook   = vLook;
			//_vTarget = _vEye + (_vLook * _minimumDistance);	// FIXME
			_vStrafe = vStrafe;
			_vUp     = vUp;
			}

		void SyncData(){
			SyncData( _qRotation );
			};

		void computePosition(const osg::Vec3d& eye,const osg::Vec3d& target,const osg::Vec3d& up);
		void computeHomePosition();
		osg::Quat computeQuat(const osg::Vec3d& eye,const osg::Vec3d& target,const osg::Vec3d& up);

		void storeView(){
			_vStoredEye[ViroManipulator::TO]      = _vEye;
			_qStoredRotation[ViroManipulator::TO] = _qRotation;
			};
		void storeView(osg::Vec3d eye, double p=0.0f){
			_vStoredEye[ViroManipulator::TO]      = _Mix.interpolate(p, eye,_vEye);
			_qStoredRotation[ViroManipulator::TO] = _qRotation;
			};
		void storeView(osg::Vec3d eye, osg::Quat rot){
			_vStoredEye[ViroManipulator::TO]      = eye;
			_qStoredRotation[ViroManipulator::TO] = rot;
			};

		void reloadView(){
			FlyTo( _lastFrameTime );
			};
		double getLastFrameTime(){ return _lastFrameTime; };

		osg::Vec3d VecBump(osg::Vec3d n, osg::Vec3d v);
		double VecAngle(osg::Vec3d a, osg::Vec3d b);

		virtual void setNode(osg::Node*);
		virtual const osg::Node* getNode() const { return _NODE.get(); };
		virtual osg::Node* getNode(){ return _NODE.get(); };

		//void scaleParameters( double scaleFactor );
		void scaleParameters( const BoundingSphere BB );

		bool Intersect(osg::Vec3d p1,osg::Vec3d p2,osg::Vec3d& vResult,osg::Vec3d& vNorm);
		bool Trace();
		void FlyTo(double callTime);

		// Given Viewer, add mouse Pick-Handler
		void addPickHandler(osgViewer::Viewer * v){ _Viewer = v; };
		//void setCamera(osg::Camera * c){ _Cam = c; };
		//osg::Camera* getCamera(){ return _Cam; };

		Vec3d getLookVec() const { return _vLook; };
		Vec3d getUpVec() const { return _vUp; };
		Vec3d getStrafeVec() const { return (_vLook ^ _vUp); };
		Vec3d getPosition() const { return _vEye; };
		double getSpeed(){ return _speed; }
		Vec3d getLastPickedPoint(){ return _vLastPickedPoint; }
		osgViewer::Viewer* getViewer(){ return _Viewer; };
		bool getHoldLock(){ return _bHoldCTRL; };
		bool getHardImpact(){ return _bImpact; };
		bool getSoftImpact(){ return _bAvoidanceZone; };
		//bool getZlock(){ return _bLockZ; };

		void tuneDistances(double mult){
			BumpDistance *= mult;
			AvoidanceDistance *= mult;
			};
		void tuneAcceleration(double mult){
			Acceleration *= mult;
			}
		void setImpactDistance(double d){
			BumpDistance = d;
			}
		void setGroundDistance(double d){
			GroundDistance = d;
			}
		void setAvoidanceDistance(double d){
			AvoidanceDistance = d;
			}
		bool getTumble(){ return _bTumble; };

		// Get Framing Time (UNUSED)
		void UpdateFrameTime(double now){
			_FrameDT = now - _lastFrameTime;
			_lastFrameTime = now;
			};
		// Get time interval between last 2 events
		long double getDtime(){
			long double dt = 0.0;
			if (_ev0.get()!=NULL && _ev1.get()!=NULL) dt = (_ev0->time() - _ev1->time());
			return dt;
			};

		void setGlassPrison(osg::BoundingBox BB){ _GlassPrison = BB; };
		void setEnvFog(osg::Fog* F){ _envFog = F; _envFogDensity = F->getDensity(); };

		std::string handleAction(std::string argument);
		std::string ExecCommand(std::string lcommand, std::string rcommand);	// 4 Luglio

		// ViRo Options, up to 32
		unsigned int ViroOptions;

		enum {
			// Mouse/Peripheral event locking ON/OFF
			PERIPHERAL_LOCK   = (1<<0),
			// Model collisions ON/OFF
			COLLISIONS        = (1<<1),
			// Collision Avoidance ON/OFF
			AVOIDANCE         = (1<<2),
			// Tracer MultiSampling ON/OFF
			MULTISAMPLING     = (1<<3),
			// SpiderMan Picking ON/OFF
			SPIDERMAN_PICKING = (1<<4),
			// Gravity ON/OFF
			GRAVITY           = (1<<5),
			// Surface Adaption ON/OFF
			SURFACE_ADAPTION  = (1<<6),
			// Hold Target ON/OFF
			HOLD_TARGET       = (1<<7),
			// Hotspot Reactor ON/OFF
			HOTSPOT_REACTION  = (1<<8),
			// Check always for vertical collisions 
			Z_COLLIDE         = (1<<9),
			// Cannot escape from model bounds
			GLASS_PRISON      = (1<<10),
			// Auto-Scale steps
			AUTO_SCALE_STEP   = (1<<11),
			// Height Lock
			Z_LOCK            = (1<<12)
			};

		void ClearOptions(){ ViroOptions = 0; };	
		void Enable(unsigned short B){ ViroOptions |= B; };
		void Disable(unsigned short B){ ViroOptions &= ~B; };
		bool isEnabled(unsigned short B){ return bool(ViroOptions & B); };
		void Invert(unsigned short B){ ViroOptions ^= B; };

		double	Acceleration;			// Acceleration
		double	BumpDistance;			// Surface Hard-Collision distance
		double  GroundDistance;
		double	AvoidanceDistance;		// Surface Distance to apply Avoidance-Correction
		double	RollAutoCorrection;		// Auto-Correction factor
		double	RollWithYaw;			// Roll with Yaw factor
		double	SurfaceAdhesion;		// Surface Adhesion factor
		double	SurfaceSpeedLimiter;	// The Max Speed on Surface
		double	FlyToDurationTime;		// Fly-To Duration Time (in seconds). must be > 0
		int NumTracerSamplers;			// Num. of Samplers (Note: This influences performance)
		osg::Vec2d TracerFrustum;		// Size & shape of ray-shooter (in MultiSampling)
		double GravityAcceleration;
		double SurfaceAdaption;			// % of surface adaption, when Gravity and SurfaceAdaption ON
		double PeripheralSensibility;	// Mouse/Other peripherals sensibility.
		double autoStepFactor;			// Merc. 10/09/08

		//osg::ref_ptr<HotspotList> Hotspots;		// Dec 27 - Hotspot list
		//int LastPickedHotspot;					// Jan 19

		bool bAutoControlLock;

	protected:
		enum { FROM = 0, TO = 1 };

		~ViroManipulator();

		osg::ref_ptr<osg::Node> _NODE;	// Root node of Manipulator
		osg::ref_ptr<osg::Fog> _envFog;
		float _envFogDensity;
		
		/** The main function for auto-correction, collision detection & avoidance.*/
		void AutoControl(double callTime);

		/** Flush mouse stack.*/
		void flushMouseEventStack();
		/** Add the current mouse GUIEvent to internal stack.*/
		void addMouseEvent(const GUIEventAdapter& ea);
		/* true if mouse-speed > a threshold */
		bool isMouseMoving();
		bool handlePick(double dx,double dy,float A=0.4);
		bool handleRelease(const GUIEventAdapter& ea);
		void handleFlyToTransition(double calltime);

		/** Mouse Listener */
		bool MouseListener();

		// Used to pick the right normal according to eye position
		osg::Vec3d RightNormal(osg::Vec3d p, osg::Vec3d n);

		// Instance of Viewer for Picking
		osgViewer::Viewer* _Viewer;

		// Instance of Camera for Picking
		//osg::Camera* _Cam;

		// Last 2 Mouse-Events stack
		osg::ref_ptr<const GUIEventAdapter> _ev0;
		osg::ref_ptr<const GUIEventAdapter> _ev1;

		osg::Vec3d	_vEye;					// eye
		osg::Vec3d	_vTarget;				// target
		osg::Vec3d	_vUp;					// up
		osg::Vec3d	_vLook;					// Look Vector
		osg::Vec3d	_vStrafe;				// Strafe Vector

		osg::Vec3d	_vStoredEye[2];			// From/To Stored eye
		osg::Quat	_qStoredRotation[2];	// From/To Stored rotation

		osg::Quat	_qRotation;				// Camera rotation

		double		_yaw;					//
		double		_pitch;					//
		double		_roll;					//

		double		_FrameDT;
		double		_lastFrameTime;
		double		_tLastAvoidanceWarn;
		double		_tLastLMB;
		double		_tLastImpact;
		double		_tFallTimer;

		double		_speed;							// Current Speed
		double		_AvoidanceReaction;				// Collision-Avoidance reaction factor
		double		_UserControlPercentage;			// User Control (0 = None, 1 = Full)
		double		_CurrentGravityAcceleration;	// UNUSED
		double		_currentGroundDistance;
		double		_zLock;

		double		_modelScale;
		osg::Vec3d	_vIpoint;				// Current Intersection Point (or avg)
		osg::Vec3d	_vInormal;				// Current Intersection Normal (or avg)

		osg::Vec3d _holdTarget;
		osg::Vec3d _vLastPickedPoint;
		osg::Vec3d _vLockDir;

		// Bool
		bool _bNeedUpdateTrace;				// Tracer needs to be called
		bool _bIntersect;					// True if Tracer found intersection...
		bool _bFlying;						// Performing a Fly-To
		bool _bSatMode;						// Satellite mode
		bool _bHoldCTRL;
		bool _bSurfaceImpact;				// Impact with surface (when GRAVITY = ON)
		bool _bImpact;
		bool _bAvoidanceZone;
		bool _bReqHome;
		//bool _bLockZ;
		bool _bTumble;
		bool _bMidButton;
		bool _bTurning;
		bool _bDirtyScale;

		// Web NavPad
		unsigned short _padEvent;

		enum {
			NAVPAD_NONE = 0,
			NAVPAD_STRAFELEFT,
			NAVPAD_STRAFERIGHT,
			NAVPAD_UP,
			NAVPAD_DOWN,
			NAVPAD_TURNLEFT,
			NAVPAD_TURNRIGHT,
			NAVPAD_FORWARD,
			NAVPAD_BACKWARD,
			NAVPAD_PITCHUP,
			NAVPAD_PITCHDOWN,
			};

		double _FlyToCallTime;

		// Scaling-Sensible safe parameters
		double _safeAccel;
		double _safeBD;
		double _safeAD;

		BoundingBox _GlassPrison;

		//osg::ref_ptr<Mixer>		_Mix;		// Mixer Module
		Mixer	_Mix;							// Mixer Module
		//osg::ref_ptr<osg::LightSource> _HeadLight;
	};

};

#endif	// ViroManipulator