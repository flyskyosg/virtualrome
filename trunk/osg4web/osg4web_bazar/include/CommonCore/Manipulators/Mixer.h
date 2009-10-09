/*
	Mixer.h - Mixer Header
	by Bruno Fanini ( http://www.brunofanini.tk )
========================================================================*/

#ifndef __MIXER__
#define __MIXER__

#include <osg/Referenced>
#include <osgGA/MatrixManipulator>
#include <osg/Math>
#include <algorithm>
#include <osg/Vec2>
#include <osg/Vec3>
#include <osg/Vec4>

using namespace osg;
using namespace std;

class Mixer : public Referenced {

	public:

		/// This factor soften/sharpen the Acceleration & Deceleration of BOOST_TO
		/// interpolation. Set to 1.0 by default. Lower values will smooth interpolation 
		/// while higher ones will result in a sharpened Acceleration/Deceleration.
		/// Must be > 0.0, values near 0.0 will approximate a LINEAR interpolation.
		#define FLYTOSHARPENFACTOR	1.7

			Mixer(){ _mode = Mixer::LINEAR; };
			~Mixer(){};

		/// Interpolation Modes
		enum InterpolationMode {
			/// Default
			LINEAR,
			/// Square (^2)
			SQUARE,
			/// Square Root (sqrt)
			SQUARE_ROOT,
			/// Cosinoidal Interpolation
			COSINOIDAL,
			/// Boost-To Acceleration/Deceleration
			BOOST_TO,
			};

			void setInterpolationMode(InterpolationMode m){ _mode = m; }
			InterpolationMode getInterpolationMode() { return _mode; }
		
			inline double Lerp(double t, double a,double b){
				return (a + t*(b - a));
				};

			double interpolate(double t, double a, double b, InterpolationMode M=LINEAR);

			Vec2d interpolate(double t, Vec2d A, Vec2d B, InterpolationMode M=LINEAR){
				return Vec2d(
					interpolate(t, A.x(), B.x(), M),
					interpolate(t, A.y(), B.y(), M)
					);
				}
			Vec3d interpolate(double t, Vec3d A, Vec3d B, InterpolationMode M=LINEAR){
				return Vec3d(
					interpolate(t, A.x(), B.x(), M),
					interpolate(t, A.y(), B.y(), M),
					interpolate(t, A.z(), B.z(), M)
					);
			}
			Vec4d interpolate(double t, Vec4d A, Vec4d B, InterpolationMode M=LINEAR){
				return Vec4d(
					interpolate(t, A.x(), B.x(), M),
					interpolate(t, A.y(), B.y(), M),
					interpolate(t, A.z(), B.z(), M),
					interpolate(t, A.w(), B.w(), M)
					);
				}
			Quat interpolate(double t, Quat A, Quat B, InterpolationMode M=LINEAR);
		
			float cubicInterpolate(double t, float a1,float a,float b,float b1,InterpolationMode M=LINEAR);


		protected:
			InterpolationMode	_mode;
			void ApplyDistributionFunction(double &t);

};

#endif