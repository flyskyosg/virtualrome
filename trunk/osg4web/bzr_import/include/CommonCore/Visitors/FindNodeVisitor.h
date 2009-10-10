

#include <string>
#include <vector>

#include <osg/Node>
#include <osg/NodeVisitor>



namespace Visitors
{

	/***********************************************************************
	 *
	 * FindNodeVisitor class
	 *
	 ***********************************************************************/

	class FindNodeVisitor : public osg::NodeVisitor
	{
	public:
		//Costruttore
		FindNodeVisitor(std::string nodename) : osg::NodeVisitor(osg::NodeVisitor::TRAVERSE_ACTIVE_CHILDREN),
			_NodeName(nodename),
			_unique(true)
		{ 
			_CheckName.clear();
		}

		//Distruttore
		~FindNodeVisitor()
		{
			_NodeVect.clear();
		}

		void setUniCheck(std::string uname) { _CheckName = uname; }
		
		bool isUnique() { return _unique; }

		//Apply sui nodi 
		virtual void apply(osg::Node& node) //FIXME: Check di Object ?
		{
			//Check di unicità. Se il nome nodo attraversato == nome settato unique = false;
			if (!_CheckName.empty() && _CheckName == node.getName())
				_unique = false;

			if (node.getName() == _NodeName)
				_NodeVect.push_back(this->getNodePath());
			
			traverse(node);
		}

		//Ritorna il numero di nodi trovati
		unsigned int getNodeFoundSize() { return _NodeVect.size(); }

		//Ritorna il nodo tramite posizione di indice del vettore
		osg::NodePath getNodeByIndex(unsigned int i) { return _NodeVect.at(i); }

		//Ritorna il nome dei nodi cercati
		std::string getNodeName() { return  _NodeName; }

	protected:
		//Vettori di nodi Found
		std::vector< osg::NodePath > _NodeVect;
		//Nome nodo
		std::string _NodeName;
		
		//Check Unicità
		bool _unique;

		std::string _CheckName;
	};

};
