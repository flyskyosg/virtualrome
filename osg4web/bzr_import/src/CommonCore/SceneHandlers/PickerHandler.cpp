#include <CommonCore/Scenehandlers/PickerHandler.h>

#include <iostream>

#include <osg/Group>


using namespace SceneHandlers;

/********************************************************************
 *
 *  Metodi Classe PickerHandler
 *
 ********************************************************************/

/** Definizione della Handle degli eventi */

bool PickerHandler::handle(const osgGA::GUIEventAdapter& ea,osgGA::GUIActionAdapter& aa)
{
    switch(ea.getEventType()) //Switch degli eventi
    {
        case(osgGA::GUIEventAdapter::FRAME): //Catch evento FRAME
        {
            osgViewer::Viewer* viewer = dynamic_cast<osgViewer::Viewer*>(&aa);
            if (viewer)
				callmethodqueue(viewer,ea); //Funzione virtuale di gestione evento FRAME
			
			return false;
        }    
        default:
            return false;
    }
}

/** Funzione Virtuale per la gestione dell'evento FRAME - pu� essere ridefinita */

void PickerHandler::callmethodqueue(osgViewer::Viewer* viewer, const osgGA::GUIEventAdapter& ea)
{
	//Faccio Intersect sulla scena
	mouseRayIntersection(viewer,ea);
}

/** Traccia il raggio di intersezione con la scena e salva il nodepath dell'intersezione pi� vicina */

void PickerHandler::mouseRayIntersection(osgViewer::Viewer* viewer, const osgGA::GUIEventAdapter& ea)
{
	_savenp.clear(); //Pulisco il nodepath di store

	osgUtil::LineSegmentIntersector::Intersections intersections;

	//Calcolo intersezione
	if (viewer->computeIntersections(ea.getX(), ea.getY(), intersections, _traversalnodemask)) //Ci sono intersezioni
    {
		double distance = -1.0;
		osg::Vec3 eye, center, up;

		//Prendo il punto di vista dell'osservatore
		viewer->getCamera()->getViewMatrix().getLookAt(eye, center, up);

		//Prendo il pi� vicino all'osservatore
		osgUtil::LineSegmentIntersector::Intersections::iterator savedhitr;
        for(osgUtil::LineSegmentIntersector::Intersections::iterator hitr = intersections.begin();
            hitr != intersections.end();
            ++hitr)
        {
			osg::Vec3d hitpoint = hitr->getLocalIntersectPoint(); //Punto di Hit
			double lenght = sqrt( pow(center.x() - hitpoint.x(), 2) + pow(center.y() - hitpoint.y(), 2) + pow(center.z() - hitpoint.z(), 2) ); //distanza euclidea

			if(distance == -1.0 || lenght < distance) //Se punto pi� vicino faccio store
			{
				savedhitr = hitr;
				distance = lenght;
			}
        }

		//Salvo il nodepath dell'intersezione pi� vicina
		_savenp = savedhitr->nodePath;
    }
}	



