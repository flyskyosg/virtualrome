
#include <CommonCore/SceneHandlers/AnimateViewHandler.h>

#include <osg/io_utils>

#include <osgDB/ReaderWriter>
#include <osgDB/Registry>



using namespace SceneHandlers;

AnimateViewHandler::AnimateViewHandler(osgViewer::Viewer* viewer) : CommandSchedule("ANIMATEVIEW"), 
	_mainViewer(viewer),
	_options(new osgDB::ReaderWriter::Options),
	_activateTransition(false),
	_duringAnimation(false),
	_currentKey(0),
	_animationPathSmoothness(DEFAULT_ANIMATION_PATH_SMOOTHNESS),
	_sequenceTransitionMatrix(osg::Matrix::identity()),
	_prevTensor(osg::Matrix::identity()),
	_nextTensor(osg::Matrix::identity()),
	_transitionMatrix(osg::Matrix::identity()),
	_animationTime(DEFAULT_ANIMATION_TIME)
{
	//Default value
	this->setCommandAction( "UNKNOWN_ACTION" );
	this->setCommandAction( "GET_CURRENT_MATRIX" );
	this->setCommandAction( "GO_TO_MATRIX_DIRECTLY" );
	this->setCommandAction( "GET_ANIMATION_TIME" );
	this->setCommandAction( "GET_ANIMATION_PATH_SMOOTHNESS" ); 
	this->setCommandAction( "SET_ANIMATION_TIME" );
	this->setCommandAction( "SET_ANIMATION_KEY" );
	this->setCommandAction( "SET_ANIMATION_PATH_SMOOTHNESS" );
	this->setCommandAction( "START_ANIMATION" );
	this->setCommandAction( "CONTINUE_ANIMATION" );
	this->setCommandAction( "STOP_ANIMATION" );
	this->setCommandAction( "RESET_DEFAULT" );
}
        
AnimateViewHandler::~AnimateViewHandler()
{
	this->resetSettings();
	
	if(_mainViewer.valid())
		_mainViewer = NULL;

	if(_options.valid())
		_options = NULL;
}
#include <iostream>

bool AnimateViewHandler::doTransition(osg::Matrix animMatrix, double animTime)
{
	double ms = osg::Timer::instance()->delta_m( _startTime, osg::Timer::instance()->tick() );
	
	if( ms < animTime)
	{
		double blend_factor = ms / animTime;
		double rot_blend_factor;	//FIXME: trovare il modo fi anticipare la rotazione
				
		osg::Quat rotation;
		osg::Vec3 position, scale;

		osg::Vec3 p1,p2;

		osg::Matrix matrix;
		
		p1 = _oldMatrix.getTrans();
		p2 = animMatrix.getTrans();

		double Zfactor = 0.1f;		// Fattore di Alzata Parabolica, per i terreni mi sembra ok cosi'.

		double D = osg::Vec3(p1 - p2).length();
		D *= Zfactor;
		//osg::Vec3 p0,p3;
		//p0 = osg::Vec3(p1.x(),p1.y(),(p1.z()-D));
		//p3 = osg::Vec3(p2.x(),p2.y(),(p2.z()-D));
		double t,tmp;
		tmp = blend_factor - 1.0;

		//t = 1.0 - (tmp*tmp);
		//t = atan(blend_factor*5.0);

		//double h = 1.0 - (2.0*(t-0.5)*(t-0.5));
		
		if (!_duringAnimation){
			t = cos(tmp*tmp*osg::PI_2);
			position = ( p1 * (1.0 - t) + p2 * t);
			}
		else {
			t = 0.5 * (cos(tmp*osg::PI) + 1.0);

			osg::Vec3 p0,p3,A,B;

			p0 = _prevTensor.getTrans();
			p3 = _nextTensor.getTrans();
			A = p0 - p1;
			B = p3 - p2;
			A *= _animationPathSmoothness;
			B *= _animationPathSmoothness;

			#ifdef ANIMATION_PATH_SLOW_ON_KEYS
			position = interpolateCR(t, (p1+A),p1,p2,(p2+B));
			#else
			position = interpolateCR(blend_factor, (p1+A),p1,p2,(p2+B));
			#endif

			//position = interpolateCR(blend_factor, p0,p1,p2,p3);
			}

		if (!_duringAnimation){
			double h = cos((t-0.5)*osg::PI);
			position += osg::Vec3(0.0,0.0,h*D);

			rot_blend_factor = sqrt( t );			// Anticipa l'orientamento della camera rispetto al movimento camera
			//rot_blend_factor = t*t;				// Ritarda l'orientamento della camera rispetto al movimento camera
			}
		else rot_blend_factor = t;//t*t;

		scale = _oldMatrix.getScale();
		rotation.slerp(rot_blend_factor, _oldMatrix.getRotate(), animMatrix.getRotate());
		matrix.set(osg::Matrix::rotate(rotation)*osg::Matrix::scale(scale)*osg::Matrix::translate(position));

		_mainViewer->getCameraManipulator()->setByMatrix( matrix );

		return true;
	}
	else
		return false;
}

bool AnimateViewHandler::handle(const osgGA::GUIEventAdapter& ea,osgGA::GUIActionAdapter& aa)
{
	if(_activateTransition)
	{
		switch(ea.getEventType())
		{
		    case(osgGA::GUIEventAdapter::FRAME):
		    {
				if(_transitionMatrix != osg::Matrix::identity()) //Transizione diretta
				{
					if(!this->doTransition(_transitionMatrix, _animationTime))
					{
						_mainViewer->getCameraManipulator()->setByMatrix( _transitionMatrix );

						_oldMatrix.set(osg::Matrix::identity());
						_transitionMatrix.set(osg::Matrix::identity());
						_activateTransition = false;
					}
				}
				else
				{
					if(!this->doTransition(_sequenceTransitionMatrix, _sequenceAnimationTime))
					{
						_mainViewer->getCameraManipulator()->setByMatrix( _sequenceTransitionMatrix );

						//if(!_animationMatrixArray.empty() && !_animationTimeArray.empty())
						if(!_animationMatrixVector.empty() && !_animationTimeVector.empty() && _currentKey < (int)_animationMatrixVector.size())
						{
							/*
							_sequenceTransitionMatrix.set( _animationMatrixArray.front() );
							_sequenceAnimationTime = _animationTimeArray.front();

							_animationMatrixArray.pop();
							_animationTimeArray.pop();
							*/

							int prv = (_currentKey <= 1)? 0 : _currentKey-2;
							int nxt = ((_currentKey+1) >= (int)_animationMatrixVector.size())? (_animationMatrixVector.size()-1) : (_currentKey+1);
							//int prv = (_currentKey < 1)? 0 : _currentKey-1;
							//int nxt = ((_currentKey+2) >= (int)_animationMatrixVector.size())? (_animationMatrixVector.size()-1) : (_currentKey+2);

							_sequenceTransitionMatrix.set( _animationMatrixVector[_currentKey] );
							_sequenceAnimationTime = _animationTimeVector[_currentKey];

							//prv = _currentKey-1; nxt = (_currentKey+2);

							_prevTensor.set( _animationMatrixVector[prv] );
							_nextTensor.set( _animationMatrixVector[nxt] );

							_currentKey++;

							//Setto la posizione corrente della camera
							_oldMatrix.set(_mainViewer->getCameraManipulator()->getMatrix() );

							_startTime = osg::Timer::instance()->tick();
						}
						else
						{
							_oldMatrix.set(osg::Matrix::identity());
							_sequenceTransitionMatrix.set(osg::Matrix::identity());
							_activateTransition = false;
							_duringAnimation    = false;
						}
					}
				}
				
				return false;
			}
			case(osgGA::GUIEventAdapter::KEYDOWN):
			case(osgGA::GUIEventAdapter::KEYUP):
			case(osgGA::GUIEventAdapter::RELEASE):
		    {
				this->stopAnimation();
			}
				break;
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

	//FIXME: Finire la gestione delle command con errori!!!
	// spostare il controllo del downloader dopo rendering thread
	
	switch(this->getCommandActionIndex(lcommand))
	{
	case GET_CURRENT_MATRIX:
		retstr = this->getViewMatrix();
		break;
	case GO_TO_MATRIX_DIRECTLY:	
		if(!this->setViewMatrixAndGoTo(rcommand))
			retstr = "STREAM_ERROR";
		break;
	case SET_ANIMATION_KEY:
		if(!this->setAnimationKey(rcommand))
			retstr = "STREAM_ERROR";
		break;
	case SET_ANIMATION_PATH_SMOOTHNESS:
		if(!this->setViewAnimationPathSmoothness(rcommand))
			retstr = "STREAM_ERROR";
		break;
	case GET_ANIMATION_TIME:
		retstr = this->getViewAnimationTime();
		break;
	case GET_ANIMATION_PATH_SMOOTHNESS:
		retstr = this->getViewAnimationPathSmoothness();
		break;
	case SET_ANIMATION_TIME:	
		if(!this->setViewAnimationTime(rcommand))
			retstr = "STREAM_ERROR";
		break;
	case START_ANIMATION:
		if(!this->startAnimation())
			retstr = "NO_ANIMATION";
		break;
	case CONTINUE_ANIMATION:
		if(!this->continueAnimation())
			retstr = "NO_ANIMATION";
		break;
	case STOP_ANIMATION:
		if(!this->stopAnimation())
			retstr = "NO_ANIMATION";
		break;
	case RESET_DEFAULT:
		this->resetSettings();
		break;
	default:
		retstr = "UNKNOWN_ACTION";
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


bool AnimateViewHandler::setViewMatrixAndGoTo( std::string viewstring )
{
	if(_activateTransition)
		return false;

	std::stringstream s(viewstring);
    
    osg::ref_ptr<osgDB::ReaderWriter> reader = osgDB::Registry::instance()->getReaderWriterForExtension(std::string("osg"));
	
	if( !reader.valid() ) 
		return false;
	
	osgDB::ReaderWriter::ReadResult res = reader->readObject( s, _options.get() );
	
	if(res.validObject()) 
	{
		osg::ref_ptr<osg::MatrixTransform> m1 = static_cast<osg::MatrixTransform *> (res.getObject());

		//Setto la matrice di trasformazione finale
		_transitionMatrix.set( m1->getMatrix() );

		//Setto la posizione corrente della camera
		_oldMatrix.set(_mainViewer->getCameraManipulator()->getMatrix() );
	
		//Setto lo start time
		_startTime = osg::Timer::instance()->tick();

		//
		_activateTransition = true;
	} 
	else
		return false;

	return true;
}

std::string AnimateViewHandler::getViewAnimationTime()
{
	double time = _animationTime / 1000.0; //Riporto in secondi 
	std::ostringstream convstream;
	convstream << time << std::flush;
	return convstream.str();
}

bool AnimateViewHandler::setViewAnimationTime(std::string animtime)
{
	if(_activateTransition)
		return false;

	std::istringstream atstream(animtime);
	double anitemp = DEFAULT_ANIMATION_TIME;
	
	if(!(atstream >> anitemp >> std::dec).fail())
	{
		_animationTime = anitemp * 1000; //riporto in ms
		return true;
	}

	return false;
}

std::string AnimateViewHandler::getViewAnimationPathSmoothness()
{
	double s = _animationPathSmoothness;
	std::ostringstream convstream;
	convstream << s << std::flush;
	return convstream.str();
}

bool AnimateViewHandler::setViewAnimationPathSmoothness(std::string s)
{
	std::istringstream atstream( s );
	double tmp = DEFAULT_ANIMATION_PATH_SMOOTHNESS;
	
	if(!(atstream >> tmp >> std::dec).fail())
	{
		_animationPathSmoothness = tmp;
		return true;
	}

	return false;
}

bool AnimateViewHandler::startAnimation()
{
	//if( (_animationMatrixArray.size() < 1) || (_animationTimeArray.size() < 1) || _activateTransition)
	if( (_animationMatrixVector.size() < 1) || (_animationTimeVector.size() < 1) || _activateTransition || _duringAnimation)
		return false;

	 //Setto la posizione corrente della camera
	_oldMatrix.set(_mainViewer->getCameraManipulator()->getMatrix() );
	
	//Setto la prima matrice e il primo tempo 
/*
	_sequenceTransitionMatrix = _animationMatrixArray.front();
	_sequenceAnimationTime = _animationTimeArray.front();

	_animationMatrixArray.pop();
	_animationTimeArray.pop();
*/
	_sequenceTransitionMatrix = _animationMatrixVector[0];
	_sequenceAnimationTime = _animationTimeVector[0];

	_currentKey++;

	//Setto lo start time
	_startTime = osg::Timer::instance()->tick();

	_activateTransition = true;
	_duringAnimation = true;

	return true;
}

bool AnimateViewHandler::continueAnimation()
{
	//if( (_animationMatrixArray.size() < 1) || (_animationTimeArray.size() < 1) || !_activateTransition)
	if( ( _currentKey < (int)_animationMatrixVector.size()) || !_activateTransition)
		return false;

	_activateTransition = true;
	_duringAnimation = true;

	return true;
}

bool AnimateViewHandler::stopAnimation()
{
	_activateTransition = false;
	_duringAnimation = false;
	_currentKey = 0;

	return true;
}

void AnimateViewHandler::resetSettings()
{
/*
	while(_animationMatrixArray.size())
		_animationMatrixArray.pop();

	while(_animationTimeArray.size())
		_animationTimeArray.pop();
*/
	while(_animationMatrixVector.size()) _animationMatrixVector.pop_back();
	while(_animationTimeVector.size()) _animationTimeVector.pop_back();

	_currentKey = 0;
	_animationTime = DEFAULT_ANIMATION_TIME;
	_activateTransition = false;
	_duringAnimation    = false;
}

bool AnimateViewHandler::setAnimationKey( std::string key )
{
	if(_activateTransition)
		return false;

	std::string lcommand, rcommand;
	double anitemp = DEFAULT_ANIMATION_TIME;

	this->splitActionCommand(key, lcommand, rcommand);

	if(lcommand != "TIME")
		return false;

	this->splitActionCommand(rcommand, lcommand, rcommand);
	
	std::istringstream atstream(lcommand);
		
	if( (atstream >> anitemp >> std::dec).fail() )
		return false;
	
	this->splitActionCommand(rcommand, lcommand, rcommand);

	if(lcommand != "MATRIX")
		return false;

	std::stringstream s(rcommand);
    
    osg::ref_ptr<osgDB::ReaderWriter> reader = osgDB::Registry::instance()->getReaderWriterForExtension(std::string("osg"));
	
	if( !reader.valid() ) 
		return false;
	
	osgDB::ReaderWriter::ReadResult res = reader->readObject( s, _options.get() );
	
	if(res.validObject()) 
	{
		osg::ref_ptr<osg::MatrixTransform> m1 = static_cast<osg::MatrixTransform *> (res.getObject());

		/*
		_animationMatrixArray.push( m1->getMatrix() );
		_animationTimeArray.push(anitemp * 1000.0); //Riporto in millisecondi
		*/

		//_animationMatrixVector[_currentKey] = m1->getMatrix();
		_animationMatrixVector.push_back( m1->getMatrix() );
		//_animationMatrixVector.insert( _animationMatrixVector.begin(), m1->getMatrix() );
		//_animationTimeVector[_currentKey] = (anitemp * 1000.0); //Riporto in millisecondi
		_animationTimeVector.push_back( anitemp * 1000.0 );
		//_animationTimeVector.insert( _animationTimeVector.begin(), (anitemp * 1000.0) );
		
		return true;
	} 
	else
		return false;
}

osg::Vec3 AnimateViewHandler::interpolateCR(double t, osg::Vec3 p0, osg::Vec3 p1, osg::Vec3 p2, osg::Vec3 p3){
	osg::Vec3 R;
	double t2,t3;
	t2 = t * t;
	t3 = t2 * t;

	R = (p1 * 2.0) + ((p2 - p0)*t) + ((p0*2.0 - p1*5.0 + p2*4.0 - p3)*t2) + ((p1*3.0 - p0 - p2*3.0 + p3)*t3);
	R *= 0.5;

	return R;
}