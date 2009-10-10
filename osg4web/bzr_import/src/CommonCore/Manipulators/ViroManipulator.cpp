/*
	ViRo Manipulator
	by Bruno Fanini ( http://www.brunofanini.tk )

========================================================================*/

// Header
#include <CommonCore/Manipulators/ViroManipulator.h>
#include <CommonCore/Manipulators/Mixer.h>

#include <osg/Notify>
#include <osg/Matrixd>
#include <osg/Transform>
#include <osg/CullStack>
#include <osg/io_utils>
#include <osgUtil/IntersectVisitor>
#include <osgViewer/Viewer>

using namespace osg;
using namespace osgGA;

using namespace Manipulators;

//////////////////////////////////////////////////////////////////////////

ViroManipulator::ViroManipulator() : CommandSchedule("WALK") {
	ClearOptions();	// Initialize Options
	
	// Enable defaults
	//Enable(MULTISAMPLING | GRAVITY | SURFACE_ADAPTION);

	Acceleration = _safeAccel = 1000.0;

	RollWithYaw         = 1.2;	// Roll with Yaw factor (0.0 to disable)
	RollAutoCorrection  = 0.1;	// The speed of Roll-Correction

	//BumpDistance      = _safeBD = 5.0;  // Distance from surface to consider hard collision
	//AvoidanceDistance = _safeAD = 100.0; // Distance from surface to begin avoidance correction

	SurfaceSpeedLimiter = Acceleration;	// The Speed-Limiter on surface

	NumTracerSamplers        = 15;		// Num. of Tracer Samplers.
	TracerFrustum.set( 0.1, 0.05 );		// A default for Size & Shape of Ray-Frustum

	GravityAcceleration      = 6.0;
	SurfaceAdhesion	         = 0.999;	// % of speed conservation when it impacts
	SurfaceAdaption          = 0.4;		// % of surface adaption

	FlyToDurationTime      = 2.0;		// Duration of Fly-To, in seconds
	
	PeripheralSensibility    = 50.0;	// Mouse sensibility

	//Hotspots = new HotspotList;
	//LastPickedHotspot = -1;

	//////////////////////////////////////////////////////////////////////////////////
	_pitch    = 0.0;
	_yaw      = 0.0;

	_speed					 = 0.0;

	_bNeedUpdateTrace        = true;
	_bIntersect              = false;
	_UserControlPercentage   = 1.0;
	_bFlying                 = false;
	_bReqHome                = false;
	_bSatMode                = false;
	_bHoldCTRL               = false;
	bAutoControlLock         = false;
	_AvoidanceReaction       = 1e-5;
	_modelScale				 = 0.01;
	_tLastAvoidanceWarn      = -1.0;
	_tLastLMB                = -1.0;
	_tLastImpact             = -1.0;
	_tFallTimer              = 0.0;

	_CurrentGravityAcceleration = GravityAcceleration;
	_bSurfaceImpact = false;

	//_Mix = new Mixer;
	setMinimumDistance( 20.0f );
	_GlassPrison.init();

	Enable(PERIPHERAL_LOCK);
	//_HeadLight = new osg::LightSource;

	_padEvent = NAVPAD_NONE;
	_bLockDir = false;
}

ViroManipulator::~ViroManipulator(){
}

void ViroManipulator::flushMouseEventStack(){
	_ev0 = NULL;
	_ev1 = NULL;
}

void ViroManipulator::addMouseEvent(const GUIEventAdapter& ea){
	_ev1 = _ev0;
	_ev0 = &ea;
}

// UNUSED
bool ViroManipulator::isMouseMoving(){
	if (_ev0.get()==NULL || _ev1.get()==NULL) return false;

	static const double velocity = 0.1f;
	osg::Vec2d d;

	d.set(( _ev0->getXnormalized()-_ev1->getXnormalized()),( _ev0->getYnormalized()-_ev1->getYnormalized() ));

	double len = d.length();
	double dt = _ev0->time()-_ev1->time();

	return (len > (dt*velocity) );
}

// Handle picking
bool ViroManipulator::handlePick(double dx,double dy, float A){
	// Our picked point initialized to INF
	osg::Vec3d pickedPoint(DBL_MAX, DBL_MAX, DBL_MAX );
	osg::Vec3d pickedNormal;

	bool found = false;
	
	// Viewer is not set
	if (!_Viewer) return false;
	/*	
	{
		if (!_Cam) return false;

		osg::notify(ALWAYS)<<"Cam is not set. I cannot handle pick.\n";

		osgUtil::PickVisitor pv(_Cam->getViewport(),_Cam->getProjectionMatrix(),_Cam->getViewMatrix(),dx,dy);
		_NODE->accept( pv );
		
		if (!pv.hits()) return false;

		//osgUtil::PickVisitor::LineSegmentHitListMap::iterator k = ;	static_cast<osg::LineSegment*>
		const osg::LineSegment* L = pv.getSegHitList().begin()->first;

		osgUtil::IntersectVisitor::HitList pickList = pv.getHitList(L);
		
		for(osgUtil::PickVisitor::HitList::iterator i=pickList.begin(); i!=pickList.end(); ++i){
			if( i->_geode.valid() && (i->_geode.get()->getName() != "NOPICK") ){
				osg::Vec3d pp = i->getWorldIntersectPoint();
				osg::Vec3d pn = i->getWorldIntersectNormal();

				if (Vec3d(_vEye - pp).length() < Vec3d(_vEye - pickedPoint).length()){
					pickedPoint  = pp;
					pickedNormal = pn;
					found = true;
					}
				}
			}
		}
	*/

	// we have a Viewer
	else {
		//return false;
		//osgUtil::IntersectVisitor::HitList pickList;
		osgUtil::LineSegmentIntersector::Intersections pickList;

		if (_Viewer->computeIntersections(dx,dy, pickList)){
			for(osgUtil::LineSegmentIntersector::Intersections::iterator i=pickList.begin();i!=pickList.end();++i){
				if( i->drawable.valid() && (i->drawable.get()->getName()!="Contraint_OutBox_Geode") &&
											(i->drawable.get()->getName()!="environment")
					/*&& ( i->drawable.get()->getParent(0)->getNodeMask() != NOT_SOLID_MASK )
					(i->drawable.get()->getName() != "NOPICK") i->_geode.valid() && (i->_geode.get()->getName() != "NOPICK")*/ ){
					
					osg::Vec3d pp = i->getWorldIntersectPoint();
					osg::Vec3d pn = i->getWorldIntersectNormal();

					if (Vec3d(_vEye - pp).length() < Vec3d(_vEye - pickedPoint).length()){
						pickedPoint  = pp;
						pickedNormal = pn;
						found = true;
						}
					}
				}
			}
		}

	// We found (h)it !!!
	if ( found ){
			#define MAX_PICK_DISTANCE_MULT	4.0
			double D = max(BumpDistance,GroundDistance);
			if ( Vec3d(_vEye - pickedPoint).length() < (D*MAX_PICK_DISTANCE_MULT)) return false;	// too near
			if ( isEnabled(GLASS_PRISON) && !_GlassPrison.contains( pickedPoint ) ) return false;	// OUT

			/*
			if ( isEnabled(HOTSPOT_REACTION) ){
				if ( isEnabled(GLASS_PRISON) && !_GlassPrison.contains(pickedPoint) ) return false;

				LastPickedHotspot = Hotspots.get()->checkHotspotCapture( pickedPoint );
				if (LastPickedHotspot>0 && !Hotspots.get()->getHotspot(LastPickedHotspot)->isSensible() ) return false;

				LastPickedHotspot = Hotspots.get()->nearestHotspot( pickedPoint );
				// we dont want pick repulsive hotspots
				if (LastPickedHotspot>0 && (Hotspots.get()->getHotspot(LastPickedHotspot)->getFeedback() > 0.0f || Hotspots.get()->getHotspot(LastPickedHotspot)->getGravityFeedback() < 0.0f )) return false;
				}
			*/

			//osg::notify(ALWAYS)<<"Picked Point = "<< pickedPoint.x()<<","<< pickedPoint.y() <<","<< pickedPoint.z()<<std::endl;
			_vLastPickedPoint = pickedPoint;

			pickedNormal = RightNormal(pickedPoint,pickedNormal);
			double d = Vec3d(_vEye - pickedPoint).length() * 0.05;
			
			// Approaching Factor [0,1]
			double approach = A;
			//if (_bSatMode) approach = 0.0;	// SatMode : exactly perpendicular to surface
			//else           approach = 0.4;	// As you like...
			
			// Store Destination i.e the final observation point.
			_vStoredEye[TO] = _Mix.interpolate(approach,(pickedPoint+(pickedNormal*d)),_vEye);

			if (isEnabled(SPIDERMAN_PICKING))
				_qStoredRotation[TO] = computeQuat(_vStoredEye[TO],pickedPoint,(pickedNormal*d));
			else
				_qStoredRotation[TO] = computeQuat(_vStoredEye[TO],pickedPoint,osg::Z_AXIS);//_vUp);

			}
	return found;
}

// Given (eye,target,up) compute Quaternion.
osg::Quat ViroManipulator::computeQuat(const osg::Vec3d& eye,const osg::Vec3d& target,const osg::Vec3d& up){
	osg::Quat Q;
	osg::Vec3d vLook = target - eye;

	osg::Vec3d f(vLook);
	f.normalize();
	osg::Vec3d s(f^up);
	s.normalize();
	osg::Vec3d u(s^f);
	u.normalize();

	osg::Matrixd rMatrix(s[0],     u[0],     -f[0],     0.0f,
						s[1],     u[1],     -f[1],     0.0f,
						s[2],     u[2],     -f[2],     0.0f,
						0.0f,     0.0f,     0.0f,      1.0f);

	rMatrix.get( Q );
	
	return ( Q.inverse() );
}

// Given (eye,target,up) compute position.
void ViroManipulator::computePosition(const osg::Vec3d& eye,const osg::Vec3d& target,const osg::Vec3d& up){
	_vEye    = eye;
	_vTarget = target;
	_vUp     = up;
	_vLook   = target - eye;
	_vStrafe = _vLook ^ _vUp;
	_qRotation = computeQuat(eye,target,up);
}

void ViroManipulator::computeHomePosition(){
	if (_NODE.get()){
		const osg::BoundingSphere& BS =_NODE->getBound();
		double r = BS._radius * 0.60; //1.25;
		
		_homeCenter = BS._center;
		_homeEye    = BS._center + ( Vec3d(cos(osg::PI_4),0.0,sin(osg::PI_4 * 0.05))* r );
		_homeUp     = osg::Z_AXIS;

		osg::notify(ALWAYS)<<"Home Computed.\n";
	}
}

osg::Matrixd ViroManipulator::getMatrix() const {
	//return osg::Matrixd::rotate(_qRotation) * osg::Matrixd::translate(_vEye);

	double c1 = cos(_yaw * 0.5);
	double s1 = sin(_yaw * 0.5);
	double c2 = cos(_pitch * 0.5);
	double s2 = sin(_pitch * 0.5);
	double c3 = cos(_roll * 0.5);
	double s3 = sin(_roll * 0.5);
    double c1c2 = c1*c2;    
	double s1s2 = s1*s2;    
	
	osg::Quat rot( c1c2*s3 + s1s2*c3, s1*c2*c3 + c1*s2*s3, c1*s2*c3 - s1*c2*s3, c1c2*c3 - s1s2*s3);
	return ( osg::Matrixd::rotate(rot) * osg::Matrix::translate(_vEye) );
}

osg::Matrixd ViroManipulator::getInverseMatrix() const {
	return osg::Matrixd::translate(-_vEye) * osg::Matrixd::rotate(_qRotation.inverse());
}

void ViroManipulator::setByMatrix(const osg::Matrixd& matrix){
	this->Disable(GRAVITY);
	_speed    = 0.0;
	_padEvent = NAVPAD_NONE;

	_vEye = matrix.getTrans();
	osg::Quat q = matrix.getRotate();

	double qx,qy,qz,qw;

	qx = q.x();
	qy = q.y();
	qz = q.z();
	qw = q.w();

	_yaw   = atan2( 2*qy*qw-2*qx*qz , 1 - 2*qy*qy - 2*qz*qz);
	_pitch = asin(2*qx*qy + 2*qz*qw);
	_roll  = atan2(2*qx*qw-2*qy*qz , 1 - 2*qx*qz - 2*qz*qz);

	osg::Quat rot = osg::Quat( (_pitch+osg::PI_2), osg::X_AXIS) * osg::Quat(0.0/*(_roll) */, osg::Y_AXIS) * osg::Quat( (_yaw), osg::Z_AXIS);
	_qRotation    = rot;

	//matrix.decompose(_vEye, Q, vScale, q);

	//osg::Matrixd M,Ms;
	//M = matrix;
	//M.makeScale(-vScale);
	
	//_qRotation = M.getRotate();

	/*
	osg::Vec3d vScale,E,T,U;
	osg::Quat Q;
	matrix.getLookAt(E, T, U );
	matrix.decompose(_vEye, _qRotation, vScale, Q);

	_vEye = matrix.getTrans();
	//osg::Vec3d vLook = -osg::Z_AXIS * matrix;
	//vLook = matrix.getScale() ^ (-vLook);
	//_vTarget = vLook + _vEye;


	//_vUp  = osg::Z_AXIS;
	//matrix.getScale();
	//matrix.get( _qRotation );
	//matrix.getLookAt( _vEye, _vTarget, _vUp );
	//computePosition( _vEye, _vTarget, osg::Z_AXIS );
	//_qRotation.set( matrix );
	//SyncData( matrix.getRotate() );

	//SyncData(_qRotation);
	computePosition( _vEye, T, U );
	SyncData();
	//computePosition( matrix.getTrans(), (matrix.getTrans()+matrix.get), osg::Z_AXIS );
	*/
	SyncData();
}

/*
void ViroManipulator::scaleParameters( double scaleFactor ){
	BumpDistance        = _safeBD * scaleFactor;
	AvoidanceDistance   = _safeAD * scaleFactor;
	Acceleration        = _safeAccel * scaleFactor;
	SurfaceSpeedLimiter = Acceleration * 0.1;
	//_AvoidanceReaction = scaleFactor * 5e-8;

	osg::notify(ALWAYS)<<"Acceleration = " << Acceleration <<std::endl;
	osg::notify(ALWAYS)<<"Bump-Distance = " << BumpDistance <<std::endl;
	osg::notify(ALWAYS)<<"Avoidance-Distance = " << AvoidanceDistance <<std::endl;
}
*/

void ViroManipulator::scaleParameters( const BoundingSphere BB ){
	if ( !BB.valid() ) return;

	double scaleFactor = BB.radius() * 0.001;
	printf("Model Scale Factor: %.4f\n", scaleFactor);

	//BumpDistance        = _safeBD * scaleFactor;
	//AvoidanceDistance   = _safeAD * scaleFactor;
	Acceleration        = _safeAccel * scaleFactor;
	SurfaceSpeedLimiter = Acceleration * 0.1;
	//_AvoidanceReaction = scaleFactor * 5e-8;

	osg::notify(ALWAYS)<<"Acceleration = " << Acceleration <<std::endl;
	osg::notify(ALWAYS)<<"Bump-Distance = " << BumpDistance <<std::endl;
	osg::notify(ALWAYS)<<"Avoidance-Distance = " << AvoidanceDistance <<std::endl;
}

// Sets the main node.
// This also (re)scales some parameters
void ViroManipulator::setNode(osg::Node* node){
	_NODE = node;
	if (_NODE.get()){
		scaleParameters( _NODE->getBound() );
		_GlassPrison.expandBy( _NODE->getBound() );
		//_NODE.get()->asGroup()->addChild( _HeadLight.get() );
		}

	if (getAutoComputeHomePosition()) computeHomePosition();    
}

// Boost
void ViroManipulator::boost(int mode){
	Vec3d vD;
	long double dt = getDtime();
	long double s = _speed;
	long double F = (s * dt);
	if (isEnabled(AUTO_SCALE_STEP)) F *= autoStepFactor;

	switch (mode){
	
		case(BOOST):{
			if (_speed != 0.0){
				if (!_bLockDir) vD = _vLook * F;
				else vD = _vLockDir * F;

				_vEye    += vD;
				_vTarget += vD;		// FIXME?

				if (_speed < 0.0) _UserControlPercentage = 1.0;
				return;
				}
			}

		case (UP):{
			vD = osg::Z_AXIS * F;

			_vEye    += vD;
			_vTarget += vD;		// FIXME?
			
			if (_speed < 0.0) _UserControlPercentage = 1.0;
			return;
			}
	}
}

// Turn
void ViroManipulator::turn(double dx, double dy){
	double dt     = getDtime();

	if (!_bHoldCTRL){
		double dPitch = -inDegrees(dy * PeripheralSensibility * dt);
		double dYaw   =  inDegrees(dx * PeripheralSensibility * dt);
		double dRoll  = dYaw * RollWithYaw;

		// Update global pitch & yaw. UNUSED
		_pitch += dPitch;
		_yaw   += dYaw;

		osg::Quat dRotation;	// Delta Rotation
		osg::Quat pitch_rotate;
		osg::Quat yaw_rotate;

		pitch_rotate.makeRotate(dPitch, _vStrafe);
		yaw_rotate.makeRotate(dYaw, _vUp);

		osg::Quat roll_rotate;
		if (RollWithYaw != 0.0){
			roll_rotate.makeRotate(-dRoll, _vLook);
			dRotation = pitch_rotate * roll_rotate * yaw_rotate;
			}
		else dRotation = pitch_rotate * yaw_rotate;
	
		// Apply deltaRotation and store it.
		_qRotation = _qRotation * dRotation;
		}

	else {
		long double Fy = (dt * 200 * dy);
		long double Fx = (dt * 200 * dx);
		if (isEnabled(AUTO_SCALE_STEP)){ Fy *= autoStepFactor; Fx *= autoStepFactor; }

		//osg::Quat dRotation;
		//dRotation.makeRotate(-(Fx * (_vEye-_vLastPickedPoint).length() * 0.1 ), osg::Z_AXIS);

		Vec3d M,Slide;
		Slide = _vStrafe;
		Slide.normalize();
		Slide *= -Fx;

		//dRotation.makeRotate( (_vEye-_vLastPickedPoint),(_vEye-_vLastPickedPoint)+Slide+Vec3d(0,0,-Fy));

		M = Vec3d(Slide.x(),Slide.y(),-Fy);
		//M = Vec3d(0,0,-Fy);

		_vEye    += M;
		_vTarget += M;

		//_qRotation = _qRotation * dRotation;
		}
}

void ViroManipulator::FlyTo(double callTime){
	if ( !_bFlying ){
		_FlyToCallTime = callTime;
		_vStoredEye[ViroManipulator::FROM]      = _vEye;
		_qStoredRotation[ViroManipulator::FROM] = _qRotation;

		osg::Vec3d vDirection = _vLook; //getLookVec();
		vDirection.normalize();
		vDirection *= (_speed);
		//_vLook = vDirection;

		_bFlying   = true;
		Enable(PERIPHERAL_LOCK);	// Disable user controls
		Disable(GRAVITY);			// Disable Gravity if active
		}
}

// Provides a smooth Self-Correction of Position, ensuring that 
// StrafeVector lies on XY-Plane + Automatic-Avoidance + Collision Detection.
void ViroManipulator::AutoControl(double callTime){
	if (bAutoControlLock) return;

	_bImpact               = false;
	_bAvoidanceZone        = false;
	_currentGroundDistance = -1.0;
	
	// Collision Detection & Avoidance
	if (isEnabled(COLLISIONS) && _bIntersect){
		double impactDistance = Vec3d(_vEye - _vIpoint).length();

		if ((_speed != 0.0) && (impactDistance < AvoidanceDistance)){
			_tLastAvoidanceWarn = callTime;
			//_speed *= 0.1;
			_speed = 0.0;
			_bAvoidanceZone = true;
/*			
			osg::Vec3d vBump = VecBump(_vInormal, _vLook);
			osg::Vec3d vImpactStrafe = vBump ^ _vLook;

			osg::Quat qAvoidanceCorrection;
			double impactAngle = (90.0 - VecAngle(-_vLook,_vInormal));
			double s = (_speed*_modelScale*0.0001);
			double correctionAngle =  _AvoidanceReaction * impactAngle * s;
			if (impactAngle > 85.0) correctionAngle = 0.0;
*/
			// In this case we crashed. Totally disable user-control.
			if (impactDistance <= BumpDistance){
				_bImpact = true;
				//osg::notify(DEBUG_INFO)<<"Impact !! Sorry that was too hard to avoid...\n";
				//osg::Vec3d E = (vBump*BumpDistance) + _vIpoint;
				_vInormal.normalize();
				osg::Vec3d E = (_vInormal * BumpDistance) + _vIpoint;
				//osg::Vec3d D = _vEye - E;
				_vEye = E;
				_speed = 0.0;
				_UserControlPercentage = 1.0;
				
				//_qRotation = computeQuat(_vEye,_vTarget-D,_vUp);
				}

			// ...Else try to avoid surfaces...
			else {
				if ( isEnabled(AVOIDANCE) ){
					osg::Vec3d vBump = VecBump(_vInormal, _vLook);
					osg::Vec3d vImpactStrafe = vBump ^ _vLook;

					osg::Quat qAvoidanceCorrection;
					double impactAngle = (90.0 - VecAngle(-_vLook,_vInormal));
					double s = (_speed*_modelScale*0.0001);
					double correctionAngle =  _AvoidanceReaction * impactAngle * s;
					if (impactAngle > 85.0) correctionAngle = 0.0;

					double dd = (AvoidanceDistance - BumpDistance);
					_UserControlPercentage = (impactDistance - BumpDistance) / dd;
					//osg::notify(DEBUG_INFO)<<"You are about to Crash. Please correct. Control Percentage = "<<(_UserControlPercentage*100.0)<<"\n";
				
					// If we are approaching surface, enable interpolated speed limiter
					double revLimiter = _Mix.interpolate(_UserControlPercentage,SurfaceSpeedLimiter,_speed);
					if (_speed > revLimiter) _speed = revLimiter;

					// Calculate the right correction angle step
					correctionAngle = _Mix.interpolate(_UserControlPercentage,correctionAngle,0.0f);


					//_speed *= _Mix.interpolate(_UserControlPercentage,0.0,(impactAngle/90.0));

					qAvoidanceCorrection.makeRotate(-correctionAngle, vImpactStrafe);

					// Apply correction to current quat.
					_qRotation = _qRotation * qAvoidanceCorrection;
					}
				}
			}

		// Reset Full control after a certain amount of time.
		else if (impactDistance >= AvoidanceDistance && _tLastAvoidanceWarn>=0.0){
			//_UserControlPercentage = 1.0;
			double Timer = 2.0;
			double t = (callTime-_tLastAvoidanceWarn) / Timer;

			if (t >= 1.0) _UserControlPercentage = 1.0;
			else{
				_UserControlPercentage = _Mix.interpolate(t,_UserControlPercentage,1.0);
				osg::notify(DEBUG_INFO)<<"Please Wait. Full control in "<<Timer-(callTime-_tLastAvoidanceWarn)<<" Seconds.\n";
				}
			}
		}

	// Apply Gravity
	if ( isEnabled(GRAVITY) ){
		osg::Vec3d P,N;
		double R = _NODE->getBound().radius();

		// Check for surface attraction
		if ( Intersect(_vEye+Vec3d(0.0,0.0,(_tFallTimer*R*0.5)),_vEye-Vec3d(0.0,0.0,R), P,N) ){
			_currentGroundDistance = _vEye.z()-P.z();
			if ( _vEye.z() > P.z() + GroundDistance ){
				_bSurfaceImpact = false;
				//_CurrentGravityAcceleration *= GravityAcceleration;
				_tFallTimer += getDtime();
				
				// My function to approx. gravity
				//double accel = pow(_tFallTimer,double(GravityAcceleration)); //pow(log(1.+_tFallTimer),double(GravityAcceleration));
				//_vEye -= Vec3d(0.0,0.0,(accel * GravityAcceleration * 5.));
				_vEye[2] -= (_tFallTimer * _tFallTimer) * GravityAcceleration;
				
				// Look down when falling
				/*
				if ( _vUp.z() > 0.0 ){
					osg::Quat qFall;
					// ----------------------- Front : Rear weights
					double r = (_speed >=0.0)? -0.05 : 0.02;
					qFall.makeRotate((r * _vUp.z()), _vStrafe );
					_qRotation = _Mix.interpolate(0.4,_qRotation,_qRotation*qFall);
					}
				*/
				}
			// Impact
			else {
				_tFallTimer = 0.0;
				//_CurrentGravityAcceleration = GravityAcceleration;
				if (!_bSurfaceImpact) _tLastImpact = callTime;
				_bSurfaceImpact = true;
				
				// Surface adaption
				if (isEnabled(SURFACE_ADAPTION) && (_speed!=0.0 || (callTime-_tLastImpact)<5.0)){
					Vec3d lookP,LA;
					LA = _vLook;
					LA.normalize();
					LA *= 4.0;
					LA += _vEye;
					//Intersect(_vTarget+Vec3d(0.0,0.0,R),_vTarget-Vec3d(0.0,0.0,R*2.0), lookP,N);
					Intersect(LA+Vec3d(0.0,0.0,R),LA-Vec3d(0.0,0.0,R*2.0), lookP,N);
					//osg::notify(DEBUG_INFO)<<"--> SURFACE ADAPTION.\n";
					//N = RightNormal(,N)

					osg::Quat qAdapt;
					qAdapt.makeRotate( Vec3d(_vUp), Vec3d(_Mix.interpolate(SurfaceAdaption,_vUp,N)) );
					_qRotation = _Mix.interpolate(0.4,_qRotation,_qRotation*qAdapt);
					}
				}

			if (_bSurfaceImpact && (P.z()+GroundDistance >= _vEye.z()))
				//_vEye = _Mix.interpolate(0.5,_vEye,Vec3d(_vEye.x(),_vEye.y(),P.z()+BumpDistance));
				//_vEye = Vec3d(_vEye.x(),_vEye.y(),P.z()+(BumpDistance*0.99));
				_vEye[2] = P.z() + GroundDistance;


}

		// Out of scope
		else { _tFallTimer = 0.0; }

		if (_bSurfaceImpact && _speed != 0.0) _speed *= SurfaceAdhesion;
		}

	// Hotspot Engine Reactor --- (Dec 27)
	/*
	if ( isEnabled(HOTSPOT_REACTION) ){
		osg::Quat qReactor;
		int c = Hotspots.get()->checkCaptures( _vEye );
		if ( c > 0 ){
			Vec3d newTarget = _vTarget + Hotspots.get()->getReactionVector();
			qReactor = computeQuat(_vEye,newTarget,_vUp);

			_qRotation = _Mix.interpolate(Hotspots.get()->getTotalFeedback(), _qRotation, qReactor);
			
			if ( Hotspots.get()->getTotalGravity() != 0.0f ){
				_vEye  = _Mix.interpolate(fabs( Hotspots.get()->getTotalGravity() ), _vEye, newTarget);
				_speed = _Mix.interpolate(fabs( Hotspots.get()->getTotalGravity() ), _speed, 0.0);
				//osg::notify(ALWAYS)<<"Eye influenced.\n";
				}
			
			//osg::notify(ALWAYS)<<"Inside "<< c <<" hotspots. Total Feedback = "<< Hotspots.get()->getTotalFeedback() <<"\n";
			}
		}
	*/

	// Roll Correction
	if ( _vStrafe.z()!=0.0 && (!isEnabled(SPIDERMAN_PICKING) || !isEnabled(PERIPHERAL_LOCK))){
		osg::Quat qCorrection;
		double r = _vStrafe.z() * RollAutoCorrection;	// Gran classe
		//r *= getDtime();
		qCorrection.makeRotate(r, _vLook);

		// Apply correction to current quat.
		_qRotation = _qRotation * qCorrection;
		}

	if ( isEnabled(Z_COLLIDE) && !isEnabled(GRAVITY) ){
		osg::Vec3d P,N;
		double Rad = _NODE->getBound().radius();

		// Check for surface Z collisions
		if ( Intersect(_vEye/*+Vec3d(0.0,0.0,Rad)*/,_vEye-Vec3d(0.0,0.0,Rad), P,N) ){
			_currentGroundDistance = _vEye.z()-P.z();
			if ( (_vEye.z() - P.z()) <= GroundDistance ){
				_vEye[2] = P.z() + GroundDistance;
				}
			}
		}

	// Wall Reaction
	if ( isEnabled(COLLISIONS) && (_bImpact || _bAvoidanceZone) ){
		osg::Vec3d R(_vInormal);
		
		R.normalize();
		R *= 0.1;
		_vEye    += R;
		_vTarget += R;
		}

	if ( isEnabled(GLASS_PRISON) ){
		bool impact = false;
		if (_vEye.x() > _GlassPrison.xMax()){ _vEye = Vec3d(_GlassPrison.xMax(),_vEye.y(),_vEye.z()); impact=true; }
		if (_vEye.x() < _GlassPrison.xMin()){ _vEye = Vec3d(_GlassPrison.xMin(),_vEye.y(),_vEye.z()); impact=true; }

		if (_vEye.y() > _GlassPrison.yMax()){ _vEye = Vec3d(_vEye.x(),_GlassPrison.yMax(),_vEye.z()); impact=true; }
		if (_vEye.y() < _GlassPrison.yMin()){ _vEye = Vec3d(_vEye.x(),_GlassPrison.yMin(),_vEye.z()); impact=true; }

		if (_vEye.z() > _GlassPrison.zMax()){ _vEye = Vec3d(_vEye.x(),_vEye.y(),_GlassPrison.zMax()); impact=true; }
		if (_vEye.z() < _GlassPrison.zMin()){ _vEye = Vec3d(_vEye.x(),_vEye.y(),_GlassPrison.zMin()); impact=true; }

		if (impact && _speed != 0.0) _speed = _Mix.interpolate(0.3, _speed, 0.0);
		}

	if ( isEnabled(AUTO_SCALE_STEP) ){
		osg::Vec3d C = _NODE.get()->getBound().center();
		double max,min,r;
		r = _NODE.get()->getBound().radius();
		max = C.z() + r;
		min = C.z() - r;

		if (_currentGroundDistance > 0.0) autoStepFactor = _currentGroundDistance/2000;
		else autoStepFactor = (_vEye.z()-10.0) / 2500; //(max-min);
		//autoStepFactor -= 0.3;

		double R = 3;

		if (autoStepFactor < 0.1){
			//_Viewer->getCamera()->setNearFarRatio( autoStepFactor * 0.001 );
			double Zfar;
			Zfar = _Mix.interpolate(autoStepFactor/0.1, 800,9000);
			//_Viewer->getCamera()->setComputeNearFarMode(osg::CullSettings::DO_NOT_COMPUTE_NEAR_FAR);
			//_Viewer->getCamera()->setProjectionMatrixAsPerspective(30.0,R, 0.01,Zfar);
			_Viewer->getCamera()->setNearFarRatio( 0.00001 );
			RollWithYaw = _Mix.interpolate(autoStepFactor/0.1, 0.01,1.5); // 0.01;
			}
		else {
			_Viewer->getCamera()->setComputeNearFarMode(osg::CullSettings::COMPUTE_NEAR_FAR_USING_PRIMITIVES);
			RollWithYaw = 1.5;
			_Viewer->getCamera()->setNearFarRatio( 0.01 );
			}

		autoStepFactor *= 5.0;
		}
}

void ViroManipulator::init(const GUIEventAdapter& ,GUIActionAdapter& ){
	flushMouseEventStack();
	_FrameDT       = 0.0;
	_lastFrameTime = 0.0;
	osg::notify(ALWAYS)<<"ViRo-Manipulator initialized.\n";
}

bool ViroManipulator::MouseListener(){
	bool EventCaptured = false;

	// return if less then two events have been added.
	if (_ev0.get()==NULL || _ev1.get()==NULL) return false;

	double dt = _ev0->time() - _ev1->time();

	unsigned int eventMask  = _ev0->getEventType();

	double step = (Acceleration + _speed) * dt;

	if (eventMask == GUIEventAdapter::SCROLL || _padEvent == NAVPAD_FORWARD || _padEvent == NAVPAD_BACKWARD){
		if ( _ev0->getScrollingMotion() == GUIEventAdapter::SCROLL_UP || _padEvent == NAVPAD_FORWARD){
			_speed += step;
			EventCaptured = true;
			osg::notify(ALWAYS)<<"Boost Up to "<<_speed<<"\n";
			}
		if ( _ev0->getScrollingMotion() == GUIEventAdapter::SCROLL_DOWN || _padEvent == NAVPAD_BACKWARD){
			if (_speed > 0.0)	_speed = 0.0;
			else				_speed -= step;

			EventCaptured = true;
			osg::notify(ALWAYS)<<"Slow Boost to "<<_speed<<"\n";
			}
		}

	if (!_bFlying && _speed != 0.0){
		//if (eventMask == GUIEventAdapter::KEYDOWN) boost(UP);
		//else
			boost();

		EventCaptured = true;
		}

	bool b = (	_padEvent == NAVPAD_STRAFELEFT
				|| _padEvent == NAVPAD_TURNLEFT
				|| _padEvent == NAVPAD_STRAFERIGHT
				|| _padEvent == NAVPAD_TURNRIGHT
				|| _padEvent == NAVPAD_PITCHUP
				|| _padEvent == NAVPAD_PITCHDOWN
				|| _padEvent == NAVPAD_UP
				|| _padEvent == NAVPAD_DOWN );

	if (b || (!isEnabled(PERIPHERAL_LOCK) && !_bFlying)){
		double dx = _ev0->getXnormalized();
		double dy = _ev0->getYnormalized();

		if (isEnabled(AVOIDANCE)){
			dx *= (double)_UserControlPercentage;
			dy *= (double)_UserControlPercentage;
			}

		// Web NavPad
		#define NAVPAD_SENSIBILITY	0.3
		bool lock = (_padEvent == NAVPAD_STRAFELEFT || _padEvent == NAVPAD_STRAFERIGHT || _padEvent == NAVPAD_UP || _padEvent == NAVPAD_DOWN);

		if (_padEvent == NAVPAD_STRAFELEFT || _padEvent == NAVPAD_TURNLEFT)  { dx = -NAVPAD_SENSIBILITY;  dy = 0; }
		if (_padEvent == NAVPAD_STRAFERIGHT || _padEvent == NAVPAD_TURNRIGHT){ dx = NAVPAD_SENSIBILITY; dy = 0; }
		if (_padEvent == NAVPAD_UP){ dx = 0; dy = NAVPAD_SENSIBILITY; }
		if (_padEvent == NAVPAD_DOWN){ dx = 0; dy = -NAVPAD_SENSIBILITY; }

		if (dx!=0.0 || dy!=0.0 || _UserControlPercentage > 0.0){
			if (b && lock) _bHoldCTRL = true;
			turn(-dx,-dy);
			if (b && lock) _bHoldCTRL = false;
			EventCaptured = true;
			}
	}
	// Update some data
	SyncData();

	if (EventCaptured) _bNeedUpdateTrace = true;
	return EventCaptured;
}

// UNUSED. Future Uses
bool ViroManipulator::handleRelease(const GUIEventAdapter& ea){
	// Mid Button
	if (ea.getButtonMask() == GUIEventAdapter::MIDDLE_MOUSE_BUTTON){
		return true;
		}

	// Left Button
	else if (ea.getButtonMask() == GUIEventAdapter::LEFT_MOUSE_BUTTON){
		return true;
		}

	// Right Button
	else if (ea.getButtonMask() == GUIEventAdapter::RIGHT_MOUSE_BUTTON){
		Enable(PERIPHERAL_LOCK);
		return true;
		}
	return false;
}


//Event Listener
bool ViroManipulator::handle(const GUIEventAdapter& ea,GUIActionAdapter& us){
	_lastFrameTime = ea.getTime();
	switch( ea.getEventType() ){
		case(GUIEventAdapter::FRAME):{
			addMouseEvent(ea);

			if (RollWithYaw >0.0 || isEnabled(COLLISIONS)) AutoControl( ea.time() );
			if ( _bNeedUpdateTrace ) Trace();
			if (_bFlying){
				// Time based percentage
				double t = (ea.time() - _FlyToCallTime)/FlyToDurationTime;
				
				// New interpolated Position
				_vEye = _Mix.interpolate(t,_vStoredEye[ViroManipulator::FROM],
					                        _vStoredEye[ViroManipulator::TO],
											_Mix.BOOST_TO);
				// Apply Inertia
				/*
				if (_speed != 0.0){
					//_vEye += _Mix.interpolate(t, _vLook,Vec3d(0.0,0.0,0.0)) * t;
					//if (_vEye[2] < BumpDistance)
					}
				*/

				// New interpolated Rotation
				_qRotation = _Mix.interpolate(t,_qStoredRotation[ViroManipulator::FROM],
					                             _qStoredRotation[ViroManipulator::TO],
												 _Mix.SQUARE);
				
				// Flight terminated. Unset bool & release Mouse-Control Locks..
				if (t > 1.0){
					_bFlying  = false;
					_bReqHome = false;
					_speed    = 0.0;
					//SyncData();
					}
				}

			if ( MouseListener() ) us.requestRedraw();
			us.requestContinuousUpdate(true);
			return false;
			}
		case(GUIEventAdapter::RESIZE):{
			init(ea,us);
			us.requestRedraw();
			return true;
			}
		case(GUIEventAdapter::MOVE):{
			addMouseEvent(ea);

			if ( MouseListener() ) us.requestRedraw();
			us.requestContinuousUpdate(true);
			return true;
			}
		case(GUIEventAdapter::SCROLL):{
			//addMouseEvent(ea);
			//osg::notify(ALWAYS) << "Scroll\n";
			switch (ea.getScrollingMotion()){
				case(GUIEventAdapter::SCROLL_UP):{
					addMouseEvent(ea);
					osg::notify(ALWAYS) << "Scroll-up\n";

					if ( MouseListener() ) us.requestRedraw();
					us.requestContinuousUpdate(true);
					return true;
					}
				case(GUIEventAdapter::SCROLL_DOWN):{
					addMouseEvent(ea);
					osg::notify(ALWAYS) << "Scroll-down\n";

					if ( MouseListener() ) us.requestRedraw();
					us.requestContinuousUpdate(true);
					return true;
					}
				}

			//if ( MouseListener() ) us.requestRedraw();
			//us.requestContinuousUpdate(true);
			//return true;
			}

/* OLD IMPLEMENTATION for OSG 1.0
		case(GUIEventAdapter::SCROLL_UP):{
			addMouseEvent(ea);
			osg::notify(ALWAYS) << "Scroll-up\n";

			if ( MouseListener() ) us.requestRedraw();
			us.requestContinuousUpdate(true);
			return true;
			}
		case(GUIEventAdapter::SCROLL_DOWN):{
			addMouseEvent(ea);
			osg::notify(ALWAYS) << "Scroll-down\n";

			if ( MouseListener() ) us.requestRedraw();
			us.requestContinuousUpdate(true);
			return true;
			}
*/
		// double click
/*
		case(GUIEventAdapter::DOUBLECLICK):{
			osg::notify(DEBUG_INFO)<<"Double Left click.\n";
			_bSatMode = false;
			if (handlePick(_ev0->getXnormalized(),_ev0->getYnormalized())&&!_bFlying) FlyTo(ea.time());
			}
*/
		// Mouse Button Push
		case(GUIEventAdapter::PUSH):{
			//addMouseEvent(ea);
			
			// Mid Button
			if (ea.getButtonMask() == GUIEventAdapter::MIDDLE_MOUSE_BUTTON){	
				osg::notify(DEBUG_INFO)<<"Mid-Mouse pressed.\n";
				//_bSatMode = false;
				
				// During a Fly-To, stop flying...
				if (_bFlying){
					_speed     = 0.0;
					_bFlying   = false;
					Disable(PERIPHERAL_LOCK);
					}

				else {
					float a = 0.4;
					if (_bHoldCTRL) a = 0.0;

					if (!_Viewer && handlePick(_ev0->getX() ,_ev0->getY(),a) &&!_bFlying) FlyTo(ea.time());
					else if (handlePick(_ev0->getXnormalized(),_ev0->getYnormalized(),a) &&!_bFlying) FlyTo(ea.time());
					}
				}
			
			// Left Button
			else if (ea.getButtonMask() == GUIEventAdapter::LEFT_MOUSE_BUTTON){
				
				// Double-Click
				/*
				if ((ea.time() -_tLastLMB) < 0.4){
					osg::notify(DEBUG_INFO)<<"Double Left click.\n";
					_bSatMode = false;
					//if (handlePick(_ev0->getXnormalized(),_ev0->getYnormalized())&&!_bFlying) FlyTo(ea.time());
					//if (handlePick(_ev0->getX(),_ev0->getY())&&!_bFlying) FlyTo(ea.time());
					if (!_Viewer && handlePick(_ev0->getX() ,_ev0->getY())&&!_bFlying) FlyTo(ea.time());
					else if (handlePick(_ev0->getXnormalized(),_ev0->getYnormalized())&&!_bFlying) FlyTo(ea.time());
					}
				*/
				// Single Click
				/* else */ {
					osg::notify(DEBUG_INFO)<<"Left Button pressed.\n";
					// During a Fly-To, stop flying...
					if (_bFlying){
						_bFlying   = false;
						//Disable(PERIPHERAL_LOCK);
						}

					_speed = 0.0;
					_UserControlPercentage = 1.0;
					}
				_tLastLMB = ea.time();	// Update time last LMB.
				}

			// Right Button
			else if (ea.getButtonMask() == GUIEventAdapter::RIGHT_MOUSE_BUTTON){
				Disable(PERIPHERAL_LOCK);
				}

			if ( MouseListener() ) us.requestRedraw();
			//us.requestContinuousUpdate(true);
			return true;
			}

		case(GUIEventAdapter::RELEASE):{
			Enable(PERIPHERAL_LOCK);

			/*
			if (ea.getButtonMask() == GUIEventAdapter::RIGHT_MOUSE_BUTTON){
				Enable(PERIPHERAL_LOCK);
				}
			*/
			//addMouseEvent(ea);

			//handleRelease( ea );

			if ( MouseListener() ) us.requestRedraw();
			us.requestContinuousUpdate(false);
			return true;
			}

		// Keyboard
		case(GUIEventAdapter::KEYDOWN):{
			//osg::notify(DEBUG_INFO) << "A key has been pressed...\n";

			if (ea.getKey()==GUIEventAdapter::KEY_Control_L){
				_bHoldCTRL = true;

				us.requestContinuousUpdate(false);
				return true;
				}
			if (ea.getKey()==' '){
				flushMouseEventStack();
				//home(ea,us);
				
				requestFlyToHome();
				FlyTo( ea.getTime() );

				us.requestRedraw();
				us.requestContinuousUpdate(false);
				return true;
				}
			if (ea.getKey()== 'v'){
				flushMouseEventStack();

				//_vStoredEye[ViroManipulator::TO]      = _vEye;	OLD
				//_qStoredRotation[ViroManipulator::TO] = _qRotation;		OLD
				storeView();

				osg::notify(ALWAYS) << "Position stored. Press 'c' when you want to recall it.\n";
				us.requestContinuousUpdate(false);
				return true;
				}
			if (ea.getKey()== 'c'){
				flushMouseEventStack();
				osg::notify(ALWAYS) << "Recalled stored position.\n";
				
				//FlyTo( ea.time() ); OLD
				reloadView();

				us.requestRedraw();
				us.requestContinuousUpdate(false);
				return true;
				}
			if (ea.getKey()== 'S'){
				flushMouseEventStack();
				Invert(SPIDERMAN_PICKING);
				if (isEnabled(SPIDERMAN_PICKING)) osg::notify(ALWAYS) << "SpiderMan Mode ON.\n";
				else osg::notify(ALWAYS) << "SpiderMan Mode OFF.\n";
				us.requestContinuousUpdate(false);
				return true;
				}
			if (ea.getKey()== GUIEventAdapter::KEY_Up){
				/*
				flushMouseEventStack();
				NumTracerSamplers++;
				osg::notify(ALWAYS) << "Increase Tracer-Samplers : "<<NumTracerSamplers<<"\n";
				us.requestContinuousUpdate(false);
				return true;
				*/
				_padEvent = NAVPAD_FORWARD;
				us.requestContinuousUpdate(false);
				}
			if (ea.getKey()== GUIEventAdapter::KEY_Down){
				/*
				flushMouseEventStack();
				if (NumTracerSamplers>1) NumTracerSamplers--;
				osg::notify(ALWAYS) << "Decrease Tracer-Samplers : "<<NumTracerSamplers<<"\n";
				us.requestContinuousUpdate(false);
				return true;
				*/
				_padEvent = NAVPAD_BACKWARD;
				us.requestContinuousUpdate(false);
				}
			if (ea.getKey()== GUIEventAdapter::KEY_Page_Up){
				_padEvent = NAVPAD_UP;
				us.requestContinuousUpdate(false);
				return true;
				}
			if (ea.getKey()== GUIEventAdapter::KEY_Left){
				_padEvent = NAVPAD_STRAFELEFT;
				us.requestContinuousUpdate(false);
				return true;
				}
			if (ea.getKey()== GUIEventAdapter::KEY_Right){
				_padEvent = NAVPAD_STRAFERIGHT;
				us.requestContinuousUpdate(false);
				return true;
			}
			if (ea.getKey()== GUIEventAdapter::KEY_Page_Down){
				_padEvent = NAVPAD_DOWN;
				us.requestContinuousUpdate(false);
				return true;
			}
			if (ea.getKey()== 'f'){
				
				// TODO : Switch Fullscreen ON/OFF
				
				return true;
				}
			if (ea.getKey()== 'l'){
				/*
				_HeadLight->getLight()->setLightNum(3);
				_HeadLight->getLight()->setDiffuse( Vec4(1,1,1, 0) );
				_HeadLight->getLight()->setDirection( _vLook );
				_HeadLight->getLight()->setPosition( Vec4(_vEye.x(),_vEye.y(),_vEye.z(), 0) );
				*/
				_bLockDir = !_bLockDir;
				
				if (_bLockDir){
					SyncData();
					osg::Vec3d D = _vLook;
					D.normalize();
					_vLockDir = D;
					}

				us.requestContinuousUpdate(false);
				return true;
				}
				
			if (ea.getKey()== 'k'){
				flushMouseEventStack();
				Invert(COLLISIONS);
				//if (isEnabled(COLLISIONS)) osg::notify(ALWAYS) << "Model Collision ON.\n";
				//else osg::notify(ALWAYS) << "Model Collision OFF.\n";
				us.requestContinuousUpdate(false);
				return true;
				}
			if (ea.getKey()== 'i'){
				flushMouseEventStack();
				_Viewer->getCamera()->setProjectionMatrixAsPerspective(30.0,3, 0.01,50);
				us.requestContinuousUpdate(false);
				return true;
				}
			if (ea.getKey()== 'g'){
				flushMouseEventStack();
				Invert(GRAVITY);
				//if (isEnabled(GRAVITY)) osg::notify(ALWAYS) << "Gravity ON.\n";
				//else osg::notify(ALWAYS) << "Gravity OFF.\n";
				us.requestContinuousUpdate(false);
				return true;
				}
				
			// Keyboard navigation
			if (ea.getKey()== 'a'){
				//flushMouseEventStack();
				Enable(PERIPHERAL_LOCK);
				double d = 50.;
				turn( d*getDtime(), 0.0);
				SyncData();
				us.requestContinuousUpdate(false);
				return true;
				}
			if (ea.getKey()== 'd'){
				//flushMouseEventStack();
				Enable(PERIPHERAL_LOCK);
				double d = 50.;
				turn(-d* getDtime(), 0.0);
				SyncData();
				us.requestContinuousUpdate(false);
				return true;
			}
			if (ea.getKey()== 'w'){
				//flushMouseEventStack();
				Enable(PERIPHERAL_LOCK);
				_speed += Acceleration * getDtime();
				boost();
				SyncData();
				us.requestContinuousUpdate(false);
				return true;
			}
			if (ea.getKey()== 's'){
				//flushMouseEventStack();
				Enable(PERIPHERAL_LOCK);
				_speed = 0.0;
				us.requestContinuousUpdate(false);
				return true;
			}
			if (ea.getKey()== 'x'){
				//flushMouseEventStack();
				Enable(PERIPHERAL_LOCK);
				_speed -= Acceleration * getDtime();
				boost();
				SyncData();
				us.requestContinuousUpdate(false);
				return true;
				}

			return true;
			}
		case(GUIEventAdapter::KEYUP):{
			_padEvent = NAVPAD_NONE;

			if (ea.getKey()==GUIEventAdapter::KEY_Control_L){
				_bHoldCTRL = false;

				us.requestContinuousUpdate(false);
				return true;
				}

			return true;
			}
		
		default: return false;
		}
//	return false;
}

// Home : compute home and warp mouse cursor to screen center.
void ViroManipulator::home(const GUIEventAdapter& ea,GUIActionAdapter& us){
	if (getAutoComputeHomePosition()) computeHomePosition();

	computePosition(_homeEye, _homeCenter, _homeUp);
	_holdTarget = _homeCenter;

	_speed = 0.0;

	us.requestRedraw();

	//us.requestWarpPointer((ea.getXmin()+ea.getXmax())/2.0f,(ea.getYmin()+ea.getYmax())/2.0f);
	flushMouseEventStack();
}

void ViroManipulator::requestFlyToHome(){
	_qStoredRotation[TO] = computeQuat(_homeEye,_homeCenter,_homeUp);
	_vStoredEye[TO] = _homeEye;

	_bReqHome = true;
}

// Return the reflected vector bumping on surface with normal n and direction v.
osg::Vec3d ViroManipulator::VecBump(osg::Vec3d n, osg::Vec3d v){
	osg::Vec3d r;
	r = n * (n*v);
	r = v - (r * 2.0);
	r.normalize();
	return r;
}

// This function returns the right surface normal according to eye position.
inline osg::Vec3d ViroManipulator::RightNormal(osg::Vec3d p, osg::Vec3d n){
	double dist  = Vec3d(_vEye - (p+n)).length();
	double idist = Vec3d(_vEye - (p-n)).length();
	if (dist < idist) return n;
	else return -n;
}

// Returns angle between 2 vectors in degrees
double ViroManipulator::VecAngle(osg::Vec3d a, osg::Vec3d b){
	double ang;
	a.normalize();
	b.normalize();
	ang = RadiansToDegrees( acos(a * b) );
	return fabs( ang );
}

// Computes all intersections in line segment (p1,p2) and returns the nearest impact point in 
// vResult and its associated surface normal.
bool ViroManipulator::Intersect(osg::Vec3d p1,osg::Vec3d p2, osg::Vec3d& vResult,osg::Vec3d& vNorm){
	osg::ref_ptr<osg::LineSegment> seg = new osg::LineSegment;
	seg->set(p1,p2);
	if ( !seg->valid() ) return false;

	osgUtil::IntersectVisitor iv;
	iv.setTraversalMode(osgUtil::IntersectVisitor::TRAVERSE_ACTIVE_CHILDREN); //Only visible children
	iv.setLODSelectionMode(osgUtil::IntersectVisitor::USE_SEGMENT_START_POINT_AS_EYE_POINT_FOR_LOD_LEVEL_SELECTION);
	iv.addLineSegment(seg.get());
	iv.setEyePoint( _vEye );

	_NODE->accept( iv );

	bool hitfound = false;
	if (iv.hits()){
		osgUtil::IntersectVisitor::HitList& hitList = iv.getHitList(seg.get());
		if (!hitList.empty()){
			for(osgUtil::IntersectVisitor::HitList::iterator hitr=hitList.begin(); hitr!=hitList.end(); ++hitr){
				if( hitr->_geode.valid() && (hitr->_geode.get()->getName()!="Contraint_OutBox_Geode") && (hitr->_geode.get()->getName()!="environment")){
						osg::Vec3d ip = hitr->getWorldIntersectPoint();
						osg::Vec3d in = hitr->getWorldIntersectNormal();

						if(!hitfound){
							vResult = ip;
							vNorm   = in;
							}
						else{
							osg::Vec3d vLastIP    = vResult;
							osg::Vec3d vCurrentIP = ip;
							double dPrev    = Vec3d(_vEye - vLastIP).length();
							double dCurrent = Vec3d(_vEye - vCurrentIP).length();

							if( dCurrent < dPrev ){
								vResult = ip;
								vNorm   = in;
								}
							}
						hitfound = true;
						}
					}
				}
			}
	if (hitfound) vNorm = RightNormal(vResult,vNorm);
	return hitfound;
}

/// Tracer : This is the core engine of collision avoidance algorithm.
/// Shots out some rays with 2 different methods, and tries to compute the 
/// best results and let AutoControl() avoid obstacles.
bool ViroManipulator::Trace(){
	//const osg::BoundingSphere& BS =_NODE->getBound();
	double lookAhead = AvoidanceDistance;	// Optimized ray length
	osg::Vec3d vEnd,vStart,L;
	bool hit = false;

	L = _vLook;
	L.normalize();
	vEnd = _vEye + (L * lookAhead);
	vStart = _vEye - (L * lookAhead * 0.5);

	unsigned int hits = 0;

	// If activated, this method tries to shot out a given number of samplers.
	// Then it gathers all the results and store average values to provide the max possible 
	// accuracy.
	if ( isEnabled(MULTISAMPLING) ){
		Vec3d ip,in,N,P,vSampler;

		for (int i=0; i<NumTracerSamplers; i++){
			double s = double((rand()%200)-100) / 100.0;	// Random double in [-1,1]
			double u = - double((rand()%100)) / 100.0;		// Random double in [-1,0]

			// Calculate displacement vector and apply to final destination
			vSampler = ((_vStrafe * s * TracerFrustum.x())+(_vUp * u * TracerFrustum.y()))*lookAhead;

			// Gather hits
			if (hit |= Intersect(_vEye,(vEnd+vSampler), ip,in)){
				hits++;
				N += in;
				P += ip;
				}
			}
		// Avg Values
		if (hits >0){
			_vInormal = (N/hits);
			_vIpoint  = (P/hits);
			osg::notify(DEBUG_INFO) << "Tracer Hits = "<< hits <<"\n";
			}
		}

	// Else use standard method
	else {
		if (_speed == 0.0) return false;

		Vec3d p[8],n[8],X,Y,Slide,W,H,L;
		X = _vStrafe * TracerFrustum.x() * lookAhead;
		Y = _vStrafe * TracerFrustum.y() * lookAhead;
		double avatarW = 0.5;

		Slide = _vStrafe;
		H = Vec3d(0,0,lookAhead);
		Slide.normalize();
		W = Slide;
		W *= avatarW;
		Slide *= lookAhead;
		L = _vLook;
		L.normalize();
		L *= lookAhead;

		if (_speed > 0.0){
			if (hit |= Intersect(_vEye,(_vEye + L), p[0],n[0])) hits++;
			if (hit |= Intersect((_vEye+W),(_vEye + L + W), p[1],n[1])) hits++;
			if (hit |= Intersect((_vEye-W),(_vEye + L - W), p[2],n[2])) hits++;
			}
		else {
			if (hit |= Intersect(_vEye,(_vEye - L), p[0],n[0])) hits++;
			if (hit |= Intersect((_vEye+W),((_vEye - L) + W), p[1],n[1])) hits++;
			if (hit |= Intersect((_vEye-W),((_vEye - L) - W), p[2],n[2])) hits++;
			} 

		if (hit |= Intersect(_vEye,(_vEye - Slide), p[3],n[3])) hits++;
		if (hit |= Intersect(_vEye,(_vEye + Slide), p[4],n[4])) hits++;
		if (hit |= Intersect(_vEye,(_vEye + Vec3d(0,0,GroundDistance)), p[5],n[5])) hits++;

		//if (hit |= Intersect(vStart,vEnd-Y, p[3],n[3])) hits++;

		// vertical trace (Apr 26 07)
		//if (hit |= Intersect(vStart,Vec3d(_vEye.x(),_vEye.y(),_vEye.z()-lookAhead), p[4],n[4])) hits++;
		
		if (hits>0){
			_vIpoint  = p[0]; //Vec3d(0,0,0);
			_vInormal = Vec3d(0,0,0);

			for (unsigned int i=0; i<hits; i++){
				if (i>0 && ((_vEye-p[i]).length() < (_vEye-_vIpoint).length()) ) _vIpoint = p[i];
				_vInormal += n[i];
				}

			//_vIpoint  /= (double)hits;
			_vInormal /= (double)hits;
			}
		}

	if (!hit) _UserControlPercentage = 1.0;

	// Update some booleans
	_bIntersect       = hit;
	_bNeedUpdateTrace = false;

	return hit;
}

void ViroManipulator::getUsage(osg::ApplicationUsage& usage) const
{
	usage.addKeyboardMouseBinding("ViRo: Mouse Wheel","Accelerate / Decelerate");
	usage.addKeyboardMouseBinding("ViRo: Left-Button","Stop");
	usage.addKeyboardMouseBinding("ViRo: Right-Button","Press and Turn - Hold CTRL for Slide/Height");
	usage.addKeyboardMouseBinding("ViRo: Mid-Button","Fly-To picked point - Hold CTRL for Satellite Picking");
}

std::string ViroManipulator::handleAction(std::string argument)
{
	std::string retstr = "CORE_DONE";
	std::string lcommand, rcommand;

	this->splitActionCommand(argument, lcommand, rcommand);
	retstr = this->ExecCommand(lcommand, rcommand);
		
	return retstr;
}

std::string ViroManipulator::ExecCommand(std::string lcommand, std::string rcommand){
	std::string retstr_done = "CORE_DONE";
	std::string retstr_fail = "CORE_FAILED";
	
	if ((lcommand.compare("STRAFE") == 0) && (rcommand.compare("LEFT") == 0))       _padEvent = NAVPAD_TURNLEFT;  // NAVPAD_STRAFELEFT
	else if ((lcommand.compare("STRAFE") == 0) && (rcommand.compare("RIGHT") == 0)) _padEvent = NAVPAD_TURNRIGHT; // NAVPAD_STRAFERIGHT
	else if ((lcommand.compare("FORWARD") == 0) && (rcommand.compare("UP") == 0))   _padEvent = NAVPAD_FORWARD;
	else if ((lcommand.compare("FORWARD") == 0)&& (rcommand.compare("DOWN") == 0))  _padEvent = NAVPAD_BACKWARD;

	else if ((lcommand.compare("PITCH") == 0)&& (rcommand.compare("UP") == 0))   _padEvent = NAVPAD_PITCHUP;
	else if ((lcommand.compare("PITCH") == 0)&& (rcommand.compare("DOWN") == 0)) _padEvent = NAVPAD_PITCHDOWN;
	else if ((lcommand.compare("YAW") == 0)&& (rcommand.compare("LEFT") == 0))   _padEvent = NAVPAD_TURNLEFT;
	else if ((lcommand.compare("YAW") == 0)&& (rcommand.compare("RIGHT") == 0))  _padEvent = NAVPAD_TURNRIGHT;
	else if (lcommand.compare("STOP") == 0){
		_speed = 0.0;
		_padEvent = NAVPAD_NONE;
		}
	else if ((lcommand.compare("LIFT") == 0)&& (rcommand.compare("UP") == 0)){
		//_bHoldCTRL = true;
		_padEvent = NAVPAD_UP;
		}
	else if ((lcommand.compare("LIFT") == 0)&& (rcommand.compare("DOWN") == 0)){
		//_bHoldCTRL = true;
		_padEvent = NAVPAD_DOWN;
		}

	else if (lcommand.compare("GOTO") == 0){
		double x,y,z,yaw,pitch,roll;
		if (sscanf(rcommand.c_str(), "%lf %lf %lf %lf %lf %lf", &x,&y,&z, &yaw,&pitch,&roll) == 6){
			// TODO --- Unused??
			}
		else return retstr_fail;
		}

	else if ((lcommand.compare("SET_GROUND_DISTANCE") == 0)){
		double gd;
		if (sscanf(rcommand.c_str(), "%lf", &gd) == 1){
			if (gd > 0.0) setGroundDistance(gd);;
			}
		else return retstr_fail;
		}
	else if ((lcommand.compare("SET_COLLISION_DISTANCE") == 0)){
		double a,b;
		if (sscanf(rcommand.c_str(), "%lf %lf", &a,&b) == 2){
			if ((b > a) && (a > 0.0)){ setImpactDistance( a ); setAvoidanceDistance( b ); }
			}
		else return retstr_fail;
		}
	else if ((lcommand.compare("SET_GLASS_PRISON") == 0)){
		double xmin,ymin,zmin, xmax,ymax,zmax;
		if (sscanf(rcommand.c_str(), "%lf %lf %lf %lf %lf %lf", &xmin,&ymin,&zmin,&xmax,&ymax,&zmax) == 6){
			osg::BoundingBox bb;
			bb.set(xmin,ymin,zmin, xmax,ymax,zmax);
			this->setGlassPrison( bb );
			}
		else return retstr_fail;
		}

	// Options
	else if (lcommand.compare("GRAVITY") == 0){
		Invert( GRAVITY );
		}
	else if (lcommand.compare("COLLISIONS") == 0){
		Invert( COLLISIONS );
		}
	else if (lcommand.compare("GET_GRAVITY") == 0){
		std::string G;
		if ( this->isEnabled(GRAVITY) )		G = std::string("ON");
		else								G = std::string("OFF");
		return G;
		}
	else if (lcommand.compare("GET_COLLISIONS") == 0){
		std::string C;
		if ( this->isEnabled(COLLISIONS) )	C = std::string("ON");
		else								C = std::string("OFF");
		return C;
		}

	else return retstr_fail;
	
	return retstr_done;
}
