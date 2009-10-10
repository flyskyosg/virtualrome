
#include <CommonCore/Scenehandlers/NodeCommandList.h>



using namespace SceneHandlers;

/********************************************************************
 *
 *  Metodi Classe NodeCommandList::NodeCommand
 *
 ********************************************************************/

/** Distruttore */
NodeCommandList::NodeCommand::~NodeCommand()
{
	if(_node.valid())
		_node = NULL; //Pulisco il nodo se valido
}

/** Ritorna il nodo associato valido, altrimenti false */
osg::Node* NodeCommandList::NodeCommand::getNode() 
{ 
	if(_node.valid())
		return _node.get(); 

	return NULL;
}

/** Setta il node di comando ed il NodePath index relativo */
void NodeCommandList::NodeCommand::setNode(osg::Node* node, osg::NodePath::iterator npitr)
{ 
	_node = node;
	_npitr = npitr;
}

/** Ritorna il comando nella description list, false se nodo non valido */
bool NodeCommandList::NodeCommand::getCommandInDescList(std::string &nodecommand)
{
	bool ret = false;

	if(_node.valid())
	{
		if(_node->getDescriptions().size() > 0)
		{
			nodecommand = _node->getDescriptions().at(0);
			ret = true;
		}
	}

	return ret;
}

/** Ritorna la description list del node */
bool NodeCommandList::NodeCommand::getDescList(std::vector<std::string> &desclist) 
{ 
	bool ret = false;

	if(_node.valid())
	{
		desclist = _node->getDescriptions(); //setto descriptions
		ret = true;
	}

	return ret;
}

/** Ritorna la description list del node senza la prima entry di comando*/
bool NodeCommandList::NodeCommand::getDescListWithoutCommand(std::vector<std::string> &desclist) 
{ 
	bool ret = false;

	if(_node.valid())
	{
		if(_node->getDescriptions().size() > 1)
		{
			std::vector<std::string> newdesc;
			std::vector<std::string>::iterator itr = _node->getDescriptions().begin();
			for(itr++; itr != _node->getDescriptions().end(); itr++)
				newdesc.push_back(*itr);
	
			desclist = newdesc;
			ret = true;
		}
	}

	return ret;
}

/** Controlla la validità del Nodo */
bool NodeCommandList::NodeCommand::isEmptyCommand()
{ 
	return _node.valid(); 
}



/********************************************************************
 *
 *  Metodi Classe NodeCommandList
 *
 ********************************************************************/

/** Crea un NodeCommand e lo aggiunge alla Lista */
void NodeCommandList::addNodeCommand(osg::Node* node, osg::NodePath::iterator npitr) 
{ 
	NodeCommandList::NodeCommand nd(node, npitr);
	this->addNodeCommand(nd); 
}

/** Aggiunge un NodeCommand alla Lista */
void NodeCommandList::addNodeCommand(NodeCommand nd)
{ 
	_nodecommandlist.push_back(nd);
}

/** Setta NodeCommand se esiste il command string in tutto il NodeCommandList scansionato, false altrimenti */
bool NodeCommandList::findNodeCommand(std::string commandstr, NodeCommandList::NodeCommand &ndc, NodeCommandList::SearchOptions searchOption)
{
	std::vector<NodeCommandList::NodeCommand>::iterator beginitr;
	std::vector<NodeCommandList::NodeCommand>::iterator enditr;
	int adder = 0;

	switch(searchOption)
	{
	case NodeCommandList::UP_TO_DOWN:
		{
			beginitr = _nodecommandlist.end();
			enditr = _nodecommandlist.begin();
			adder = -1;
		}
		break;
	default:
		{
			beginitr = _nodecommandlist.begin();
			enditr = _nodecommandlist.end();
			adder = 1;
		}
		break;
	}

	//Cerco tra i NodeCommand e faccio comparazione con stringa di ingresso
	for(std::vector<NodeCommandList::NodeCommand>::iterator itr = beginitr; itr != enditr; itr += adder)
	{
		std::string str;
		if((*itr).getCommandInDescList(str))
		{
			if(commandstr == str)
			{
				ndc = (*itr);
				return true;
			}
		}
	}
	
	return false;
}

/** Setta il primo NodeCommand incontrato sul NodePath partendo dalla foglia */
bool NodeCommandList::findNodeCommand(NodeCommandList::NodeCommand &ndc, NodeCommandList::SearchOptions searchOption)
{
	std::vector<NodeCommandList::NodeCommand>::iterator beginitr;
	std::vector<NodeCommandList::NodeCommand>::iterator enditr;
	int adder = 0;

	switch(searchOption)
	{
	case NodeCommandList::UP_TO_DOWN:
		{
			beginitr = _nodecommandlist.end();
			enditr = _nodecommandlist.begin();
			adder = -1;
		}
		break;
	default:
		{
			beginitr = _nodecommandlist.begin();
			enditr = _nodecommandlist.end();
			adder = 1;
		}
		break;
	}

	//Cerco tra i NodeCommand e faccio la comparazione con i comandi registrati
	for(std::vector<NodeCommandList::NodeCommand>::iterator itr = beginitr; itr != enditr; itr += adder)
	{
		std::string str;
		if((*itr).getCommandInDescList(str))
		{
			for(unsigned int i = 0; i < _registeredCommandList.size(); i++)
			{
				if( str == _registeredCommandList.at(i) )
				{
					ndc = (*itr);
					return true;
				}
			}
		}
	}

	return false;
}

/** Setta NodeCommand tramite index della lista, false altrimenti */
bool NodeCommandList::getNodeCommand(unsigned int i, NodeCommandList::NodeCommand &ndc)
{
	if(i < _nodecommandlist.size())
		ndc = _nodecommandlist.at(i);

	return false;
}

/** Setta command string tramite index di lista, false altrimenti */
bool NodeCommandList::getNodeCommandName(unsigned int i, std::string &commandstr)
{
	if(i < _nodecommandlist.size())
	{
		std::string str;
		
		if(_nodecommandlist.at(i).getCommandInDescList(str))
		{
			commandstr = str;
			return true;
		}
	}

	return false;
}

/** Setta command string tramite index di lista, false altrimenti */
bool NodeCommandList::getNodeCommandDescList(unsigned int i, std::vector<std::string> &commanddesclist)
{
	if(i < _nodecommandlist.size())
	{
		std::vector<std::string> commstr;
		
		if(_nodecommandlist.at(i).getDescList(commstr))
		{
			commanddesclist = commstr;
			return true;
		}
	}

	return false;
}

/** Setta command string tramite index di lista, elimina str di comando */
bool NodeCommandList::getNodeCommandDescListWithoutCommand(unsigned int i, std::vector<std::string> &commanddesclist)
{
	if(i < _nodecommandlist.size())
	{
		std::vector<std::string> commstr;
		
		if(_nodecommandlist.at(i).getDescListWithoutCommand(commstr))
		{
			commanddesclist = commstr;
			return true;
		}
	}

	return false;
}

/** Ritorna nodo tramite index di lista, NULL altrimenti */
osg::Node* NodeCommandList::getNodeCommandNode(unsigned int i)
{
	if(i < _nodecommandlist.size())
		return _nodecommandlist.at(i).getNode();
	
	return NULL;
}

/** Registra i comandi accettati dal parser */
void NodeCommandList::registerCommandInCommandList(std::string command)
{
	_registeredCommandList.push_back(command);
}

/** Pulisce i comandi accettati dal parser */
void NodeCommandList::clearCommandList()
{
	_registeredCommandList.clear();
}

