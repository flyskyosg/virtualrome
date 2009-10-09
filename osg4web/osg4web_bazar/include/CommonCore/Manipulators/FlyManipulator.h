#ifndef __OSG4WEB_FLYMANIPULATOR__
#define __OSG4WEB_FLYMANIPULATOR__ 1

#include <osgGA/MatrixManipulator>
#include <osg/Quat>
#include <osg/AnimationPath>
#include <osgViewer/Viewer>

using namespace osgGA;

namespace Manipulators
{
	class FlyManipulator : public MatrixManipulator
	{
	public:

		FlyManipulator();

		virtual const char* className() const { return "fly"; }
	    virtual void setByMatrix(const osg::Matrixd& matrix);
	    virtual void setByInverseMatrix(const osg::Matrixd& matrix) { setByMatrix(osg::Matrixd::inverse(matrix)); }
		virtual osg::Matrixd getMatrix() const;
	    virtual osg::Matrixd getInverseMatrix() const;

	    virtual void setNode(osg::Node*); 
	    virtual const osg::Node* getNode() const {return m_node.get();};
	    virtual osg::Node* getNode() {return m_node.get();};

	    virtual void setExcludeNode(osg::Node* en) {m_exclude_node = en;}; 
	    virtual const osg::Node* getExcludeNode() const {return m_exclude_node.get();};
	
		osg::Vec3d getCenter() {return m_center;};

	    /** Move the camera to the default position.*/
	    virtual void home(const GUIEventAdapter& ea,GUIActionAdapter& us);
	    //virtual void computeHomePosition();
        
	    virtual void init(const GUIEventAdapter& ea,GUIActionAdapter& us);

	    virtual bool handle(const GUIEventAdapter& ea,GUIActionAdapter& us);
        
		void FlyTo( osg::Vec3 wposition );
	    void FlyTo(double x,double y,double yaw,double pitch, double dist);

		void setConstraint( bool value ) { m_constraints = value; }
		bool getConstraint() { return m_constraints; }
		void checkConstraints();

		void setMaxDistance( bool value ) { _maximumDistance = value; }
		double getMaxDistance() { return _maximumDistance; }

		/** Get the keyboard and mouse usage of this manipulator.*/
		virtual void getUsage(osg::ApplicationUsage& usage) const;

	protected:
		virtual ~FlyManipulator();
		void computePosition(const osg::Vec3d& eye,const osg::Vec3d& lv,const osg::Vec3d& up);

		void flushMouseEventStack();
	    void addMouseEvent(const GUIEventAdapter& ea);
	    bool calcMovement();
    
		void rotate(double dx, double dy);
	    void pan(double dx, double dy);
	    void zoom(double dx, double dy);

	    bool isMouseMoving(); /* true if the speed at which the mouse is moving is over a threshold */

		void RotToYawPitch(osg::Matrixd m, double& yaw, double& pitch  );
		void YawPitchToRot(double yaw, double pitch, osg::Quat& r  );
	    void IntersectTerrain();
		bool Intersect( osg::Vec3d p1, osg::Vec3d p2, osg::Vec3d& result);
	
		// Internal event stack comprising last three mouse events.
	    osg::ref_ptr<const GUIEventAdapter> m_ga_t1;
		osg::ref_ptr<const GUIEventAdapter> m_ga_t0;

		osg::ref_ptr<osg::Node> m_node;
		osg::ref_ptr<osg::Node> m_exclude_node;
		bool m_thrown;
		osg::Vec3d m_center;
		osg::Quat m_rotation;
		float m_distance;
		osg::Vec3d m_previousUp;

		// f1 --- begin Fly position
		osg::Vec3d m_f1_center;
		osg::Quat m_f1_rotation;
		float m_f1_distance;

		// f2 --- end Fly position
		bool m_f2_valid;
		osg::Vec3d m_f2_center;
		osg::Quat m_f2_rotation;
		float m_f2_distance;

		// fly
		bool m_flying;
		bool m_request_fly;
		double m_begin_fly_time;
		double m_flying_duration;

		double m_yaw;
		double m_pitch;
		bool m_needIntersect;

		class CurrentInst : public osg::Referenced
		{
		public:
			CurrentInst() : x(0), y(0), yaw(0), pitch(0), dist(0) {}
            
			double x;
			double y;
			double yaw;
			double pitch;
			double dist; 
		};

		osg::ref_ptr<CurrentInst> m_pts[9];

		//Distanza massima di allontanamento
		double _maximumDistance;

		bool m_constraints;
	};
};

#endif //__OSG4WEB_FLYMANIPULATOR__



