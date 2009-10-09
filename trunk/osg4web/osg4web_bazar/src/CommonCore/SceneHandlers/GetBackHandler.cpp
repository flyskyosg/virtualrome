#include <CommonCore/Scenehandlers/GetBackHandler.h>

using namespace SceneHandlers;

/********************************************************************
 *
 *  Metodi Classe GetBackHandler
 *
 ********************************************************************/

bool GetBackHandler::commandBridge(std::string argument, osg::Node* node, osgViewer::Viewer* viewer, const osgGA::GUIEventAdapter& ea)
{
	if(ea.getButtonMask() == osgGA::GUIEventAdapter::LEFT_MOUSE_BUTTON) //FIXME: Spostare a Picker Time tramite settaggi del Parser
		_RaiseInterface->raiseCommand(argument);

	return false;
}

bool GetBackHandler::setJSCallToNode(osg::Node* nd, std::string argument)
{
	if(!nd)
		return false;

	osg::Node::DescriptionList desclist = nd->getDescriptions();

	desclist.push_back(this->getCommand() + " " + argument);

	nd->setDescriptions(desclist);

	return true;
}