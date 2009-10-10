
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
					osg::Matrix matrix;
					osg::Matrix::value_type *pt,*pt_old,*pt_new;

					for( pt = matrix.ptr(), pt_old = _oldMatrix.ptr(), pt_new = _transitionMatrix.ptr(); pt < matrix.ptr() + 16; pt++,pt_old++,pt_new++) 
					{
						*pt = *pt_old * (1.0 - blend_factor) + blend_factor * *pt_new;
					}

//					osg::ref_ptr<osg::Camera> cam = _mainViewer->getCamera();
					_mainViewer->getCameraManipulator()->setByMatrix(matrix );
					//credo che sia il manipulator attivo che resetta la camera come gli pare

					
//					if(cam.valid())
//						cam->setViewMatrix( matrix ); //FIXME: tutto perfetto... ma se ne FOTTE... come tutte le impostazioni delle camere!!!!!!
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
		
//		_oldMatrix.set( _mainViewer->getCamera()->getViewMatrix() );
		_oldMatrix.set(_mainViewer->getCameraManipulator()->getMatrix() );
		_transitionMatrix.set( m1->getMatrix() );
		
		_startTime = osg::Timer::instance()->tick();
		
		_activateTransition = true;
	} 
	else
		return false;

	return true;
}
