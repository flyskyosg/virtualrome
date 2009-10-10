
#include <CommonCore/Scenehandlers/NodeParserHandler.h>

#include <iostream>



using namespace SceneHandlers;


/********************************************************************
 *
 *  Metodi Classe NodeParserHandler::EventCommandRegister
 *
 ********************************************************************/

NodeParserHandler::EventCommandRegister::~EventCommandRegister()
{
	for(unsigned int i = 0; i < _registeredStubb.size(); i++)
		_registeredStubb.at(i) = NULL;
}

/********************************************************************
 *
 *  Metodi Classe NodeParser Handler
 *
 ********************************************************************/

/** Gestione dell'evento FRAME */

void NodeParserHandler::callmethodqueue(osgViewer::Viewer* viewer, const osgGA::GUIEventAdapter& ea)
{
	//Chiamo callmethodqueue di PickerHandler per calcolare il nodepath 
	PickerHandler::callmethodqueue(viewer, ea);
	
	//Parse del NodePath
	for(osg::NodePath::iterator iter = _savenp.begin(); iter != _savenp.end(); iter++)
	{
		bool stopiteration = false;
		osg::ref_ptr<osg::Node> node = (osg::Node*) *iter;
	
		if(handleNodeDescriptions(node.get(), viewer, ea))
			iter = _savenp.end(); //se stubb torna true blocco fermo l'iterazione
	}
}

bool NodeParserHandler::handleNodeDescriptions(osg::Node* node, osgViewer::Viewer* viewer, const osgGA::GUIEventAdapter& ea)
{
	bool stopiteration = false;

	osg::Node::DescriptionList desclist = node->getDescriptions();

	if( desclist.size() < 0)
		return stopiteration;

	int index = -1;
	for(int k = 0; k < (int) _registeredEvents.size(); k++)
	{
		if(_registeredEvents.at(k).getEvent() == _saveeventtype)
			index = k;
	}

	if(index != -1)
	{
		for(unsigned int i = 0; i < desclist.size(); i++)
		{
			std::string command, argument;
			this->splitCommandArgument(desclist.at(i), command, argument);
			
			for(unsigned int k = 0; k < _registeredEvents.at(index).getStubbVectorSize(); k++)
			{
				if(_registeredEvents.at(index).getStubbById(k)->getCommand() == command)
				{
					if( _registeredEvents.at(index).getStubbById(k)->commandBridge( argument, node, viewer, ea ) ) //Se stubb ritorna true stoppo l'iterazione sul nodepath
						return true;
				}
			}
		}
	}

	return stopiteration;
}

bool NodeParserHandler::addCommand(osgGA::GUIEventAdapter::EventType evt, NodeParserHandler::CommandStubb* stubb)
{
	std::vector<NodeParserHandler::EventCommandRegister>::iterator itr = _registeredEvents.begin();
	
	for(;itr != _registeredEvents.end(); ++itr)
		if( itr->getEvent() == evt)
			return itr->addCommand(stubb);

	NodeParserHandler::EventCommandRegister newevnt(evt);
	newevnt.addCommand(stubb);
	_registeredEvents.push_back(newevnt);

	this->addUniquePickingEventType(evt);

	return true;
}

bool NodeParserHandler::EventCommandRegister::addCommand(NodeParserHandler::CommandStubb* stubb)
{ 
	bool ret = true;

	for(unsigned int i = 0; i < _registeredStubb.size(); i++)
		if(_registeredStubb.at(i)->getCommand() == stubb->getCommand())
			ret = false;

	if(ret)
		_registeredStubb.push_back(stubb);

	return ret;
}

//Divide il comando in command e argument
void NodeParserHandler::splitCommandArgument(std::string command, std::string& largs, std::string& rargs, std::string delimiter)
{
	std::string::size_type pos = command.find( delimiter );

	largs = command.substr(0, pos);

	if( pos != std::string::npos )
		rargs = command.substr(pos + 1, command.size() -1);
	else
		rargs.clear();
}