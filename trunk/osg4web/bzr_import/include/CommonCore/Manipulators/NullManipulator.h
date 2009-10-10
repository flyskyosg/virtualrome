#ifndef __OSG4WEB_NULLMANIPULATOR__
#define __OSG4WEB_NULLMANIPULATOR__ 1

#include <osg/Quat>
#include <osgGA/MatrixManipulator>


namespace Manipulators
{
	class NullManipulator : public osgGA::MatrixManipulator
	{
		public:
			NullManipulator();

			virtual const char* className() const { return "NullManipulator"; }

			/** set the position of the matrix manipulator using a 4x4 Matrix.*/
			virtual void setByMatrix(const osg::Matrixd& matrix);

			/** set the position of the matrix manipulator using a 4x4 Matrix.*/
			virtual void setByInverseMatrix(const osg::Matrixd& matrix) { setByMatrix(osg::Matrixd::inverse(matrix)); }

			/** get the position of the manipulator as 4x4 Matrix.*/
			virtual osg::Matrixd getMatrix() const;

			/** get the position of the manipulator as a inverse matrix of the manipulator, typically used as a model view matrix.*/
			virtual osg::Matrixd getInverseMatrix() const;

			/** Get the FusionDistanceMode. Used by SceneView for setting up setereo convergence.*/
			virtual osgUtil::SceneView::FusionDistanceMode getFusionDistanceMode() const { return osgUtil::SceneView::USE_FUSION_DISTANCE_VALUE; }

			/** Get the FusionDistanceValue. Used by SceneView for setting up setereo convergence.*/
			virtual float getFusionDistanceValue() const { return _distance; }

			/** Attach a node to the manipulator. 
				Automatically detaches previously attached node.
				setNode(NULL) detaches previously nodes.
				Is ignored by manipulators which do not require a reference model.*/
			virtual void setNode(osg::Node*);

			/** Return node if attached.*/
			virtual const osg::Node* getNode() const;

			/** Return node if attached.*/
			virtual osg::Node* getNode();

			/** Move the camera to the default position. 
				May be ignored by manipulators if home functionality is not appropriate.*/
			virtual void home(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& us);
			virtual void home(double);
	        
			/** Get the keyboard and mouse usage of this manipulator.*/
			virtual void getUsage(osg::ApplicationUsage& usage) const;

			/** handle events, return true if handled, false otherwise.*/
			virtual bool handle(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& us);

		protected:
			virtual ~NullManipulator();

			void computePosition(const osg::Vec3& eye,const osg::Vec3& lv,const osg::Vec3& up);

			osg::ref_ptr<osg::Node>       _node;

			double _modelScale;

			osg::Vec3d   _center;
			osg::Quat    _rotation;
			double       _distance;

	};
};

#endif //__OSG4WEB_NULLMANIPULATOR__