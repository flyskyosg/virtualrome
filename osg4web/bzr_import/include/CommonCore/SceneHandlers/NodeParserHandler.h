#ifndef __OSG4WEB_NODEPARSERHANDLER__
#define __OSG4WEB_NODEPARSERHANDLER__ 1

#include <CommonCore/Scenehandlers/PickerHandler.h>
#include <CommonCore/Scenehandlers/NodeCommandList.h>



namespace SceneHandlers
{
	/***********************************************************************
	 *
	 * NodeParserHandler class
	 *
	 ***********************************************************************/
	class NodeParserHandler : public PickerHandler
	{
	public:
		//Inizializza il NodeParser Handler
		NodeParserHandler(osg::Node::NodeMask nm = 0xffffffff) : PickerHandler(nm) { }

		//Metodo di gestione del FRAME event
		virtual void callmethodqueue(osgViewer::Viewer* viewer, const osgGA::GUIEventAdapter& ea);
		
	protected:
		//Parsing del NodePath trovato tramite Picker
		virtual void parsingNode();

		// Setta NodeCommand tramite NCommandList index, false altrimenti
		bool getNodeCommand(unsigned int i, NodeCommandList::NodeCommand &ndc);

		// Setta NodeCommand se trova corrispondenza con commandstr, false altrimenti
		bool findNodeCommandByName(std::string commandstr, NodeCommandList::NodeCommand &ndc, NodeCommandList::SearchOptions searchOption = NodeCommandList::DOWN_TO_UP);

		// Setta NodeCommand se trova corrispondenza coi comandi registrati, false altrimenti
		bool findNodeCommandByRegister(NodeCommandList::NodeCommand &ndc, NodeCommandList::SearchOptions searchOption = NodeCommandList::DOWN_TO_UP);

		//Registra un comando per case sul parsing
		void registerCommand(std::string command);

		// Ritorna il numero di elementi in NodeCommandList
		unsigned int getNodeCommandListSize();

	private:
		//NodeCommand List
		NodeCommandList _ndcommlist;
	};
};

#endif //__OSG4WEB_NODEPARSERHANDLER__


