#ifndef __OSG4WEB_PICKERHANDLER__
#define __OSG4WEB_PICKERHANDLER__ 1


#include <sstream>

#include <osgText/Text>
#include <osgViewer/Viewer>



namespace SceneHandlers
{

	/***********************************************************************
	 *
	 * TooPickerHandlerltipHandler class
	 *
	 ***********************************************************************/
	class PickerHandler : public osgGA::GUIEventHandler {
	public: 

		//Costruttore
		PickerHandler(osg::Node::NodeMask nm  = 0xffffffff) : _traversalnodemask(nm) {}//osg::Node* scenepicked) : _scenepicked(scenepicked) {}
	    
		//Distruttore
		~PickerHandler() {}
	    
		//Handle degli eventi
		bool handle(const osgGA::GUIEventAdapter& ea,osgGA::GUIActionAdapter& aa);

		//Gestione dell'Evento FRAME - viene ridefinita dai derivati
		virtual void callmethodqueue(osgViewer::Viewer* viewer, const osgGA::GUIEventAdapter& ea);

		//Gestione del Picking nella scena
		virtual void mouseRayIntersection(osgViewer::Viewer* viewer, const osgGA::GUIEventAdapter& ea);

		//Ritorna la NodeMask accettata dal PickHandler
		osg::Node::NodeMask getTraversalNodeMask() { return _traversalnodemask; }
	   
	protected:
		//NodePath intersecato più vicino
		osg::NodePath _savenp;
		//Picking TraversalMask
		osg::Node::NodeMask _traversalnodemask;
	};

};

#endif //__OSG4WEB_PICKERHANDLER__