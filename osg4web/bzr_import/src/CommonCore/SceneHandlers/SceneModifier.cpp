

#include <CommonCore/Scenehandlers/SceneModifier.h>
#include <CommonCore/Visitors/FindNodeVisitor.h>


#include <osg/Switch>

#include <osgDB/Registry>

#include <iostream>
#include <sstream>


using namespace SceneHandlers;

/**
 *	ISTRUZIONI PER L'USO:
 *
 *	FIXME: capire come fare sta roba senza un Parser e senza usare i marker di stringa
 *	TIPS: per adesso i nomi NON POSSONO contenere spazi
 *
 *		SMODIF_CREATE_GROUP
 *			MAINSCENENODE	"Nuovo_Nome_Nodo"
 *			"Nodo_Padre"	EXCLUSIVE/INSTANCE		"Nuovo_Nome_Nodo"
 *
 *		SMODIF_CREATE_SWITCH
 *			MAINSCENENODE	"Nuovo_Nome_Nodo"
 *			"Nodo_Padre"	EXCLUSIVE/INSTANCE		"Nuovo_Nome_Nodo"
 *
 *		SMODIF_CREATE_MATRIXTRANSFORM
 *			MAINSCENENODE	"Nuovo_Nome_Nodo"
 *			"Nome_Nodo"	EXCLUSIVE/INSTANCE		"Nuovo_Nome_Nodo"
 *
 *		SMODIF_DELETE_NODE
 *			"Nome_Nodo"
 *
 *		SMODIF_SWITCH_ALL_CHILD
 *			"Nome_Nodo"		
 *
 *		SMODIF_SWITCH_CHILD_BYNAME
 *			"Nome_Nodo"		"Nome_Nodo_Figlio"
 *
 *		SMODIF_SET_MATRIXIDENTITY
 *			"Nodo_Padre"
 *
 *		SMODIF_SET_MATRIX
 *			"Nodo_Padre"	"MatrixStream"
 *
 *		SMODIF_SET_MATRIX_ROTATE
 *			"Nodo_Padre"	PRE/POST	"DEGREES" "X" "Y" "Z"
 *
 *		SMODIF_SET_MATRIX_SCALE
 *			"Nodo_Padre"	PRE/POST	"X" "Y" "Z"
 *
 *		SMODIF_SET_MATRIX_TRANSLATE
 *			"Nodo_Padre"	PRE/POST	"X" "Y" "Z"
 *
 *		SMODIF_GET_MATRIX
 *			"Nodo_Padre"
 *
 *		SMODIF_MOVE_NODE
 *			"Nodo_To_Move"	"New_Parent_Node"
 *
 *		SMODIF_MOVE_NODE
 *			"Nodo_To_Move"	"New_Parent_Node"
 *
 */


/** Costruttore */
SceneModifier::SceneModifier() : _SceneData(NULL),
	_options(new osgDB::ReaderWriter::Options)
{
	this->initCommandActions();
}


SceneModifier::SceneModifier(osg::Group* grp) : _SceneData(grp)
{
	this->initCommandActions();
}

/** Distruttore */
SceneModifier::~SceneModifier()
{
	if(_SceneData.valid())
		_SceneData = NULL;
}

void SceneModifier::initCommandActions()
{
	this->setCommandAction("SMODIF_CREATE_GROUP");
	this->setCommandAction("SMODIF_CREATE_SWITCH");
	this->setCommandAction("SMODIF_CREATE_MATRIXTRANSFORM");
	this->setCommandAction("SMODIF_DELETE_NODE");
	this->setCommandAction("SMODIF_SWITCH_ALL_CHILD");
	this->setCommandAction("SMODIF_SWITCH_CHILD_BYNAME");
	this->setCommandAction("SMODIF_SET_MATRIXIDENTITY");
	this->setCommandAction("SMODIF_SET_MATRIX");
	this->setCommandAction("SMODIF_SET_MATRIX_ROTATE");
	this->setCommandAction("SMODIF_SET_MATRIX_SCALE");
	this->setCommandAction("SMODIF_SET_MATRIX_TRANSLATE");
	this->setCommandAction("SMODIF_GET_MATRIX");
	this->setCommandAction("SMODIF_MOVE_NODE");
}


std::string SceneModifier::handleAction(std::string action, std::string argument)
{
	std::string retstr;

	switch(this->getCommandActionIndex(action))
	{
	case CREATE_GROUP:
		retstr = this->createGroup(argument);
		break;
	case CREATE_SWITCH:
		retstr = this->createSwitch(argument);
		break;
	case CREATE_MATRIXTRANSFORM:
		retstr = this->createMatrixTransform(argument);
		break;
	case DELETE_NODE:
		retstr = this->deleteNode(argument);
		break;
	case SWITCH_ALLCHILD:
		retstr = this->switchAllChild(argument);
		break;
	case SWITCH_CHILD_BYNAME:
		retstr = this->switchChildByName(argument);
		break;
	case SET_MATRIXIDENTITY:
		retstr = this->setMatrixIdentityByName(argument);
		break;
	case SET_MATRIX:
		retstr = this->setMatrixByName(argument);
		break;
	case SET_MATRIX_ROTATE:
		retstr = this->setMatrixRotateByName(argument);
		break;
	case SET_MATRIX_SCALE:
		retstr = this->setMatrixRotateByName(argument);
		break;
	case SET_MATRIX_TRANSLATE:
		retstr = this->setMatrixTranslateByName(argument);
		break;
	case GET_MATRIX:
		retstr = this->getMatrixByName(argument);
		break;
	case MOVE_NODE:
		retstr = this->moveNode(argument);
		break;
	default: //UNKNOWN_CORE_COMMAND
		retstr = "UNKNOWN_CORE_COMMAND"; //FIXME: qua non ci può finire. Da togliere
		break;
	}

	return retstr;
}

std::string SceneModifier::createGroup(std::string action)
{
	osg::ref_ptr<osg::Group> grp = new osg::Group;
	return this->addNodeToSceneParsingAction(grp.get(), action);
}

std::string SceneModifier::createSwitch(std::string action)
{
	osg::ref_ptr<osg::Switch> grp = new osg::Switch;
	return this->addNodeToSceneParsingAction(grp.get(), action);
}

std::string SceneModifier::createMatrixTransform(std::string action)
{
	osg::ref_ptr<osg::MatrixTransform> mt = new osg::MatrixTransform;
	mt->setMatrix(osg::Matrix::identity()); //Setto la matrice d'identità per default
	return this->addNodeToSceneParsingAction(mt.get(), action);
}

std::string SceneModifier::moveNode(std::string action)
{
	std::string nodename, newparentnode;
	std::string retstr("SMODIF_BAD_COMMAND");

	this->splitActionCommand(action, nodename, newparentnode);

	if(newparentnode.empty())
		return retstr;
	
	Visitors::FindNodeVisitor fnvbn(nodename);
	_SceneData->accept(fnvbn);

	unsigned int number = fnvbn.getNodeFoundSize();

	if(number == 0)
		return "SMODIF_NODE_NOTFOUND";

	osg::ref_ptr<osg::Node> nodetomove = dynamic_cast<osg::Node*>(fnvbn.getNodeByIndex(0).at(fnvbn.getNodeByIndex(0).size() - 1));

	if(nodetomove.valid())
	{
		Visitors::FindNodeVisitor fnvbnParent(newparentnode);
		_SceneData->accept(fnvbnParent);

		number = fnvbnParent.getNodeFoundSize();

		if(number == 0)
			return "SMODIF_NODE_NOTFOUND";

		osg::ref_ptr<osg::Group> newpnode = dynamic_cast<osg::Group*>(fnvbnParent.getNodeByIndex(0).at(fnvbnParent.getNodeByIndex(0).size() - 1)); //Prendo il nuovo padre
		osg::ref_ptr<osg::Group> oldpnode = dynamic_cast<osg::Group*>(fnvbn.getNodeByIndex(0).at(fnvbn.getNodeByIndex(0).size() - 2)); //Prendo il vecchio Padre

		if(newpnode.valid() && oldpnode.valid())
		{
			newpnode->addChild(nodetomove.get());
			oldpnode->removeChild(nodetomove.get());
		}
		else
			return "SMODIF_CAST_ERROR";
	}
	else
		return "SMODIF_CAST_ERROR";

	retstr = "SMODIF_COMMAND_OK";

	_SceneData->dirtyBound();

	return retstr;
}

bool SceneModifier::addNodeToParent(osg::Node* parent, osg::Node* child, std::string &retstr)
{
	osg::ref_ptr<osg::Group> grp = dynamic_cast<osg::Group*>(parent);

	if(!grp.valid())
	{
		retstr = "SMODIF_CAST_ERROR";
		return false;
	}

	return grp->addChild(child);
}

std::string SceneModifier::addNodeToSceneParsingAction(osg::Node* newnode, std::string action)
{
	std::string parent, args;
	std::string retstr("SMODIF_BAD_COMMAND");
	
	this->splitActionCommand(action, parent, args);

	if(!args.empty()) //Servono almeno 2 argomenti
	{
		if(parent == "MAINSCENENODE") //Attacco al nodo di Root
		{
			Visitors::FindNodeVisitor fnvbn(parent);
			fnvbn.setUniCheck(args);
			_SceneData->accept(fnvbn);

			if(!fnvbn.isUnique()) //Nome già usato
				return "SMODIF_BAD_NAME";

			newnode->setName(args);
			_SceneData->addChild(newnode);

			_SceneData->dirtyBound();

			return "SMODIF_COMMAND_OK";
		}
		else
		{
			std::string newname, modestr;

			this->splitActionCommand(args, modestr, newname);

			newnode->setName(newname);

			Visitors::FindNodeVisitor fnvbn(parent);
			fnvbn.setUniCheck(newname);
			_SceneData->accept(fnvbn);

			if(!fnvbn.isUnique()) //Nome già usato
				return "SMODIF_BAD_NAME";

			unsigned int number = fnvbn.getNodeFoundSize();

			if(number != 0 ) //Controllo se il numero di nodi trovati è > 0 
			{
				if(modestr == "INSTANCE") //Inserimento di istanze multiple 
				{
					for(unsigned int i = 0; i < number; i++)
						if( !this->addNodeToParent(fnvbn.getNodeByIndex(i).at(fnvbn.getNodeByIndex(i).size() - 1), newnode, retstr) )
							return retstr;
				
					_SceneData->dirtyBound();

					retstr = "SMODIF_COMMAND_OK";
				}
				else if(modestr == "EXCLUSIVE") //Istanza esclusiva
				{
					if(number == 1)
					{
						if( !this->addNodeToParent(fnvbn.getNodeByIndex(0).at(fnvbn.getNodeByIndex(0).size() - 1), newnode, retstr) )
							return retstr;

						_SceneData->dirtyBound();

						retstr = "SMODIF_COMMAND_OK";
					}
					else
						retstr = "SMODIF_EXCLUSIVE_ERROR";
				}
			}
			else
				retstr = "SMODIF_NODE_NOTFOUND";
		}
	}

	return retstr;
}

bool SceneModifier::removeNode(osg::Node* node, std::string &retstr)
{
	//FIXME: controllare che non deallochi il nodo prima dell'ultima rimozione
	for(unsigned int i = 0; i < node->getNumParents(); i++)
	{
		osg::ref_ptr<osg::Group> parent = node->getParent(i);
		if( !parent->removeChild(node) )
		{
			retstr = "SMODIF_DELETE_ERROR";
			return false;
		}
	}

	return true;
}

std::string SceneModifier::deleteNode(std::string action)
{
	std::string retstr("SMODIF_BAD_COMMAND");

	Visitors::FindNodeVisitor fnvbn(action);
	_SceneData->accept(fnvbn);

	unsigned int number = fnvbn.getNodeFoundSize();

	if(number == 0)
		return "SMODIF_NODE_NOTFOUND";

	if( !this->removeNode(fnvbn.getNodeByIndex(0).at(fnvbn.getNodeByIndex(0).size() - 1), retstr) )
		return retstr;

	_SceneData->dirtyBound();

	retstr = "SMODIF_COMMAND_OK";
	
	return retstr;
}

bool SceneModifier::switchAllChildByNode(osg::Node* parent, std::string &retstr)
{
	osg::ref_ptr<osg::Switch> swt = dynamic_cast<osg::Switch*>(parent);

	if(!swt.valid())
	{
		retstr = "SMODIF_CAST_ERROR";
		return false;
	}

	bool childvalue = false;
	for(unsigned int i = 0; i < swt->getNumChildren(); i++)
		if(swt->getValue(i) == true)
			childvalue = true;

	if(!childvalue)
		return swt->setAllChildrenOn();
	else
		return swt->setAllChildrenOff();

	_SceneData->dirtyBound();
}

std::string SceneModifier::switchAllChild(std::string action)
{
	std::string retstr("SMODIF_BAD_COMMAND");
	
	Visitors::FindNodeVisitor fnvbn(action);
	_SceneData->accept(fnvbn);

	unsigned int number = fnvbn.getNodeFoundSize();

	if(number == 0)
		return "SMODIF_NODE_NOTFOUND";

	for(unsigned int i = 0; i < number; i++) //Switch di tutti i nodi trovati a swtype
		if( !this->switchAllChildByNode(fnvbn.getNodeByIndex(i).at(fnvbn.getNodeByIndex(i).size() - 1), retstr) )
			return retstr;

	_SceneData->dirtyBound();

	retstr = "SMODIF_COMMAND_OK";

	return retstr;
}

bool SceneModifier::switchNamedChildByNode(osg::Node* parent, std::string childname, std::string &retstr)
{
	osg::ref_ptr<osg::Switch> swt = dynamic_cast<osg::Switch*>(parent);

	if(!swt.valid())
	{
		retstr = "SMODIF_CAST_ERROR";
		return false;
	}

	osg::ref_ptr<osg::Node> child = NULL;

	bool found = false;
	for(unsigned int i = 0; i < swt->getNumChildren(); i++)
		if(swt->getChild(i)->getName() == childname) //Switch di tutti i figli con lo stesso nome
		{
			swt->setChildValue(child.get(), !swt->getChildValue(child.get()));
			found = true;
		}
	
	if(!found)
	{
		retstr = "SMODIF_SWITCH_CHILD_NOTFOUND";
		return false;
	}

	_SceneData->dirtyBound();

	return true;
}

std::string SceneModifier::switchChildByName(std::string action)
{
	std::string retstr("SMODIF_BAD_COMMAND");
	std::string nodename, childname;
	
	this->splitActionCommand(action, nodename, childname); 

	Visitors::FindNodeVisitor fnvbn(nodename);
	_SceneData->accept(fnvbn);

	unsigned int number = fnvbn.getNodeFoundSize();

	if(number == 0)
		return "SMODIF_NODE_NOTFOUND";

	for(unsigned int i = 0; i < number; i++) //Switch di tutti i nodi trovati a swtype
		if( !this->switchNamedChildByNode(fnvbn.getNodeByIndex(i).at(fnvbn.getNodeByIndex(i).size() - 1), childname, retstr) )
			return retstr;

	_SceneData->dirtyBound();

	retstr = "SMODIF_COMMAND_OK";

	return retstr;
}

std::string SceneModifier::setMatrixIdentityByName(std::string action)
{
	std::string retstr("SMODIF_BAD_COMMAND");
	
	Visitors::FindNodeVisitor fnvbn(action);
	_SceneData->accept(fnvbn);

	unsigned int number = fnvbn.getNodeFoundSize();

	if(number == 0)
		return "SMODIF_NODE_NOTFOUND";

	for(unsigned int i = 0; i < number; i++)
	{
		osg::ref_ptr<osg::MatrixTransform> mt = dynamic_cast<osg::MatrixTransform*>(fnvbn.getNodeByIndex(i).at(fnvbn.getNodeByIndex(i).size() - 1));
		
		if(!mt.valid())
			return "SMODIF_CAST_ERROR";
		
		this->setMatrixToMatTrans(mt.get(), osg::Matrix::identity());
	}
	
	_SceneData->dirtyBound();

	retstr = "SMODIF_COMMAND_OK";

	return retstr;
}

std::string SceneModifier::setMatrixByName(std::string action)
{
	std::string retstr("SMODIF_BAD_COMMAND");

	std::string nodename, matrixstr;
	
	this->splitActionCommand(action, nodename, matrixstr); 
	
	Visitors::FindNodeVisitor fnvbn(nodename);
	_SceneData->accept(fnvbn);

	unsigned int number = fnvbn.getNodeFoundSize();

	if(number == 0)
		return "SMODIF_NODE_NOTFOUND";

	std::stringstream s(action);
    
    osg::ref_ptr<osgDB::ReaderWriter> reader = osgDB::Registry::instance()->getReaderWriterForExtension(std::string("osg"));
	
	if( !reader.valid() ) 
		return "SMODIF_READERWRITER_ERROR";
	
	osgDB::ReaderWriter::ReadResult res = reader->readObject( s, _options.get() );
	
	if(res.validObject()) 
	{
		osg::ref_ptr<osg::MatrixTransform> matrtransf = static_cast<osg::MatrixTransform *> (res.getObject());

		if(matrtransf.valid())
		{
			for(unsigned int i = 0; i < number; i++)
			{
				osg::ref_ptr<osg::MatrixTransform> mt = dynamic_cast<osg::MatrixTransform*>(fnvbn.getNodeByIndex(i).at(fnvbn.getNodeByIndex(i).size() - 1));
		
				if(!mt.valid())
					return "SMODIF_CAST_ERROR";
			
				this->setMatrixToMatTrans(mt.get(), matrtransf->getMatrix());
			}		
		}
		else
			return "SMODIF_BAD_MATRIXTRANSFORM";
	} 
	else
		return "SMODIF_STREAM_ERROR";

	_SceneData->dirtyBound();

	retstr = "SMODIF_COMMAND_OK";

	return retstr;
}

std::string SceneModifier::setMatrixRotateByName(std::string action)
{
	std::string retstr("SMODIF_BAD_COMMAND");

	double deg, x, y, z;
	std::istringstream instr;
	std::string nodename, multtype, degrees, xstr, ystr, zstr, otherargs;
	
	this->splitActionCommand(action, nodename, otherargs); 
	
	Visitors::FindNodeVisitor fnvbn(nodename);
	_SceneData->accept(fnvbn);

	unsigned int number = fnvbn.getNodeFoundSize();

	if(number == 0)
		return "SMODIF_NODE_NOTFOUND";

	this->splitActionCommand(otherargs, multtype, otherargs);

	if(multtype != "PRE" || multtype != "POST")
		return retstr;

	this->splitActionCommand(otherargs, degrees, otherargs);
	this->splitActionCommand(otherargs, xstr, otherargs);
	this->splitActionCommand(otherargs, ystr, zstr);
	
	if(degrees.empty() || xstr.empty() || ystr.empty() || zstr.empty())
		return retstr;

	instr.str(degrees);
	instr >> deg;

	instr.str(xstr);
	instr >> x;

	instr.str(ystr);
	instr >> y;

	instr.str(zstr);
	instr >> z;

	osg::Matrix newmatr = osg::Matrix::rotate(osg::inDegrees(deg), osg::Vec3(x, y, z));

	for(unsigned int i = 0; i < number; i++)
	{
		osg::ref_ptr<osg::MatrixTransform> mt = dynamic_cast<osg::MatrixTransform*>(fnvbn.getNodeByIndex(i).at(fnvbn.getNodeByIndex(i).size() - 1));
		
		if(!mt.valid())
			return "SMODIF_CAST_ERROR";
		
		if(multtype == "PRE")
			this->preMultMatrixToMatTrans(mt.get(), newmatr);
		else
			this->postMultMatrixToMatTrans(mt.get(), newmatr);
	}

	_SceneData->dirtyBound();

	retstr = "SMODIF_COMMAND_OK";

	return retstr;
}

std::string SceneModifier::setMatrixScaleByName(std::string action)
{
	std::string retstr("SMODIF_BAD_COMMAND");

	double x, y, z;
	std::istringstream instr;
	std::string nodename, multtype, xstr, ystr, zstr, otherargs;
	
	this->splitActionCommand(action, nodename, otherargs); 
	
	Visitors::FindNodeVisitor fnvbn(nodename);
	_SceneData->accept(fnvbn);

	unsigned int number = fnvbn.getNodeFoundSize();

	if(number == 0)
		return "SMODIF_NODE_NOTFOUND";

	this->splitActionCommand(otherargs, multtype, otherargs);

	if(multtype != "PRE" || multtype != "POST")
		return retstr;

	this->splitActionCommand(otherargs, xstr, otherargs);
	this->splitActionCommand(otherargs, ystr, zstr);
	
	if(xstr.empty() || ystr.empty() || zstr.empty())
		return retstr;

	instr.str(xstr);
	instr >> x;

	instr.str(ystr);
	instr >> y;

	instr.str(zstr);
	instr >> z;

	osg::Matrix newmatr = osg::Matrix::scale(osg::Vec3(x, y, z));

	for(unsigned int i = 0; i < number; i++)
	{
		osg::ref_ptr<osg::MatrixTransform> mt = dynamic_cast<osg::MatrixTransform*>(fnvbn.getNodeByIndex(i).at(fnvbn.getNodeByIndex(i).size() - 1));
		
		if(!mt.valid())
			return "SMODIF_CAST_ERROR";
		
		if(multtype == "PRE")
			this->preMultMatrixToMatTrans(mt.get(), newmatr);
		else
			this->postMultMatrixToMatTrans(mt.get(), newmatr);
	}

	_SceneData->dirtyBound();

	retstr = "SMODIF_COMMAND_OK";

	return retstr;
}

std::string SceneModifier::setMatrixTranslateByName(std::string action)
{
	std::string retstr("SMODIF_BAD_COMMAND");

	double x, y, z;
	std::istringstream instr;
	std::string nodename, multtype, xstr, ystr, zstr, otherargs;
	
	this->splitActionCommand(action, nodename, otherargs); 
	
	Visitors::FindNodeVisitor fnvbn(nodename);
	_SceneData->accept(fnvbn);

	unsigned int number = fnvbn.getNodeFoundSize();

	if(number == 0)
		return "SMODIF_NODE_NOTFOUND";

	this->splitActionCommand(otherargs, multtype, otherargs);

	if(multtype != "PRE" || multtype != "POST")
		return retstr;

	this->splitActionCommand(otherargs, xstr, otherargs);
	this->splitActionCommand(otherargs, ystr, zstr);
	
	if(xstr.empty() || ystr.empty() || zstr.empty())
		return retstr;

	instr.str(xstr);
	instr >> x;

	instr.str(ystr);
	instr >> y;

	instr.str(zstr);
	instr >> z;

	osg::Matrix newmatr = osg::Matrix::translate(osg::Vec3(x, y, z));

	for(unsigned int i = 0; i < number; i++)
	{
		osg::ref_ptr<osg::MatrixTransform> mt = dynamic_cast<osg::MatrixTransform*>(fnvbn.getNodeByIndex(i).at(fnvbn.getNodeByIndex(i).size() - 1));
		
		if(!mt.valid())
			return "SMODIF_CAST_ERROR";
		
		if(multtype == "PRE")
			this->preMultMatrixToMatTrans(mt.get(), newmatr);
		else
			this->postMultMatrixToMatTrans(mt.get(), newmatr);
	}

	_SceneData->dirtyBound();

	retstr = "SMODIF_COMMAND_OK";

	return retstr;
}

std::string SceneModifier::getMatrixByName(std::string action)
{
	std::string retstring("SMODIF_BAD_COMMAND");
	std::stringstream s;
	
	Visitors::FindNodeVisitor fnvbn(action);
	_SceneData->accept(fnvbn);

	unsigned int number = fnvbn.getNodeFoundSize();

	if(number == 0)
		return "SMODIF_NODE_NOTFOUND";

	osg::ref_ptr<osg::MatrixTransform> mt = dynamic_cast<osg::MatrixTransform*>(fnvbn.getNodeByIndex(0).at(fnvbn.getNodeByIndex(0).size() - 1));
	
	if(mt.valid())
	{
		osg::ref_ptr<osg::MatrixTransform> newmt = new osg::MatrixTransform;
		newmt->setMatrix(mt->getMatrix());
	  
		osg::ref_ptr<osgDB::ReaderWriter> writer = osgDB::Registry::instance()->getReaderWriterForExtension(std::string("osg"));
	
		if( !writer.valid() ) 
			return retstring;
	
		osgDB::ReaderWriter::WriteResult res = writer->writeObject(*newmt, s, _options.get());
    
		if( res.success() )
			retstring = s.str();
	}
	else
		retstring = "SMODIF_CAST_ERROR";
	
	return retstring;
}

bool SceneModifier::setMatrixToMatTrans(osg::MatrixTransform* mt, osg::Matrix matrix)
{
	if(mt != NULL)
		return false;

	mt->setMatrix(matrix);
	return true;
}

bool SceneModifier::preMultMatrixToMatTrans(osg::MatrixTransform* mt, osg::Matrix matrix)
{
	if(mt == NULL)
		return false;
	
	mt->preMult(matrix);
	return true;
}

bool SceneModifier::postMultMatrixToMatTrans(osg::MatrixTransform* mt, osg::Matrix matrix)
{
	if(mt == NULL)
		return false;
	
	mt->postMult(matrix);
	return true;
}