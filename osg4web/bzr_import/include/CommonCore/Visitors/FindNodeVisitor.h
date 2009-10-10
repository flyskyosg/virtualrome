#ifndef __OSG4WEB_FINDNODEVISITORS__
#define __OSG4WEB_FINDNODEVISITORS__ 1

#include <string>
#include <vector>

#include <osg/Node>
#include <osg/Geode>
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


	/***********************************************************************
	 *
	 * FindGeodeVisitor class
	 *
	 ***********************************************************************/

	class FindGeodeVisitor : public osg::NodeVisitor
	{
	public:
		//Costruttore
		FindGeodeVisitor(std::string geodename) : osg::NodeVisitor(osg::NodeVisitor::TRAVERSE_ACTIVE_CHILDREN),
			_GeodeName(geodename),
			_unique(true)
		{ 
			_CheckName.clear();
		}

		//Distruttore
		~FindGeodeVisitor()
		{
			_GeodeVect.clear();
		}

		void setUniCheck(std::string uname) { _CheckName = uname; }
		
		bool isUnique() { return _unique; }

		//Apply sui nodi 
		virtual void apply(osg::Geode& geode) //FIXME: Check di Object ?
		{
			//Check di unicità. Se il nome nodo attraversato == nome settato unique = false;
			if (!_CheckName.empty() && _CheckName == geode.getName())
				_unique = false;

			if (geode.getName() == _GeodeName)
				_GeodeVect.push_back(this->getNodePath());
			
			traverse(geode);
		}

		//Ritorna il numero di nodi trovati
		unsigned int getGeodeFoundSize() { return _GeodeVect.size(); }

		//Ritorna il nodo tramite posizione di indice del vettore
		osg::NodePath getGeodeByIndex(unsigned int i) { return _GeodeVect.at(i); }

		//Ritorna il nome dei nodi cercati
		std::string getGeodeName() { return  _GeodeName; }

	protected:
		//Vettori di NodePath trovati
		std::vector< osg::NodePath > _GeodeVect;
		//Nome Geode da cercare
		std::string _GeodeName;
		
		//Check Unicità
		bool _unique;

		std::string _CheckName;
	};

};



#endif //__OSG4WEB_FINDNODEVISITORS__