

#include <CommonCore/Visitors/AttachNodeVisitor.h>

using namespace Visitors;

/***********************************************************************
 *
 * AttachNodeVisitor class
 *
 ***********************************************************************/

/** Costruttore */
AttachNodeVisitor::AttachNodeVisitor(std::string nodename, osg::Node* node, bool multipleattach) : osg::NodeVisitor(osg::NodeVisitor::TRAVERSE_ACTIVE_CHILDREN),
	_nodeName(nodename),
	_attachNode(node),
	_attached(false),
	_multipleattach(multipleattach)
{

}

/** Attacca al/ai nodi che corrispondono a nodeName */
void AttachNodeVisitor::apply(osg::Group& grp)
{
	if (grp.getName() == _nodeName)
	{
		if(!_attached || _multipleattach)
		{
			grp.addChild(_attachNode.get());
			_attached = true;
		}
	}

	traverse(grp);
}