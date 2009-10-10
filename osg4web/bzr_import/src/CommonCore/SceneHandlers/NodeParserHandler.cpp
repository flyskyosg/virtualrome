
#include <CommonCore/Scenehandlers/NodeParserHandler.h>
#include <CommonCore/Scenehandlers/NodeCommandList.h>

#include <iostream>



using namespace SceneHandlers;

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
	
	//Parsing del nodo
	parsingNode();
}

/** Parsing dell'eventuale NodePath trovato tramite Picker */

void NodeParserHandler::parsingNode()
{
	//Pulisco la vecchia struttura del CommandList
	_ndcommlist.clearNodeCommandList();

	//Parse del NodePath
	for(osg::NodePath::iterator iter = _savenp.begin(); iter != _savenp.end(); iter++)
	{
		osg::ref_ptr<osg::Node> node = dynamic_cast<osg::Node*>( *iter );
	
		if( node->getDescriptions().size() != 0)
			_ndcommlist.addNodeCommand(node.get(), iter);
	}
}


/** Setta NodeCommand tramite NCommandList index, false altrimenti */

bool NodeParserHandler::getNodeCommand(unsigned int i, NodeCommandList::NodeCommand &ndc) 
{ 
	return _ndcommlist.getNodeCommand(i, ndc); 
}


/** Setta NodeCommand che fa match col comando passato da command string. Il tipo di ricerca viene settata tramite searchOption, false altrimenti */

bool NodeParserHandler::findNodeCommandByName(std::string commandstr, NodeCommandList::NodeCommand &ndc, NodeCommandList::SearchOptions searchOption) 
{ 
	return _ndcommlist.findNodeCommand(commandstr, ndc, searchOption); 
}

/** Setta NodeCommand se trova corrispondenza coi comandi registrati. Il tipo di ricerca viene settata tramite searchOption, false altrimenti */

bool NodeParserHandler::findNodeCommandByRegister(NodeCommandList::NodeCommand &ndc, NodeCommandList::SearchOptions searchOption)
{
	return _ndcommlist.findNodeCommand(ndc, searchOption); 
}


/** Ritorna la dimensione del NodeCommandList */

unsigned int NodeParserHandler::getNodeCommandListSize() 
{ 
	return _ndcommlist.getNodeCommandListSize(); 
}


void NodeParserHandler::registerCommand(std::string command)
{
	_ndcommlist.registerCommandInCommandList(command);
}