

#include <string>
#include <vector>

#include <osg/Node>
#include <osg/Group>
#include <osg/NodeVisitor>


namespace Visitors
{

	/***********************************************************************
	 *
	 * AttachNodeVisitor class
	 *
	 ***********************************************************************/

	class AttachNodeVisitor : public osg::NodeVisitor
	{
	public:
		//Costruttore
		AttachNodeVisitor(std::string nodename, osg::Node* node, bool multipleattach = false);

		//Attacca al/ai nodi che corrispondono a nodeName
		virtual void apply(osg::Group& grp);

		//Ritorna true se il nodo è stato attaccato
		bool isNodeFound() { return _attached;}

	protected:
		//Definisce se il nodo va attaccato a tutti i gruppi corrispondenti a nodeName o solo al primo trovato
		bool _multipleattach;
		//Condizione di ritrovamento
		bool _attached;
		
		//Node Name da trovare
		std::string _nodeName;

		//Nodo da attaccare
		osg::ref_ptr<osg::Node> _attachNode;
	};

};