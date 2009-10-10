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
		PickerHandler(osg::Node::NodeMask nm  = 0xffffffff) : _traversalnodemask(nm), _saveeventtype(osgGA::GUIEventAdapter::NONE) {}
	    
		//Handle degli eventi
		bool handle(const osgGA::GUIEventAdapter& ea,osgGA::GUIActionAdapter& aa);

		//Ritorna la NodeMask accettata dal PickHandler
		osg::Node::NodeMask getTraversalNodeMask() { return _traversalnodemask; }

		//Setta la NodeMask accettata dal PickHandler
		void setTraversalNodeMask(osg::Node::NodeMask tm) { _traversalnodemask = tm; }

	protected:
		//Distruttore
		~PickerHandler() {}

		//Gestione dell'Evento FRAME - viene ridefinita dai derivati
		virtual void callmethodqueue(osgViewer::Viewer* viewer, const osgGA::GUIEventAdapter& ea);

		//Gestione del Picking nella scena
		virtual void mouseRayIntersection(osgViewer::Viewer* viewer, const osgGA::GUIEventAdapter& ea);

		//Ritorna l'event Type di Picking tramite id
		osgGA::GUIEventAdapter::EventType getPickingEventType(unsigned int i) { return _eventtypevector.at(i); }
		
		//Ritorna il vettore contenente gli event Type registrati
		std::vector<osgGA::GUIEventAdapter::EventType> getPickingEventTypeVector() { return _eventtypevector; }

		//Pulisce il vettore contenente gli event Type registrati
		void clearPickingEventTypeVector() { _eventtypevector.clear(); }

		//Ritorna l'event type di picking
		unsigned int getPickingEventTypeSize() { return _eventtypevector.size(); }

		//Aggiunge l'event type di picking se non è già presente
		bool addUniquePickingEventType(osgGA::GUIEventAdapter::EventType evnt);
	   
	
		//NodePath intersecato più vicino
		osg::NodePath _savenp;
		//Evento di Intersezione del nodepath
		osgGA::GUIEventAdapter::EventType _saveeventtype;
		//Picking TraversalMask
		osg::Node::NodeMask _traversalnodemask;
		//Vettore di eventi di picking
		std::vector<osgGA::GUIEventAdapter::EventType> _eventtypevector;
	};

};

#endif //__OSG4WEB_PICKERHANDLER__