
#include <CommonCore/SceneHandlers/AnimateViewHandler.h>

#include <osg/io_utils>

#include <osgDB/ReaderWriter>
#include <osgDB/Registry>



using namespace SceneHandlers;

AnimateViewHandler::AnimateViewHandler(osgViewer::Viewer* viewer) : CommandSchedule("ANIMATEVIEW"), 
	_mainViewer(viewer),
	_options(new osgDB::ReaderWriter::Options),
	_activateTransition(false),
	_animationTime(1800.0)
{
	this->setCommandAction( "GET_MATRIX" );
	this->setCommandAction( "SET_MATRIX" );

	//TODO: set animation time
}
        
AnimateViewHandler::~AnimateViewHandler()
{
	_activateTransition = false;

	if(_mainViewer.valid())
		_mainViewer = NULL;

	if(_options.valid())
		_options = NULL;
}

bool AnimateViewHandler::handle(const osgGA::GUIEventAdapter& ea,osgGA::GUIActionAdapter& aa)
{
	if(_activateTransition)
	{
		switch(ea.getEventType())
		{
		    case(osgGA::GUIEventAdapter::FRAME):
		    {
				double ms = osg::Timer::instance()->delta_m( _startTime, osg::Timer::instance()->tick() );

				if(ms < _animationTime) 
				{
					double blend_factor = ms / _animationTime;
					osg::Matrix matrix = _mainViewer->getCameraManipulator()->getMatrix();
					
					osg::Quat rotation;
					osg::Vec3 position, scale;

					osg::Vec3 p1, p2;

					p1 = matrix.getTrans();
					p2 = _transitionMatrix.getTrans();

					position = ( p1 * (1.0 - blend_factor) + p2 * blend_factor);
					scale = matrix.getScale();
					rotation.slerp(blend_factor, matrix.getRotate(), _transitionMatrix.getRotate()); //TODO: check

					matrix.set(osg::Matrix::rotate(rotation)*osg::Matrix::scale(scale)*osg::Matrix::translate(position));

					_mainViewer->getCameraManipulator()->setByMatrix(matrix );
				}
				else
				{
					_oldMatrix.set(osg::Matrix::identity());
					_transitionMatrix.set(osg::Matrix::identity());
					_activateTransition = false;
				}
				
				return false;
			}    
			default:
			    return false;
		}
	}

	return false;
}

/** Ridefinisco la funzione che gestisce i comandi */
std::string AnimateViewHandler::handleAction(std::string argument)
{
	std::string retstr = "CORE_DONE";
	std::string lcommand, rcommand;

	this->splitActionCommand(argument, lcommand, rcommand);
	
	switch(this->getCommandActionIndex(lcommand))
	{
	case GET_MATRIX:
		retstr = this->getViewMatrix();
		break;
	default:	
		if(!this->setViewMatrix(rcommand))
			retstr = "STREAM_ERROR";
		break;
	}
		
	return retstr;
}

std::string AnimateViewHandler::getViewMatrix()
{
	std::stringstream s;
	std::string retstring = "STREAM_ERROR";

	osg::ref_ptr<osg::MatrixTransform> m = new osg::MatrixTransform( _mainViewer->getCameraManipulator()->getMatrix() );
  
	osg::ref_ptr<osgDB::ReaderWriter> writer = osgDB::Registry::instance()->getReaderWriterForExtension(std::string("osg"));
	
	if( !writer.valid() ) 
		return retstring;
	
	osgDB::ReaderWriter::WriteResult res = writer->writeObject(*m, s, _options.get());
    
	if( res.success() )
		retstring = s.str();
	
	return retstring;
}


bool AnimateViewHandler::setViewMatrix( std::string viewstring )
{
	std::stringstream s(viewstring);
    
    osg::ref_ptr<osgDB::ReaderWriter> reader = osgDB::Registry::instance()->getReaderWriterForExtension(std::string("osg"));
	
	if( !reader.valid() ) 
		return false;
	
	osgDB::ReaderWriter::ReadResult res = reader->readObject( s, _options.get() );
	
	if(res.validObject()) 
	{
		osg::ref_ptr<osg::MatrixTransform> m1 = static_cast<osg::MatrixTransform *> (res.getObject());

		_oldMatrix.set(_mainViewer->getCameraManipulator()->getMatrix() );
		_transitionMatrix.set( m1->getMatrix() );
		
		_startTime = osg::Timer::instance()->tick();
		
		_activateTransition = true;
	} 
	else
		return false;

	return true;
}
