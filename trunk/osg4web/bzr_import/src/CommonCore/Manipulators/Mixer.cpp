/*
	Mixer.cpp - General purpose Interpolation Class
	by Bruno Fanini ( http://www.brunofanini.tk )
========================================================================*/

// Header
#include <CommonCore/Manipulators/Mixer.h>

#include <osg/Notify>
#include <osg/io_utils>
#include <osg/Math>

using namespace osg;
using namespace osgGA;

//////////////////////////////////////////////////////////////////////////


/// This applies to parameter t [0,1] a distribution function, 
/// previously set with setInterpolationMode().
void Mixer::ApplyDistributionFunction(double &t){
	switch( Mixer::_mode ){
		// Linear distribution
		case LINEAR:{ t=t; }	// Can skip
		
		// Square distribution
		case SQUARE:{ t *= t; }
		// Square root distribution
		case SQUARE_ROOT:{ t = sqrt(t); }

		// Cosinoidal distribution
		case COSINOIDAL:{
			double ft = (t * osg::PI);
			t = (1.0 - cos(ft))*0.5;
			}

		// Boost-To distribution: I used atan() for its behavior.
		case BOOST_TO:{
			double ft,f,k;
			// Calculates [sup]
			double sup = atan(osg::PI * FLYTOSHARPENFACTOR);
			// Calculates evaluation point
			ft = (t * osg::PI * 2.0) - osg::PI;
			k = atan(ft * FLYTOSHARPENFACTOR);
			f = ((sup-k) / (sup*2.0));
			t = 1.0 - f;
			}
		}
}

/// Interpolates 2 floats with t [0,1] and optional distribution function M.
double Mixer::interpolate(double t, double a,double b, InterpolationMode M){
	if (t<=0.0) return a;
	if (t>=1.0) return b;

	_mode = M;
	ApplyDistributionFunction( t );
	
	return ( Lerp(t,a,b) );
}

// Interpolate 2 Quaternions.
Quat Mixer::interpolate(double t, Quat A, Quat B, InterpolationMode M){
	_mode = M;
	ApplyDistributionFunction( t );
	osg::Quat Q;
	Q.slerp(t,A,B);
	return Q;		
}

/// Interpolates a & b with cubic interpolation.
float Mixer::cubicInterpolate(double t, float a1,float a,float b,float b1,InterpolationMode M){
	if (t<=0.0) return a;
	if (t>=1.0) return b;

	_mode = M;
	ApplyDistributionFunction( t );

	float c[3];
	c[0] = (b1 - b) - (a1 - a);
	c[1] = (a1 - a) - c[0];
	c[2] = (b - a1);

	return ( (c[0]*t*t*t) + (c[1]*t*t) + (c[2]*t) + a );
}