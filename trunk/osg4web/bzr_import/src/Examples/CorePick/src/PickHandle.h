#ifndef __OSG4WEB_PICKHANDLE__
#define __OSG4WEB_PICKHANDLE__ 1

#include <sstream>

#include <osgText/Text>
#include <osgViewer/Viewer>

class PickHandler : public osgGA::GUIEventHandler {
public: 

    PickHandler(osgText::Text* updateText):
        _updateText(updateText) {}
        
    ~PickHandler() {}
    
    bool handle(const osgGA::GUIEventAdapter& ea,osgGA::GUIActionAdapter& aa);

    virtual void pick(osgViewer::Viewer* viewer, const osgGA::GUIEventAdapter& ea);

    void setLabel(const std::string& name)
    {
        if (_updateText.get()) _updateText->setText(name);
    }
    
protected:

    osg::ref_ptr<osgText::Text>  _updateText;
};

#endif //__OSG4WEB_PICKHANDLE__