
#include <PickHandle.h>

#include <osg/io_utils>


bool PickHandler::handle(const osgGA::GUIEventAdapter& ea,osgGA::GUIActionAdapter& aa)
{
    switch(ea.getEventType())
    {
        case(osgGA::GUIEventAdapter::FRAME):
        {
            osgViewer::Viewer* viewer = dynamic_cast<osgViewer::Viewer*>(&aa);
            if (viewer) pick(viewer,ea);
            return false;
        }    
        default:
            return false;
    }
}

void PickHandler::pick(osgViewer::Viewer* viewer, const osgGA::GUIEventAdapter& ea)
{
    osgUtil::LineSegmentIntersector::Intersections intersections;

	 std::string gdlist="";
    if (viewer->computeIntersections(ea.getX(),ea.getY(),intersections))
    {
        for(osgUtil::LineSegmentIntersector::Intersections::iterator hitr = intersections.begin();
            hitr != intersections.end();
            ++hitr)
        {
			
            std::ostringstream os;
            if (!hitr->nodePath.empty() && !(hitr->nodePath.back()->getName().empty()))
            {
                // the geodes are identified by name.
                os<<"Object \""<<hitr->nodePath.back()->getName()<<"\""<<std::endl;
            }
            else if (hitr->drawable.valid())
            {
                os<<"Object \""<<hitr->drawable->className()<<"\""<<std::endl;
            }
			
            os <<"        local coords vertex("<< hitr->getLocalIntersectPoint()<<")"<<"  normal("<<hitr->getLocalIntersectNormal()<<")"<<std::endl;
            os <<"        world coords vertex("<< hitr->getWorldIntersectPoint()<<")"<<"  normal("<<hitr->getWorldIntersectNormal()<<")"<<std::endl;
            const osgUtil::LineSegmentIntersector::Intersection::IndexList& vil = hitr->indexList;
            for(unsigned int i=0;i<vil.size();++i)
            {
                os<<"        vertex indices ["<<i<<"] = "<<vil[i]<<std::endl;
            }
            
            gdlist += os.str();
        }
    }
    setLabel(gdlist);
}