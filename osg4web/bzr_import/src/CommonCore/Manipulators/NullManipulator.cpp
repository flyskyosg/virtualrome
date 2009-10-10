

#include <CommonCore/Manipulators/NullManipulator.h>

#include <osg/BoundsChecking>


using namespace Manipulators;

NullManipulator::NullManipulator() : 
	_modelScale(0.01f),
	_distance(1.0f)
{

}


NullManipulator::~NullManipulator()
{
}


void NullManipulator::setNode(osg::Node* node)
{
    _node = node;
    if (_node.get())
    {
        const osg::BoundingSphere& boundingSphere=_node->getBound();
        _modelScale = boundingSphere._radius;
    }
    if (getAutoComputeHomePosition()) computeHomePosition();
}


const osg::Node* NullManipulator::getNode() const
{
    return _node.get();
}


osg::Node* NullManipulator::getNode()
{
    return _node.get();
}


void NullManipulator::home(double /*currentTime*/)
{
    if (getAutoComputeHomePosition()) 
		computeHomePosition();

    computePosition(_homeEye, _homeCenter, _homeUp);
}

void NullManipulator::home(const osgGA::GUIEventAdapter& ea , osgGA::GUIActionAdapter& us)
{
    home(ea.getTime());
    us.requestRedraw();
    us.requestContinuousUpdate(false);
}


void NullManipulator::getUsage(osg::ApplicationUsage& usage) const
{
    usage.addKeyboardMouseBinding("NullManipulator","Nothing to do!!!");
}


void NullManipulator::setByMatrix(const osg::Matrixd& matrix)
{
    _center = osg::Vec3(0.0f,0.0f,-_distance)*matrix;
    _rotation = matrix.getRotate();
}

osg::Matrixd NullManipulator::getMatrix() const
{
    return osg::Matrixd::translate(0.0,0.0,_distance)*osg::Matrixd::rotate(_rotation)*osg::Matrixd::translate(_center);
}

osg::Matrixd NullManipulator::getInverseMatrix() const
{
    return osg::Matrixd::translate(-_center)*osg::Matrixd::rotate(_rotation.inverse())*osg::Matrixd::translate(0.0,0.0,-_distance);
}

void NullManipulator::computePosition(const osg::Vec3& eye,const osg::Vec3& center,const osg::Vec3& up)
{

    osg::Vec3 lv(center-eye);

    osg::Vec3 f(lv);
    f.normalize();
    osg::Vec3 s(f^up);
    s.normalize();
    osg::Vec3 u(s^f);
    u.normalize();
    
    osg::Matrix rotation_matrix(s[0],     u[0],     -f[0],     0.0f,
                                s[1],     u[1],     -f[1],     0.0f,
                                s[2],     u[2],     -f[2],     0.0f,
                                0.0f,     0.0f,     0.0f,      1.0f);
                   
    _center = center;
    _distance = lv.length() * 2.0;
    _rotation = rotation_matrix.getRotate().inverse();
}


bool NullManipulator::handle(const osgGA::GUIEventAdapter& ea,osgGA::GUIActionAdapter& us)
{
	if (ea.getHandled()) return false;

	switch(ea.getEventType())
    {
	case(osgGA::GUIEventAdapter::KEYDOWN):
		{
			if (ea.getKey()== osgGA::GUIEventAdapter::KEY_Escape)
            {
				//Nothing to do... Avoid the viewer closing
                return true;
            }
		}
			break;
		default:
            return false;
	}

	return false;
}