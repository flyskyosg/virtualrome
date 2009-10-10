/*=========================================================================
Program:   Multimod Application Framework
Module:    $RCSfile: walkManipulator.cpp,v $
Language:  C++
Date:      $Date: 2007/12/17 17:26:24 $
Version:   $Revision: 1.5 $
Authors:   Tiziano Diamanti
==========================================================================
Copyright (c) 2001/2005 
CINECA - Interuniversity Consortium (www.cineca.it)
=========================================================================*/

//#include "mafDefines.h" ---- non serve - non stiamo usando il maf
//----------------------------------------------------------------------------
// NOTE: Every CPP file in the MAF must include "mafDefines.h" as first.
// This force to include Window,wxWidgets and VTK exactly in this order.
// Failing in doing this will result in a run-time error saying:
// "Failure#0: The value of ESP was not properly saved across a function call"
//----------------------------------------------------------------------------

#include <fstream>

#include <CommonCore/Manipulators/walkManipulator.h>

#include <osg/Quat>
#include <osg/Node>
#include <osg/Group>
#include <osg/Notify>
#include <osg/io_utils>
#include <osgUtil/IntersectVisitor>
#include <osg/Geode>


using namespace osg;
using namespace osgGA;

using namespace Manipulators;

//--------------------------------------------------------------------
// const
//--------------------------------------------------------------------
double ToRad = osg::PI / 180.0; 


//--------------------------------------------------------------------
walkManipulator::walkManipulator() : CommandSchedule("WALK"),
	_yaw( 0.0f ),
   _pitch( 0.0f ),
   _roll( 0.0f ),
   _position( osg::Vec3d(0,0,0) ),
   _ground_distance( 1 ),
   _step_h( 1 ),
   _step_v( 1 ),
   _rstep( 1.5 ),
   _default_pos_set( false ),
   _H_Speed( 0.0f ),
   _V_Speed( 0.0f ),
   _going_left( 0 ),
   _going_up( 0 ),
   _calc_ground_distance( false ),
   _ground_collision_on( true ),
   _segmult1(4.0),
   _segmult2(3.0),
#if _USE_VISMAN_
   _visman( NULL ),
   _viewer( NULL ),
   _picker( NULL ),
#endif
   _keep_on_walking( false )
//--------------------------------------------------------------------
{
   _keep_on_rotating_left = _keep_on_rotating_right = _keep_on_rotating_up = _keep_on_rotating_down = false;

	//Using: WALK_MANIP nomecomando
	this->setCommandAction("WALK_MANIP");
}

#if _USE_VISMAN_
//--------------------------------------------------------------------
void walkManipulator::assignVisman(vismanMain *vs)
//--------------------------------------------------------------------
{
  _visman = vs;
}
//--------------------------------------------------------------------
unsigned int walkManipulator::get_context_id(void)
//--------------------------------------------------------------------
{
  return _visman->get_context_id();
}
#endif
//--------------------------------------------------------------------
void walkManipulator::setViewer(osgViewer::Viewer *viewer, std::string db_prefix, std::string exp_prefix)
//--------------------------------------------------------------------
{
  _viewer = viewer;
  #if _USE_VISMAN_
  _picker = new vismanPicker(_viewer, db_prefix, exp_prefix);
  if (_visman->getNodeScanner()) _picker->AssignNodeScanner(_visman->getNodeScanner());
  #endif
}
//--------------------------------------------------------------------
walkManipulator::~walkManipulator()
//--------------------------------------------------------------------
{
  stop_rotating();
  stop_walking();
}
//--------------------------------------------------------------------
void walkManipulator::setByMatrix(const osg::Matrixd& matrix)
//--------------------------------------------------------------------
{
 	osg::Quat q = matrix.getRotate();
    
	double qx,qy,qz,qw;

	qx = q.x();
	qy = q.y();
	qz = q.z();
	qw = q.w();
   
	_yaw = osg::RadiansToDegrees( atan2( 2*qy*qw-2*qx*qz , 1 - 2*qy*qy - 2*qz*qz) );
	_pitch = osg::RadiansToDegrees( asin(2*qx*qy + 2*qz*qw) );
	_roll = osg::RadiansToDegrees( atan2(2*qx*qw-2*qy*qz , 1 - 2*qx*qz - 2*qz*qz) );

	_position = matrix.getTrans(); 

    IntersectTerrain();
}
//--------------------------------------------------------------------
osg::Matrixd walkManipulator::getMatrix() const
//--------------------------------------------------------------------
{
	double heading, attitude, bank;
	heading = osg::DegreesToRadians(_yaw);
	attitude = osg::DegreesToRadians(_pitch);
	bank = osg::DegreesToRadians(_roll);

	double c1 = cos(heading/2);    
	double s1 = sin(heading/2);    
	double c2 = cos(attitude/2);    
	double s2 = sin(attitude/2);    
	double c3 = cos(bank/2);    
	double s3 = sin(bank/2);
    double c1c2 = c1*c2;    
	double s1s2 = s1*s2;    
	
	osg::Quat rot( c1c2*s3 + s1s2*c3, s1*c2*c3 + c1*s2*s3, c1*s2*c3 - s1*c2*s3, c1c2*c3 - s1s2*s3);

  return 
    osg::Matrixd::rotate(rot)* 
    osg::Matrix::translate(_position); 
}
//--------------------------------------------------------------------
osg::Matrixd walkManipulator::getInverseMatrix() const
//--------------------------------------------------------------------
{
  osg::Quat rot = 
    osg::Quat( (_pitch+90)*ToRad, osg::Vec3(1.0,0.0,0.0)) *
    osg::Quat((_roll)*ToRad, osg::Vec3(0.0,1.0,0.0)) *
    osg::Quat( (_yaw)*ToRad, osg::Vec3(0.0,0.0,1.0));
  
  return
    osg::Matrix::translate(-_position) *
    osg::Matrixd::rotate(rot.inverse());
}
//--------------------------------------------------------------------
void walkManipulator::setNode(osg::Node* node)
//--------------------------------------------------------------------
{
    _node = node;
}
//--------------------------------------------------------------------
void walkManipulator::searchDefaultPos()
//--------------------------------------------------------------------
{
  double radius;
  CameraData C;

  if( !getNode()) return;

  radius = getNode()->getBound().radius();

  _position = getNode()->getBound().center();
  _yaw = 0;
  _pitch = -12;
  _roll = 0;
  _ground_distance = calcGroundDistance();
  C.Camera_X = _position.x();
  C.Camera_Y = _position.y() - 3.0 * radius;
  C.Camera_Z = _position.z();
  C.Camera_Yaw = _yaw;
  C.Camera_Pitch = _pitch;
  C.Camera_Roll = 0.0f;
  setPosition(C, false);
  setStepAmount(radius / 10.0f, radius / 10.0f);

  IntersectTerrain();
}
//--------------------------------------------------------------------
void walkManipulator::searchPosUsingSubNode(osg::Node* subnode, double ydistfromcenter)
//--------------------------------------------------------------------
{
  double radius;
  CameraData C;

  if( subnode == NULL)
  {
	  this->searchDefaultPos();
	  return;
  }
 
  radius = subnode->getBound().radius();

  _position = subnode->getBound().center();
  _yaw = 0;
  _pitch = -12;
  _roll = 0;
  _ground_distance = calcGroundDistance();
  C.Camera_X = _position.x();
  C.Camera_Y = _position.y() - ydistfromcenter * radius;
  C.Camera_Z = _position.z();
  C.Camera_Yaw = _yaw;
  C.Camera_Pitch = _pitch;
  C.Camera_Roll = 0.0f;
  setPosition(C, false);
  setStepAmount(radius / 10.0f, radius / 10.0f);

  IntersectTerrain();
}
//--------------------------------------------------------------------
bool walkManipulator::handle(const GUIEventAdapter& ea,GUIActionAdapter& us)
//--------------------------------------------------------------------
{
  #if _USE_VISMAN_
    int _node_type;
  #endif
  std::string _num_node;
  unsigned int button_mask;

  switch(ea.getEventType())
    {
        case(GUIEventAdapter::PUSH): //Mouse Button Down
          _x0 = ea.getXnormalized();
          _y0 = ea.getYnormalized();
          if( ea.getButton() == GUIEventAdapter::MIDDLE_MOUSE_BUTTON)
            flytoScenePick(ea.getXnormalized(), ea.getYnormalized());
                                                  //Nota : -- non arrivano MOUSE_DRAG se il muose e' fermo
                                                  //          neanche con us.RequestContinousUpdate(true);
                                                  //          Pero' arrivano le FRAME ...
          if( ea.getButton() == GUIEventAdapter::LEFT_MOUSE_BUTTON)
            go_upward(false);
          #if _USE_VISMAN_
            _visman->setFocusOnGLCanvas();
          #endif
        break;

        #if _USE_VISMAN_
          case(GUIEventAdapter::RELEASE): //Mouse Button Up
            button_mask = ea.getButton();
            if (button_mask == GUIEventAdapter::LEFT_MOUSE_BUTTON)
            {
              stop_walking();
              if (_picker)
              {
                _num_node = _picker->pick(ea.getXnormalized(), ea.getYnormalized(), &_node_type);
                if ((_num_node.compare("") !=0) && (_node_type == NODE_TYPE_PVIEW))
                {
                  if (_visman)
                  {
                    if (_visman->isDB_Enabled())
                      _visman->increase_db_level();
                    _visman->getNodeScanner()->setPViewOn(atoi(_num_node.c_str()), this);
                    _visman->move_lights();
                    _visman->_gui_needs_update = true;
                    if (_visman->is_stereo_enabled())
                      _visman->change_stereo_iod(_visman->getNodeScanner()->PViewList.at(atoi(_num_node.c_str()))->get_stereo_iod());
                  }
                }
                else if ((_num_node.compare("") !=0) && (_node_type == NODE_TYPE_DB))
                {
                  if (_visman)
                    if (_visman->isDB_Enabled())
                      _visman->OpenDBForm(_num_node);
                }
                else if ((_num_node.compare("") !=0) && (_node_type == NODE_TYPE_EXP))
                  if (_visman)
                    if (_visman->isExp_Enabled())
                      _visman->OpenEXPForm(_num_node);
              }
            }
          break;
        #endif

        case(GUIEventAdapter::DRAG): // mouse move with buttons pressed
          {        
            button_mask = ea.getButtonMask();
            if( button_mask == GUIEventAdapter::RIGHT_MOUSE_BUTTON )
            {
              double dx = ea.getXnormalized() - _x0;
              double dy = ea.getYnormalized() - _y0;

              rotateHead(20 * _rstep * dx, 20 * _rstep * dy);
              
              _x0 = ea.getXnormalized();
              _y0 = ea.getYnormalized();
            }
          }
        break;

        #if _USE_VISMAN_
          case(GUIEventAdapter::MOVE): // mouse move without buttons pressed
            if (_picker)
              _num_node = _picker->pick(ea.getXnormalized(), ea.getYnormalized(), &_node_type);
            if ((_num_node.compare("") !=0) && (_node_type == NODE_TYPE_EXP))
            {
                if (_visman)
                  if (_visman->isExp_Enabled())
                  {
                    _visman->OpenEXPLabel(_num_node, ea.getXnormalized(), ea.getYnormalized());
                  }
            }
            else if ((_num_node.compare("") !=0) && (_node_type == NODE_TYPE_PVIEW) && !_visman->getNodeScanner()->isPViewOn())
            {
              _visman->getGLCanvas()->SetCursorMagnifier();
            }
            else
              if (_visman)
              {
                _visman->getGLCanvas()->DisableEXPLabel();
              }
          break;
       #endif

          case(GUIEventAdapter::KEYDOWN):
          {
              switch(ea.getKey()) 
                {
                case 'a':  // SPACE = reset camera to searchDefaultPos position
                  {
                    searchDefaultPos();
                  }
                break;

                case GUIEventAdapter::KEY_Left: //turn left if shift pressed
                  if (ea.getModKeyMask() & GUIEventAdapter::MODKEY_SHIFT)
                  {
                    _yaw += 3 * _rstep;
                    while( _yaw >= 360 ) _yaw -= 360;
                  }
                  else                          // strafe left otherwise
                  {
                    go_left(false);
                  }
                break;

                case GUIEventAdapter::KEY_Right:  //turn right if shift pressed
                  if (ea.getModKeyMask() & GUIEventAdapter::MODKEY_SHIFT)
                  {
                    _yaw -= 3 * _rstep;
                    while( _yaw < 0 ) _yaw += 360;
                  }
                  else                          // strafe right otherwise
                  {
                    go_right(false);
                  }
                break;

                case GUIEventAdapter::KEY_Up: // step forward
                //case 'w': //-- w gia usato da 'wireframe'
                //case 'W':
                  {
                    go_upward(false);
                  }
                break;

                case GUIEventAdapter::KEY_Down: // step backward
                //case 's': //-- s gia usato da 'statistics'
                //case 'S':
                  {
                    go_backward(false);
                  }
                break;

              #if _USE_VISMAN_
                case GUIEventAdapter::KEY_F1:
                {
                  if (_visman->getNodeScanner()) _visman->getNodeScanner()->SwitchList.at(0)->setSwitchValFade(0);
                } 
                break;

                case GUIEventAdapter::KEY_F2:
                  {
                    if (_visman->getNodeScanner()) _visman->getNodeScanner()->SwitchList.at(0)->setSwitchValFade(1);
                  }
                break;

                case GUIEventAdapter::KEY_F3:
                  {
                    if (_visman->getNodeScanner()) _visman->getNodeScanner()->SwitchList.at(0)->setSwitchValFade(2);
                  }
                break;

                case GUIEventAdapter::KEY_F4:
                  {
                    if (_visman->getNodeScanner()) _visman->getNodeScanner()->SwitchList.at(0)->setSwitchValFade(3);
                  }
                break;

                case GUIEventAdapter::KEY_F5:
                  {
                    if (_visman->getNodeScanner()) _visman->getNodeScanner()->SwitchList.at(0)->setSwitchValFade(4);
                  }
                break;

                case GUIEventAdapter::KEY_KP_Add:
                  {
                    if (_visman->getNodeScanner())
                    {
                      int curr_pos = _visman->getNodeScanner()->StoryList.at(0)->getCurrentPos();
                      _visman->getNodeScanner()->StoryList.at(0)->GotoPos(curr_pos + 1);
                    }
                  }
                break;

                case GUIEventAdapter::KEY_KP_Subtract:
                  {
                    if (_visman->getNodeScanner())
                    {
                      int curr_pos = _visman->getNodeScanner()->StoryList.at(0)->getCurrentPos();
                      _visman->getNodeScanner()->StoryList.at(0)->GotoPos(curr_pos - 1);
                    }
                  }
                break;

                case GUIEventAdapter::KEY_Space:
                if (_visman->getNodeScanner()->isPViewOn())
                {
                  _visman->getNodeScanner()->setPViewOff(this);
                  //setNode(_visman->getNodeScanner()->getPViewSwitch()->getChild(0));
                  if (_visman)
                  {
                    if (_visman->isDB_Enabled())
                      _visman->decrease_db_level();
                    _visman->_gui_needs_update = true;
                    _visman->change_stereo_iod(_visman->get_stereo_iod());
                  }
                }
              break;

            #endif

              case GUIEventAdapter::KEY_Page_Up:  // elevator up
                {
                  go_lift(false);
                }
              break;

              case GUIEventAdapter::KEY_Page_Down: // elevator down
                {
                  go_lower(false);
                }
              break;

              default:
                return false; // event not handled
              break;
            }
        }
        break;

        case(GUIEventAdapter::FRAME): //chiamato ad ogni Render
          {
            handleRotation();
            if (_calc_ground_distance)
            {
              _ground_distance = calcGroundDistance();
              _calc_ground_distance = false;
            }
            handleWalk();
          }
        break;

        default:
          return false; // event not handled
    }
    return true; // event handled
}

//--------------------------------------------------------------------
void walkManipulator::setGroundCollisionOnOff(bool on)
//--------------------------------------------------------------------
{
  _ground_collision_on = on;
}
//--------------------------------------------------------------------
// verifica che il center sia sul terreno
// eventualmente lo sposta 
void walkManipulator::IntersectTerrain()
//--------------------------------------------------------------------
{
  double _new_ground_distance; 
  if (!_ground_collision_on) return;
  if(!getNode()) return;

  _new_ground_distance = calcGroundDistance();
  if ((_new_ground_distance != 0.0) && (_ground_distance != _new_ground_distance))
  {
    _position[2] += _ground_distance - _new_ground_distance;
  }
}

//--------------------------------------------------------------------
void walkManipulator::adjustGroundDistance(void)
//--------------------------------------------------------------------
{
  _ground_distance = calcGroundDistance();
}

//--------------------------------------------------------------------
double walkManipulator::calcGroundDistance(void)
//--------------------------------------------------------------------
{
  osg::Vec3d intersection, p1, p2;
  double d = _ground_distance;
  
  p1 = _position;
  p1[2] -= d * 10;
  p2 = _position;

  bool found = Intersect( p1, p2, intersection );
  if(!found)
  {
    d = getNode()->getBound().radius(); 
    p1 = _position;
    p1[2] -= d;
    found = Intersect( p1, p2, intersection );
  }
  if(!found)
  {
    d *= 10;  // sono finito a meno infinito ?
    p1 = _position;
    p1[2] -= d;
    found = Intersect( p1, p2, intersection );
  }

  if (found)
    return _position[2] - intersection[2];
  else
    return 0.0;
}

//--------------------------------------------------------------------
bool walkManipulator::Intersect( osg::Vec3d p1, osg::Vec3d p2, osg::Vec3d& result)
//--------------------------------------------------------------------
{
  if( !getNode() ) return false;
  bool found = false;

  osg::ref_ptr<osg::LineSegment> seg = new osg::LineSegment;
  seg->set(p1,p2);

  osgUtil::IntersectVisitor iv;
  iv.setTraversalMode(osgUtil::IntersectVisitor::TRAVERSE_ACTIVE_CHILDREN); //cosi attraversa solo i figli visibili (e quindi caricati)
  iv.addLineSegment(seg.get());
  getNode()->accept(iv);
  if (iv.hits())
  {
    osgUtil::IntersectVisitor::HitList& hitList = iv.getHitList(seg.get());
    if (!hitList.empty())
    {
      for(osgUtil::IntersectVisitor::HitList::iterator hitr=hitList.begin();  hitr!=hitList.end();  ++hitr )
      {
        if( hitr->_geode.valid() )
        {
          osg::Vec3 p= hitr->getWorldIntersectPoint();
          if(!found) 
            result = p;
          else
            if(result[2]<p[2]) // cosi ritorno l'intersezione con la z maggiore
              result = p;
          found=true; 
        }
      }
    }
  }
  return found;
}

//--------------------------------------------------------------------
void walkManipulator::moveHorizzontally(double amount, bool move_forward, bool move_left)
//--------------------------------------------------------------------
{
  osg::Vec3d _newpos, _intersect_pos, _collision_result;
  double _long_amout = _segmult1 * amount; //4.0
  double _z_distance = - _segmult2 * _step_v; //3.0
  
  // the intersection is calculated forward and lower
  if (move_forward)
  {
    if (move_left)
      _intersect_pos = calcNextPosition(-_long_amout , 0.0f, _z_distance);
    else
      _intersect_pos = calcNextPosition(0.0f, _long_amout, _z_distance);
  }
  else
  {
    if (move_left)
      _intersect_pos = calcNextPosition(_long_amout, 0.0f, _z_distance);
    else
      _intersect_pos = calcNextPosition(0.0f, -_long_amout, _z_distance);
  }

  // if the step intersects anything, cannot move
  if (Intersect(_position, _intersect_pos, _collision_result))
  {
    _H_Speed = 0.0f;
    _going_left = 0;
    _going_up = 0;
    return;
  }

  if (move_forward)
  {
    if (move_left)
      _newpos = calcNextPosition(-amount, 0.0f, 0.0f);
    else
      _newpos = calcNextPosition(0.0f, amount, 0.0f);
  }
  else
  {
    if (move_left)
      _newpos = calcNextPosition(amount, 0.0f, 0.0f);
    else
      _newpos = calcNextPosition(0.0f, -amount, 0.0f);
  }

  _position[0] = _newpos[0];
  _position[1] = _newpos[1];
  IntersectTerrain();
}

//--------------------------------------------------------------------
void walkManipulator::moveVertically(double amount)
//--------------------------------------------------------------------
{
  osg::Vec3d _newpos, _collision_result;

  _newpos = calcNextPosition(0.0f, 0.0f, amount);
  
  // if the step intersects anything, cannot move
  if (Intersect(_position, _newpos, _collision_result))
  {
    _V_Speed = 0.0f;
    return;
  }

  _ground_distance += amount;
  if (!_ground_collision_on)
  {
    _position[2] += amount;
    return;
  }
  else
    IntersectTerrain();
}

//--------------------------------------------------------------------
osg::Vec3d walkManipulator::calcNextPosition(double dist_x, double dist_y, double dist_z)
//--------------------------------------------------------------------
{
  osg::Vec3d _newpos;
  double _sin, _cos;

  _sin = sin(_yaw*ToRad);
  _cos = cos(_yaw*ToRad);
  _newpos[0] = _position[0] + dist_x * _cos - dist_y * _sin;
  _newpos[1] = _position[1] + dist_x * _sin + dist_y * _cos;
  _newpos[2] = _position[2] + dist_z;

  return _newpos;
}

//--------------------------------------------------------------------
void walkManipulator::setPosition(CameraData Point, bool default_pos)
//--------------------------------------------------------------------
{
  _position[0] = Point.Camera_X;
  _position[1] = Point.Camera_Y;
  _position[2] = Point.Camera_Z;
  _yaw = Point.Camera_Yaw;
  _pitch = Point.Camera_Pitch;
  _roll = Point.Camera_Roll;
  //rotateHead(Point.Camera_Yaw, Point.Camera_Pitch);

  if (default_pos)
  {
    _initial_position[0] = Point.Camera_X;
    _initial_position[1] = Point.Camera_Y;
    _initial_position[2] = Point.Camera_Z;
    _initial_angles[0] = Point.Camera_Yaw;
    _initial_angles[1] = Point.Camera_Pitch;
    _initial_angles[2] = Point.Camera_Roll;
    _default_pos_set = true;
  }
  _ground_distance = calcGroundDistance();
  IntersectTerrain();
}

//--------------------------------------------------------------------
void walkManipulator::getPosition(CameraData *Point)
//--------------------------------------------------------------------
{
  Point->Camera_X = _position[0];
  Point->Camera_Y = _position[1];
  Point->Camera_Z = _position[2];
  Point->Camera_Yaw = _yaw;
  Point->Camera_Pitch = _pitch;
  Point->Camera_Roll = _roll;
}

//--------------------------------------------------------------------
void walkManipulator::rotateHead(double yaw, double pitch)
//--------------------------------------------------------------------
{
  if (yaw != 0.0f)
  {
    _yaw -= yaw;
    while( _yaw >= 360 ) _yaw -= 360;
    while( _yaw <  0   ) _yaw += 360; 
  }
  if (pitch != 0.0f)
    _pitch = osg::clampBetween(_pitch + pitch, -90.0, 90.0);
}

//--------------------------------------------------------------------
void walkManipulator::setStepAmount(double step_h, double step_v)
//--------------------------------------------------------------------
{
  _step_h = step_h;
  _step_v = step_v;
}

//--------------------------------------------------------------------
void walkManipulator::accelerateHor(double value)
//--------------------------------------------------------------------
{
  _H_Speed += value;
  if (_H_Speed > _step_h) _H_Speed = _step_h;
  if (_H_Speed < -_step_h) _H_Speed = -_step_h;
}

//--------------------------------------------------------------------
void walkManipulator::accelerateVer(double value)
//--------------------------------------------------------------------
{
  _V_Speed += value;
  if (_V_Speed > _step_v) _V_Speed = _step_v;
  if (_V_Speed < -_step_v) _V_Speed = -_step_v;
}

//--------------------------------------------------------------------
void walkManipulator::handleWalk(void)
//--------------------------------------------------------------------
{
  // Horizontal speed
  if (_H_Speed != 0.0f) 
  {
    if (_going_left > 0)
      moveHorizzontally(_H_Speed, true, true);
    else if (_going_left < 0)
      moveHorizzontally(_H_Speed, false, true);
    if (_going_up > 0)
      moveHorizzontally(_H_Speed, true, false);
    else if (_going_up < 0)
      moveHorizzontally(_H_Speed, false, false);
    
    if (_H_Speed > 0.0f)
    {
      if (!_keep_on_walking)
        _H_Speed -= _step_h / _walk_smoothing_factor;
      if (_H_Speed < _step_h / _walk_smoothing_factor) 
      {
        _H_Speed = 0.0f;
        _going_left = 0;
        _going_up = 0;
      }
    }
    else
    {
      if (!_keep_on_walking)
        _H_Speed += _step_h / _walk_smoothing_factor;
      if (_H_Speed > -_step_h / _walk_smoothing_factor)
      {
        _H_Speed = 0.0f;
        _going_left = 0;
        _going_up = 0;
      }
    }
  }

  // Vertical speed
  if (_V_Speed != 0.0f) 
  {
    moveVertically(_V_Speed);

    if (_V_Speed > 0.0f)
    {
      if (!_keep_on_walking)
        _V_Speed -= _step_v / _walk_smoothing_factor;
      if (_V_Speed < _step_v / _walk_smoothing_factor) 
        _H_Speed = 0.0f;
    }
    else
    {
      if (!_keep_on_walking)
        _V_Speed += _step_v / _walk_smoothing_factor;
      if (_V_Speed > -_step_v / _walk_smoothing_factor)
        _V_Speed = 0.0f;
    }
  }
}

//--------------------------------------------------------------------
void walkManipulator::Render(void)
//--------------------------------------------------------------------
{
  // wait for all cull and draw threads to complete.
  //_viewer->sync();

  // update the scene by traversing it with the the update visitor which will
  // call all node update callbacks and animations.
  //_viewer->update();

  // fire off the cull and draw traversals of the scene.
  _viewer->frame();
}
//--------------------------------------------------------------------
void walkManipulator::flytoScenePick(float mouse_x, float mouse_y)
//--------------------------------------------------------------------
{
  //osgUtil::IntersectVisitor::HitList hlist;
  osgUtil::LineSegmentIntersector::Intersections hlist;
  osg::LineSegment *seg;
  osg::Matrixd matrix;

  if (!_viewer) return;

  if (_viewer->computeIntersections(mouse_x, mouse_y, hlist))
  {
    for(osgUtil::LineSegmentIntersector::Intersections::iterator hitr=hlist.begin();
      hitr!=hlist.end();
      ++hitr)
      //if (hitr->drawable->getParent(0)->valid())
      {
        osg::Vec3 p = hitr->getWorldIntersectPoint();
        seg = new osg::LineSegment(p, _position);
        matrix.makeScale(0.8, 0.8, 1.0);
        // shorten the segment
        seg->mult(*seg, matrix);
        // the new position is the shortened start
        _position = seg->start();
        _calc_ground_distance = true;
        return;
      }
  }
}

//--------------------------------------------------------------------
osg::Vec3d walkManipulator::getPosition(void)
//--------------------------------------------------------------------
{
  return _position;
}

//--------------------------------------------------------------------
void walkManipulator::getYawnPitch(double *yaw, double *pitch)
//--------------------------------------------------------------------
{
  *yaw = _yaw;
  *pitch = _pitch;
}

//--------------------------------------------------------------------
void walkManipulator::getSteps(double *step_h, double *step_v)
//--------------------------------------------------------------------
{
  *step_v = _step_v;
  *step_h = _step_h;
}

//--------------------------------------------------------------------
void walkManipulator::setGroundDistance(double gd)
//--------------------------------------------------------------------
{
  _ground_distance = gd;
}
//--------------------------------------------------------------------
void walkManipulator::go_left(bool keep_on)
//--------------------------------------------------------------------
{
  _going_left = 1;
  accelerateHor(_step_h);
  if (keep_on) _keep_on_walking = true;
}
//--------------------------------------------------------------------
void walkManipulator::go_right(bool keep_on)
//--------------------------------------------------------------------
{
  _going_left = -1;
  accelerateHor(_step_h);
  if (keep_on) _keep_on_walking = true;
}
//--------------------------------------------------------------------
void walkManipulator::go_upward(bool keep_on)
//--------------------------------------------------------------------
{
  _going_up = 1;
  accelerateHor(_step_h);
  if (keep_on) _keep_on_walking = true;
}
//--------------------------------------------------------------------
void walkManipulator::go_backward(bool keep_on)
//--------------------------------------------------------------------
{
  _going_up = -1;
  accelerateHor(_step_h);
  if (keep_on) _keep_on_walking = true;
}
//--------------------------------------------------------------------
void walkManipulator::stop_walking(void)
//--------------------------------------------------------------------
{
  _keep_on_walking = false;
} 
//--------------------------------------------------------------------
void walkManipulator::go_lift(bool keep_on)
//--------------------------------------------------------------------
{
  accelerateVer(_step_v);
  if (keep_on) _keep_on_walking = true;
}
//--------------------------------------------------------------------
void walkManipulator::go_lower(bool keep_on)
//--------------------------------------------------------------------
{
  accelerateVer(-_step_v);
  if (keep_on) _keep_on_walking = true;
}
//--------------------------------------------------------------------
void walkManipulator::rotate_left(bool keep_on)
//--------------------------------------------------------------------
{
  _yaw += _angle_step;
  _keep_on_rotating_left = keep_on;
}
//--------------------------------------------------------------------
void walkManipulator::rotate_right(bool keep_on)
//--------------------------------------------------------------------
{
  _yaw -= _angle_step;
  _keep_on_rotating_right = keep_on;
}
//--------------------------------------------------------------------
void walkManipulator::rotate_up(bool keep_on)
//--------------------------------------------------------------------
{
  _pitch -= _angle_step;
  _keep_on_rotating_down = keep_on;
}
//--------------------------------------------------------------------
void walkManipulator::rotate_down(bool keep_on)
//--------------------------------------------------------------------
{
  _pitch += _angle_step;
  _keep_on_rotating_up = keep_on;
}
//--------------------------------------------------------------------
void walkManipulator::stop_rotating(void)
//--------------------------------------------------------------------
{
  _keep_on_rotating_left = _keep_on_rotating_right = _keep_on_rotating_up = _keep_on_rotating_down = false;
}
//--------------------------------------------------------------------
void walkManipulator::handleRotation(void)
//--------------------------------------------------------------------
{
  if (_keep_on_rotating_left)
    _yaw += _angle_step;
  if (_keep_on_rotating_right)
    _yaw -= _angle_step;
  if (_keep_on_rotating_up)
    _pitch -= _angle_step;
  if (_keep_on_rotating_down)
    _pitch += _angle_step;
}
//--------------------------------------------------------------------
void walkManipulator::loopCameraPoints(bool *go_on, CameraPoints *CP)
//--------------------------------------------------------------------
{
  int i, j, num_points, num_steps/*, accel*/;
  CameraData Camera, NewCamera;
  osg::Vec3d p1, p2, r1, r2;
  bool angles_done;
  bool just_once;
  double value, angle_speed, angle_increment, angle_current_speed, increment_x, increment_y, increment_z;

  /*go_on =*/ just_once = true;
  angle_speed = 0.25f;

  num_points = CP->GetNumPoints();
  while (*go_on)
  {
    for (i = 0; i < num_points - 1; i++)
    {
      angles_done = false;
      angle_current_speed = angle_increment = 0.02f;
      //accel = 15;
      Camera = *CP->GetCameraPoint(i);
      p1.set(Camera.Camera_X, Camera.Camera_Y, Camera.Camera_Z);
      r1.set(Camera.Camera_Yaw, Camera.Camera_Pitch, Camera.Camera_Roll);
      Camera = *CP->GetCameraPoint(i + 1);
      p2.set(Camera.Camera_X, Camera.Camera_Y, Camera.Camera_Z);
      r2.set(Camera.Camera_Yaw, Camera.Camera_Pitch, Camera.Camera_Roll);
      num_steps = (int)(calcPointsDistance(p1, p2) / _step_h); //2.31 è la perdita dell'accelerazione
      if (num_steps == 0) num_steps = 1; 
      increment_x = (p2[0] - p1[0]) / num_steps;
      increment_y = (p2[1] - p1[1]) / num_steps;
      increment_z = (p2[2] - p1[2]) / num_steps;

      // porto la camera al punto notevole
      Camera = *CP->GetCameraPoint(i);
      setPosition(Camera, false);
      Render();
      for (j = num_steps /*+ floor (accel * 2 - (double)log((double)accel))*/; j > 0  ; j--)
      {
        if (!angles_done)
          Camera = *CP->GetCameraPoint(i + 1);

        getPosition(&NewCamera);
        while ((fabs(NewCamera.Camera_Yaw - Camera.Camera_Yaw) >= 0.5f) || (fabs(NewCamera.Camera_Pitch - Camera.Camera_Pitch) >= 0.5f) ||
               (fabs(NewCamera.Camera_Roll - Camera.Camera_Roll) >= 0.5f))
        {
          if (fabs(NewCamera.Camera_Yaw - Camera.Camera_Yaw) >= 0.5f)
          {
            value = Camera.Camera_Yaw;
            NewCamera.Camera_Yaw = calcNextAngle(NewCamera.Camera_Yaw, &value, angle_current_speed);
            Camera.Camera_Yaw = value;
            if (!(*go_on)) return;
          }
          if (fabs(NewCamera.Camera_Pitch - Camera.Camera_Pitch) >= 0.5f)
          {
            value = Camera.Camera_Pitch;
            NewCamera.Camera_Pitch = calcNextAngle(NewCamera.Camera_Pitch, &value, angle_current_speed);
            Camera.Camera_Pitch = value;
            if (!(*go_on)) return;
          }
          if (fabs(NewCamera.Camera_Roll - Camera.Camera_Roll) >= 0.5f)
          {
            value = Camera.Camera_Roll;
            NewCamera.Camera_Roll = calcNextAngle(NewCamera.Camera_Roll, &value, angle_current_speed);
            Camera.Camera_Roll = value;
            if (!(*go_on)) return;
          }
          setPosition(NewCamera, false);
          Render();
          if (angle_current_speed < angle_speed) angle_current_speed += angle_increment;
        }
        angles_done = true;
        Camera.Camera_X = NewCamera.Camera_X + increment_x/* / accel*/;
        Camera.Camera_Y = NewCamera.Camera_Y + increment_y/* / accel*/;
        Camera.Camera_Z = NewCamera.Camera_Z + increment_z/* / accel*/;
        /*if (accel > 1) accel--;
        if (j < 16) accel = 16 - j;*/
        setPosition(Camera, false);
        Render();
        if (!(*go_on)) return;
      }
    }
    if (just_once) return;
  }
}
//--------------------------------------------------------------------
double walkManipulator::calcPointsDistance(osg::Vec3d p1, osg::Vec3d p2)
//--------------------------------------------------------------------
{
  return sqrt((p1[0] - p2[0]) * (p1[0] - p2[0]) + (p1[1] - p2[1]) * (p1[1] - p2[1]) +
              (p1[2] - p2[2]) * (p1[2] - p2[2]));
}

double walkManipulator::calcNextAngle(double StartingAngle, double *phi, double step)
{
  float Angle;

  if (StartingAngle < 0.0f) StartingAngle += 360.0f;
  if (StartingAngle > 360.0f) StartingAngle -= 360.0f;
  if (*phi < 0.0f) *phi += 360.0f;
  if (*phi > 360.0f) *phi -= 360.0f;

  if (fabs(*phi - StartingAngle) < 180.0f)
  {
    if (StartingAngle < *phi)
      Angle = StartingAngle + step;
    else
      Angle = StartingAngle - step;
  }
  else
  {
    if (StartingAngle < *phi)
      Angle = StartingAngle - step;
    else
      Angle = StartingAngle + step;
  }
  return Angle;
}

//--------------------------------------------------------------------
void walkManipulator::moveLights(osg::Light **Sun, osg::LightSource **Sun_ls, float *scene_radius)
//--------------------------------------------------------------------
{
  BoundingSphere bs = getNode()->getBound();
  *scene_radius = bs.radius();

  Sun[0]->setPosition(Vec4(bs.center().x(), bs.center().y(), bs.center().z() + bs.radius(), 0.0f));
  Sun[1]->setPosition(Vec4(bs.center().x(), bs.center().y() + bs.radius(), bs.center().z() + bs.radius(), 0.0f));
  Sun[2]->setPosition(Vec4(bs.center().x(), bs.center().y() - bs.radius(), bs.center().z() + bs.radius(), 0.0f));
  Sun[3]->setPosition(Vec4(bs.center().x() + bs.radius(), bs.center().y(), bs.center().z() + bs.radius(), 0.0f));
  Sun[4]->setPosition(Vec4(bs.center().x() - bs.radius(), bs.center().y(), bs.center().z() + bs.radius(), 0.0f));
  for (int i = 0; i < NUM_LIGHTS; i++)
  {
    // directional light from above
    Sun[i]->setAmbient(Vec4(1.0f, 1.0f, 1.0f, 1.0f));
    Sun[i]->setDiffuse(Vec4(1.0f, 1.0f, 1.0f, 1.0f));
    Sun[i]->setSpecular(Vec4(1.0f, 1.0f, 1.0f, 0.0f));
    Sun[i]->setLinearAttenuation(2.0f/bs.radius());
    Sun[i]->setQuadraticAttenuation(100.0f/osg::square(bs.radius()));
    Sun_ls[i]->setLight(Sun[i]);
  }
}

/** Ridefinisco la funzione che gestisce i comandi */
std::string walkManipulator::handleAction(std::string argument)
{
	std::string retstr = "CORE_DONE";
	std::string lcommand, rcommand;

	this->splitActionCommand(argument, lcommand, rcommand);
	retstr = this->ExecCommand(lcommand, rcommand);
		
	return retstr;
}

std::string walkManipulator::ExecCommand(std::string lcommand, std::string rcommand)
{
  std::string retstr_done = "CORE_DONE";
	std::string retstr_fail = "CORE_FAILED";
	
	if ((lcommand.compare("STRAFE") == 0) && (rcommand.compare("LEFT") == 0))
		this->go_left(true);
	else if ((lcommand.compare("STRAFE") == 0) && (rcommand.compare("RIGHT") == 0))
		this->go_right(true);
	else if ((lcommand.compare("FORWARD") == 0) && (rcommand.compare("UP") == 0))
		this->go_upward(true);
	else if ((lcommand.compare("FORWARD") == 0)&& (rcommand.compare("DOWN") == 0))
		this->go_backward(true);
	else if ((lcommand.compare("YAW") == 0)&& (rcommand.compare("LEFT") == 0))
		this->rotate_left(true);
	else if ((lcommand.compare("YAW") == 0)&& (rcommand.compare("RIGHT") == 0))
		this->rotate_right(true);
	else if ((lcommand.compare("PITCH") == 0)&& (rcommand.compare("UP") == 0))
		this->rotate_up(true);
	else if ((lcommand.compare("PITCH") == 0)&& (rcommand.compare("DOWN") == 0))
		this->rotate_down(true);
	else if ((lcommand.compare("LIFT") == 0)&& (rcommand.compare("UP") == 0))
		this->go_lift(false);
	else if ((lcommand.compare("LIFT") == 0)&& (rcommand.compare("DOWN") == 0))
		this->go_lower(false);
  else if ((lcommand.compare("SET_GROUND_COLLISION") == 0)&& (rcommand.compare("ON") == 0))
    this->setGroundCollisionOnOff(true);
  else if ((lcommand.compare("SET_GROUND_COLLISION") == 0)&& (rcommand.compare("OFF") == 0))
    this->setGroundCollisionOnOff(false);
  else if (lcommand.compare("GOTO") == 0)
  {
    CameraData CP;
    if (sscanf(rcommand.c_str(), "%lf %lf %lf %lf %lf %lf", &CP.Camera_X, &CP.Camera_Y, &CP.Camera_Z, &CP.Camera_Yaw, &CP.Camera_Pitch, &CP.Camera_Roll) == 6)
    {
      this->setPosition(CP, true);
      this->adjustGroundDistance();
    }
    else
      return retstr_fail;
  }
  else if (lcommand.compare("SETSTEPS") == 0)
  {
    double step_h, step_v;
    if (sscanf(rcommand.c_str(), "%lf %lf", &step_h, &step_v) == 2)
      this->setStepAmount(step_h, step_v);
    else
      return retstr_fail;
  }
	else if (lcommand.compare("STOP") == 0)
	{
		this->stop_walking();
		this->stop_rotating();
	}
  else
    return retstr_fail;
	
	return retstr_done;
}