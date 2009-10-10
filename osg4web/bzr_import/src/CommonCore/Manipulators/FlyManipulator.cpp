#include <fstream>
#include <sstream>

#include <osg/Quat>
#include <osg/Notify>
#include <osg/io_utils>
#include <osgText/Text>
#include <osgUtil/IntersectVisitor>

#include <CommonCore/Manipulators/FlyManipulator.h>

using namespace osg;
using namespace osgGA;

using namespace Manipulators;

FlyManipulator::FlyManipulator()
{
    m_distance = 1.0;
    m_thrown = false;
    
    m_f2_valid = false;
    m_flying = false;
    m_request_fly = false;
    m_begin_fly_time =0;
    m_flying_duration = 5;

    m_yaw =0;
    m_pitch =0;
    m_needIntersect = false;

	m_constraints = false;

	_maximumDistance = 10000.0f; //FIXME: mettere a posto la costante

    for(unsigned int i = 0; i < 9; i++ )
    {
		m_pts[i] = new CurrentInst;
    }
}

FlyManipulator::~FlyManipulator()
{

}

void FlyManipulator::init(const GUIEventAdapter& ,GUIActionAdapter& )
{
	flushMouseEventStack();
}

osg::Matrixd FlyManipulator::getMatrix() const
{
	return 
		osg::Matrixd::translate( 0.0,0.0, m_distance ) 
			* 
		osg::Matrixd::rotate( m_rotation ) 
			*
		osg::Matrix::translate( m_center );
}

osg::Matrixd FlyManipulator::getInverseMatrix() const
{
	return 
		osg::Matrix::translate( -m_center )
			*
		osg::Matrixd::rotate( m_rotation.inverse() )
			*
		osg::Matrixd::translate( 0.0,0.0,-m_distance );
}

void FlyManipulator::addMouseEvent(const GUIEventAdapter& ea)
{
	m_ga_t1 = m_ga_t0;
	m_ga_t0 = &ea;
}

void FlyManipulator::flushMouseEventStack()
{
	m_ga_t1 = NULL;
	m_ga_t0 = NULL;
}

bool FlyManipulator::isMouseMoving()
{
	if ( m_ga_t0.get() == NULL || m_ga_t1.get() == NULL) return false;

	static const float velocity = 0.1f;

	float dx = m_ga_t0->getXnormalized() - m_ga_t1->getXnormalized();
	float dy = m_ga_t0->getYnormalized() - m_ga_t1->getYnormalized();
	float len = sqrtf( dx * dx + dy * dy );
	float dt = m_ga_t0->time() - m_ga_t1->time();

	return (len > dt * velocity);
}

void FlyManipulator::setNode(osg::Node* node)
{
    m_node = node;
    
    if (m_node.valid())
    {
        const osg::BoundingSphere& bs = m_node->getBound();
        const float minimumDistanceScale = 0.0005f;
		const float maximumDistanceScale = 100.0f;
        
		_minimumDistance = osg::clampBetween( bs.radius() * minimumDistanceScale, 0.00005f, 1.0f );
		_maximumDistance = osg::clampBetween( bs.radius() * maximumDistanceScale, 1.0f, 1000.0f );
            
		osg::notify(osg::INFO) << "Setting Fly Manipulator _minimumDistance to " << _minimumDistance << std::endl;
		osg::notify(osg::INFO) << "Setting Fly Manipulator _maximumDistance to " << _maximumDistance << std::endl;
    }

    if (getAutoComputeHomePosition()) computeHomePosition();    
}

void FlyManipulator::checkConstraints()
{
	if (m_node.valid())
    {
        const osg::BoundingSphere& bs = m_node->getBound();
      
		_minimumDistance = bs.radius() * 0.0025f;
		_maximumDistance = 3 * bs.radius();
            
		//osg::notify(osg::INFO) << "Setting Land Manipulator _minimumDistance to " << _minimumDistance << std::endl;
		//osg::notify(osg::INFO) << "Setting Land Manipulator _maximumDistance to " << _maximumDistance << std::endl;
    }
}

void FlyManipulator::home(const GUIEventAdapter& ,GUIActionAdapter& us)
{
    if (getAutoComputeHomePosition()) 
		computeHomePosition();
    
	computePosition(_homeEye, _homeCenter, _homeUp);

    us.requestRedraw();
}
	
void FlyManipulator::getUsage(osg::ApplicationUsage& usage) const
{
    usage.addKeyboardMouseBinding("Fly: Space","Reset the viewing position to home");
}


void FlyManipulator::computePosition(const osg::Vec3d& eye,const osg::Vec3d& center,const osg::Vec3d& up)
{
	// ---- eye/center/up non li uso
  
	if(m_node.get())
	{
		const osg::BoundingSphere& bs = m_node->getBound();
		m_center = bs._center;
		m_distance = bs._radius /2;
	}
	else
	{
		m_center = osg::Vec3d(0.0, 0.0, 0.0); //FIXME: riscrivere in base al modello caricato
		m_distance = 10;
	}

	m_pitch = 75;  //FIXME: riscrivere in base al modello caricato
	m_yaw = 0;

	YawPitchToRot( m_yaw, m_pitch, m_rotation );

	IntersectTerrain();

	CoordinateFrame coordinateFrame = getCoordinateFrame(m_center);
    m_previousUp = getUpVector(coordinateFrame);
}

void FlyManipulator::setByMatrix(const osg::Matrixd& matrix)
{
    osg::Vec3 lookVector(- matrix(2,0),-matrix(2,1),-matrix(2,2));
    osg::Vec3 eye(matrix(3,0),matrix(3,1),matrix(3,2));
    osg::notify(INFO)<<"eye point "<<eye<<std::endl;
    osg::notify(INFO)<<"lookVector "<<lookVector<<std::endl;

    m_center = eye+ lookVector;
    m_distance = lookVector.length();
    RotToYawPitch(matrix,m_yaw,m_pitch);
    m_pitch = clampBetween( m_pitch, 2.0, 80.0 );
    YawPitchToRot(m_yaw,m_pitch,m_rotation);

	IntersectTerrain();

	CoordinateFrame coordinateFrame = getCoordinateFrame(m_center);
    m_previousUp = getUpVector(coordinateFrame);
}

bool FlyManipulator::handle(const GUIEventAdapter& ea,GUIActionAdapter& us)
{
	switch(ea.getEventType())
    {
        case(GUIEventAdapter::PUSH):
        {
			flushMouseEventStack();
            addMouseEvent(ea);

            if (calcMovement()) us.requestRedraw();
            us.requestContinuousUpdate(false);
            m_thrown = false;
			m_flying = false;
			return true;
        }

        case(GUIEventAdapter::RELEASE):
        {
			if(m_needIntersect)
                IntersectTerrain();

			if (ea.getButtonMask()==0)
            {
                if (isMouseMoving())
                {
                    if (calcMovement())
                    {
                        us.requestRedraw();
                        us.requestContinuousUpdate(true);
                        m_thrown = true;
                    }
                }
                else
                {
                    flushMouseEventStack();
                    addMouseEvent(ea);
                    if (calcMovement()) us.requestRedraw();
                    us.requestContinuousUpdate(false);
                    m_thrown = false;
                }

            }
			else	
            {
                flushMouseEventStack();
                addMouseEvent(ea);
                if (calcMovement()) us.requestRedraw();
                us.requestContinuousUpdate(false);
                m_thrown = false;
            }
            return true;
        }

        case(GUIEventAdapter::DRAG):
        {
			addMouseEvent(ea);
			if (calcMovement()) us.requestRedraw();
			us.requestContinuousUpdate(false);
			m_thrown = false;
			return true;
		}

		case(GUIEventAdapter::MOVE):
        {
            return false;
        }

		case(GUIEventAdapter::KEYDOWN):
		{  
			//ESC - f 
            // tasti disabilitati
			if (ea.getKey() == 'f' || ea.getKey() == GUIEventAdapter::KEY_Escape) //FIXME: osgProducer lo prende lo stesso
			{
				return true;
			}

            if (ea.getKey()==' ')
            {
                flushMouseEventStack();
                m_thrown = false;
                home(ea,us);

                us.requestRedraw();
                us.requestContinuousUpdate(false);
                return true;
            }
            //F1 ---------------------------------------------
            // Store Position
			if ( ea.getKey() == GUIEventAdapter::KEY_F1 && !m_flying )
            {
				m_f2_valid     = true;
				m_f2_center    = m_center;
				m_f2_rotation  = m_rotation;
				m_f2_distance  = m_distance;
				return true;  
            }

            // ---------------------------------------------
            // FlyTo Position
            if ( ea.getKey() == GUIEventAdapter::KEY_F2 && !m_flying )
            {
				if ( m_f2_valid ) 
				{
					m_f1_center    = m_center;
					m_f1_rotation  = m_rotation;
					m_f1_distance  = m_distance;
					m_flying = true;
					m_begin_fly_time = ea.time();
					m_thrown = false;
				}
				return true;  
			}

            if ( ea.getKey() >= GUIEventAdapter::KEY_F5 && ea.getKey()<= GUIEventAdapter::KEY_F12 )
            {
				if(ea.getModKeyMask() & GUIEventAdapter::MODKEY_CTRL)
				{
					// store 
					int idx = ea.getKey() - GUIEventAdapter::KEY_F5;
					m_pts[idx]->x = m_center[0];
					m_pts[idx]->y = m_center[1];
					m_pts[idx]->yaw = m_yaw;
					m_pts[idx]->pitch = m_pitch;
					m_pts[idx]->dist = m_distance;
				}
				else
				{
					// goto   
					int idx = ea.getKey() - GUIEventAdapter::KEY_F5;
					FlyTo( m_pts[idx]->x, m_pts[idx]->y, m_pts[idx]->yaw, m_pts[idx]->pitch, m_pts[idx]->dist );
				}
				return true;  
            }
            return false;
		}

        case(GUIEventAdapter::FRAME):
		{
			if( m_request_fly )
			{
				if ( m_f2_valid ) 
				{
					m_f1_center    = m_center;
					m_f1_rotation  = m_rotation;
					m_f1_distance  = m_distance;
					m_flying = true;
					m_begin_fly_time = ea.time();
					m_thrown = false;
				}
				
				m_request_fly = false;
				return true;  
			}

			if ( m_flying )
			{
				double t2 = (ea.time() - m_begin_fly_time) / m_flying_duration;
				double t1 = 1 - t2; 
            
				m_center   = m_f1_center   *t1 + m_f2_center   * t2;
				m_rotation = m_f1_rotation *t1 + m_f2_rotation * t2;
				m_distance = m_f1_distance *t1 + m_f2_distance * t2;
            
				if( ea.time() > m_begin_fly_time + m_flying_duration )
				{
					RotToYawPitch( osg::Matrixd::rotate( m_rotation ), m_yaw, m_pitch );
					IntersectTerrain();
					m_flying = false;
				}
				
				us.requestRedraw();
			}
          
			if (m_thrown)
			{
				if (calcMovement()) us.requestRedraw();
			}

			checkConstraints();
          
			return false;
		}
        
		default:
		{
			return false;
		}
	}
}

bool FlyManipulator::calcMovement()
{
	// return if less then two events have been added.
	if (m_ga_t0.get()==NULL || m_ga_t1.get()==NULL) return false;

	double dx = m_ga_t0->getXnormalized() - m_ga_t1->getXnormalized();
	double dy = m_ga_t0->getYnormalized() - m_ga_t1->getYnormalized();

	// return if there is no movement.
	if (dx==0 && dy==0) return false;

	unsigned int buttonMask = m_ga_t1->getButtonMask();

	if (buttonMask == GUIEventAdapter::LEFT_MOUSE_BUTTON)
	{
		zoom(dx, dy);
		return true;
	}
	if (buttonMask == GUIEventAdapter::RIGHT_MOUSE_BUTTON)
	{
		rotate(dx, dy);
		return true;
	}
	if (buttonMask == GUIEventAdapter::MIDDLE_MOUSE_BUTTON || buttonMask==(GUIEventAdapter::LEFT_MOUSE_BUTTON | GUIEventAdapter::RIGHT_MOUSE_BUTTON))
	{
		pan(dx, dy);
		return true;
	}
	
	return false;
}

void FlyManipulator::rotate(double dx, double dy)
{
	m_yaw   -= 30*dx;
	m_pitch += 20*dy;

	while(m_yaw < 0 ) m_yaw += 360;
	while(m_yaw > 360) m_yaw -= 360;
	if ( m_pitch < 2  ) m_pitch = 2;
	if ( m_pitch > 80 ) m_pitch = 80;

	YawPitchToRot(m_yaw, m_pitch, m_rotation);

	m_needIntersect = true;
}

/*
void FlyManipulator::rotate()
{
	// rotate camera.
	osg::Vec3 axis;
	double angle;

	double px0 = m_ga_t0->getXnormalized();
	double py0 = m_ga_t0->getYnormalized();

	double px1 = m_ga_t1->getXnormalized();
	double py1 = m_ga_t1->getYnormalized();


	trackball(axis,angle,px1,py1,px0,py0);

	osg::Quat new_rotate;
	new_rotate.makeRotate( angle, axis );

	m_rotation = m_rotation * new_rotate;
}
*/

void FlyManipulator::pan(double dx, double dy)
{
	//Codice di TerrainManipulator

	// pan model.
	double scale = -0.3f * m_distance;

	osg::Matrix rotation_matrix;
	rotation_matrix.makeRotate(m_rotation);


	// compute look vector.
	osg::Vec3d lookVector = -getUpVector(rotation_matrix);
	osg::Vec3d sideVector = getSideVector(rotation_matrix);
	osg::Vec3d upVector = getFrontVector(rotation_matrix);

	// CoordinateFrame coordinateFrame = getCoordinateFrame(_center);
	// osg::Vec3d localUp = getUpVector(coordinateFrame);
	osg::Vec3d localUp = m_previousUp;

	osg::Vec3d forwardVector =localUp^sideVector;
	sideVector = forwardVector^localUp;

	forwardVector.normalize();
	sideVector.normalize();

	osg::Vec3d dv = forwardVector * (dy*scale) + sideVector * (dx*scale);

	m_center += dv;

	// need to recompute the itersection point along the look vector.
        
	if (m_node.valid())
	{

		// now reorientate the coordinate frame to the frame coords.
		CoordinateFrame coordinateFrame =  getCoordinateFrame(m_center);

		// need to reintersect with the terrain
		osgUtil::IntersectVisitor iv;
		iv.setTraversalMask(_intersectTraversalMask);

		double distance = m_node->getBound().radius()*0.1f;
		osg::Vec3d start_segment = m_center + getUpVector(coordinateFrame) * distance;
		osg::Vec3d end_segment = start_segment - getUpVector(coordinateFrame) * (2.0f*distance);

		//osg::notify(INFO)<<"start="<<start_segment<<"\tend="<<end_segment<<"\tupVector="<<getUpVector(coordinateFrame)<<std::endl;

		osg::ref_ptr<osg::LineSegment> segLookVector = new osg::LineSegment;
		segLookVector->set(start_segment,end_segment);
		iv.addLineSegment(segLookVector.get());

		m_node->accept(iv);

		bool hitFound = false;
		if (iv.hits())
		{
			osgUtil::IntersectVisitor::HitList& hitList = iv.getHitList(segLookVector.get());
			if (!hitList.empty())
			{
				notify(INFO) << "Hit terrain ok"<< std::endl;
				osg::Vec3d ip = hitList.front().getWorldIntersectPoint();
				m_center = ip;

				hitFound = true;
			}
		}

		if (!hitFound)
		{
			// ??
			osg::notify(INFO)<<"TerrainManipulator unable to intersect with terrain."<<std::endl;
		}

		coordinateFrame = getCoordinateFrame(m_center);
		osg::Vec3d new_localUp = getUpVector(coordinateFrame);


		osg::Quat pan_rotation;
		pan_rotation.makeRotate(localUp,new_localUp);

		if (!pan_rotation.zeroRotation())
		{
			m_rotation = m_rotation * pan_rotation;
			m_previousUp = new_localUp;
			//osg::notify(osg::NOTICE)<<"Rotating from "<<localUp<<" to "<<new_localUp<<"  angle = "<<acos(localUp*new_localUp/(localUp.length()*new_localUp.length()))<<std::endl;

			//clampOrientation();
		}
		else
		{
			osg::notify(osg::INFO)<<"New up orientation nearly inline - no need to rotate"<<std::endl;
		}
	}
}

/* Vecchio codice
void FlyManipulator::pan(double dx, double dy)
{
	// cambia l'effetto a seconda della distance
  
	float scale = -0.3f * m_distance;

	osg::Matrix rotation_matrix;
	rotation_matrix.set( m_rotation );
	osg::Vec3 dv( dx * scale, dy * scale, 0.0f );
	m_center += dv * rotation_matrix;

	m_needIntersect = true;
}*/

void FlyManipulator::zoom(double dx, double dy)
{
	m_distance *= (1.0f + dy);
	if( m_distance < _minimumDistance ) m_distance = _minimumDistance;
    
	if(m_constraints) //FIXME: finire il controllo. Qua non va bene
		if( m_distance > _maximumDistance ) m_distance = _maximumDistance;	
	
	m_needIntersect = true;
}

void FlyManipulator::RotToYawPitch(osg::Matrixd m, double& yaw, double& pitch  )
{
	osg::Vec3d v = -getUpVector(m); // questo sarebbe il LookVector
	osg::Vec2d y(v[0], v[1]);
	osg::Vec2d p( y.length(), v[2] );
  
	double ToDegree = 180/osg::PI;
	yaw   = atan2(  y[1], y[0] ) *ToDegree -90;  // +  e - 90 trovati per tentativi, per matchare YawPitchToRot
	pitch = atan2(  p[1], p[0] ) *ToDegree +90;
  
	if( yaw < 0 ) yaw += 360;
	if( m_pitch < 2)  m_pitch =2;
	if( m_pitch > 80)  m_pitch =80;

	//yaw = (int)yaw;
	//pitch = (int)pitch;
	//osg::notify(osg::WARN)<<" v= " << v[0] << " " << v[1] <<" " << v[2] <<std::endl;
	//osg::notify(osg::WARN)<<"  yaw= " << yaw << " pitch= " << pitch <<std::endl;
}

void FlyManipulator::YawPitchToRot(double yaw, double pitch, osg::Quat& r  )
{
	static double ToRad = osg::PI / 180.0; 
	r = 
		osg::Quat( m_pitch * ToRad, osg::Vec3(0.1,0.0,0.0)) 
			* 
		osg::Quat( m_yaw * ToRad,   osg::Vec3(0.0,0.0,1.0));

	//osg::notify(osg::INFO)<<"@ yaw= " << yaw << " pitch= " << pitch << std::endl;
	//double a,b;
	//RotToYawPitch(osg::Matrixd::rotate(_rotation),a,b);
}

// verifica che il center sia sul terreno
// eventualmente lo sposta e corregge la distance
void FlyManipulator::IntersectTerrain()
{
	m_needIntersect = false;
   
	if(!m_node.valid()) return;

	// devo creare un segmento - orientato come il lookAt
	// che parte da eye e arriva a ???
	osg::Matrix rotation_matrix( m_rotation );
	osg::Vec3d lookVector = -getUpVector(rotation_matrix);
	lookVector.normalize();
	double d = 2 * m_node->getBound().radius();
	osg::Vec3d eye = m_center - lookVector * m_distance;
	osg::Vec3d p1 = m_center - lookVector * d;
	osg::Vec3d p2 = m_center + lookVector * d;
	osg::Vec3d new_center;

	bool hitFound = Intersect( p1, p2, new_center );

	// sposto il centro e aggiorno la 
	// distance per mantenera la posizione di Eye
	if(hitFound)
	{
		osg::Vec3d delta = eye - new_center;
		m_distance = delta.length();
		if( m_distance < _minimumDistance ) m_distance = _minimumDistance;
		m_center = new_center;
	}
}

bool FlyManipulator::Intersect( osg::Vec3d p1, osg::Vec3d p2, osg::Vec3d& result)
{
	osg::ref_ptr<osg::LineSegment> seg = new osg::LineSegment;
	seg->set( p1, p2 );
	//osg::notify(WARN)  << " eye " << eye << " center " << _center  <<std::endl;
	//osg::notify(WARN)<<" near "<< near << " far "<< far  <<std::endl;

	osgUtil::IntersectVisitor iv;
	//iv.setTraversalMask(_intersectTraversalMask);
	iv.setTraversalMode(osgUtil::IntersectVisitor::TRAVERSE_ACTIVE_CHILDREN); //SIL Only visible children
	//iv.setLODSelectionMode(osgUtil::IntersectVisitor::LODSelectionMode::USE_SEGMENT_START_POINT_AS_EYE_POINT_FOR_LOD_LEVEL_SELECTION);
	iv.addLineSegment(seg.get());
	m_node->accept(iv);
	bool hitfound = false;
	if (iv.hits())
	{
		osgUtil::IntersectVisitor::HitList& hitList = iv.getHitList(seg.get());
		if (!hitList.empty())
		{
			for(osgUtil::IntersectVisitor::HitList::iterator hitr=hitList.begin();  hitr!=hitList.end();  ++hitr )
			{
				if( hitr->_geode.valid() )
				{
					if( hitr->_geode.get() != m_exclude_node.get() )
					{
						osg::Vec3 p= hitr->getWorldIntersectPoint();
						if(!hitfound) 
							result = p;
						else
							if(result[2] < p[2]) 
								result = p;
						
						hitfound = true; 
					}
					else
					{
						//osg::notify(INFO)<<"palla dribbled" << std::endl;
					}
				}
			}
		}
	}

	if(hitfound) 
		osg::notify(DEBUG_FP)<<"picked point: "<< result <<std::endl;

	return hitfound;
}

//FIXME: da rifare!!!!!!
void FlyManipulator::FlyTo( osg::Vec3 wposition )
{
	double x = wposition.x();
	double y = wposition.y();

	if( !m_node.valid() ) 
		return;

	double r = m_node->getBound().radius();
	
	osg::Vec3d c = m_node->getBound().center();
	osg::Vec3d p1(x, y, c[2] - r );
	osg::Vec3d p2(x, y, c[2] + r );
	osg::Vec3d p;

	bool found = Intersect( p1, p2, p );
	if(!found) 
	{
		osg::notify(WARN)<<"FlyToXY: failed to find Z" <<std::endl;
		return;
	}
 
	// calc new yaw 
	osg::Vec3d delta = p - m_center;
	delta[2] = 0;
	delta.normalize();

	double yaw  = atan2(  delta[1], delta[0] ) - osg::PI/2;  

	double fixed_yaw = yaw;
	/*
	if( (yaw-_yaw) < -180) 
		fixed_yaw += 360;
	if( (yaw-_yaw) > 180)  
		fixed_yaw -= 360;
	*/

	static double ToRad = osg::PI / 180.0; 
	m_f2_valid     = true;
	m_f2_center    = p;
	m_f2_distance  = m_distance;
	m_f2_rotation  = 
		osg::Quat( m_pitch * ToRad, osg::Vec3(0.1,0.0,0.0)) 
			* 
		osg::Quat(  fixed_yaw,   osg::Vec3(0.0,0.0,1.0));
  
	m_request_fly = true;
}

void FlyManipulator::FlyTo(double x,double y,double yaw,double pitch, double dist)
{
	if( !m_node.valid() ) return;
	double     r = m_node->getBound().radius();
	osg::Vec3d c = m_node->getBound().center();
	osg::Vec3d p1(x, y, c[2] - r );
	osg::Vec3d p2(x, y, c[2] + r );
	osg::Vec3d p;
	bool found = Intersect( p1, p2, p );
	if(!found) 
	{
		osg::notify(WARN)<<"FlyToXY: failed to find Z" <<std::endl;
		return;
	}

	double fixed_yaw = yaw;
	/*
	if( (yaw-_yaw) < -180) 
		fixed_yaw += 360;
	if( (yaw-_yaw) > 180)  
		fixed_yaw -= 360;
	*/

	static double ToRad = osg::PI / 180.0; 

	m_f2_valid     = true;
	m_f2_center    = p;
	m_f2_distance  = dist;
	m_f2_rotation  = 
		osg::Quat( pitch * ToRad,       osg::Vec3(0.1,0.0,0.0)) 
			* 
		osg::Quat( fixed_yaw * ToRad,   osg::Vec3(0.0,0.0,1.0));

	m_request_fly = true;
}

