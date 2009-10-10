#ifndef __OSG4WEB_GETBACKHANDLER__
#define __OSG4WEB_GETBACKHANDLER__ 1

#include <CommonCore/RaiseEventInterface.h>
#include <CommonCore/Scenehandlers/NodeParserHandler.h>

#include <string>

namespace SceneHandlers
{

	/***********************************************************************
	 *
	 * GetBackHandler class
	 *
	 ***********************************************************************/
	class GetBackHandler : public NodeParserHandler::CommandStubb {
	public: 
		//Costruttore
		GetBackHandler(CommonCore::RaiseEventInterface* rint) : CommandStubb("raise_to_javascript"), _RaiseInterface(rint) { }

		//Command Bridge ridefinizione
		bool commandBridge(std::string argument, osg::Node* node, osgViewer::Viewer* viewer, const osgGA::GUIEventAdapter& ea);

		bool setJSCallToNode(osg::Node* nd, std::string argument);
	    
	protected:
		//Distruttore
		~GetBackHandler() 
		{
			if (_RaiseInterface)
				_RaiseInterface = NULL;
		}

	private:
		CommonCore::RaiseEventInterface* _RaiseInterface;

	};

};

#endif //__OSG4WEB_GETBACKHANDLER__