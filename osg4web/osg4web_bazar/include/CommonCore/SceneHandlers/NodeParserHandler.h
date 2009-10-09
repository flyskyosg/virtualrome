#ifndef __OSG4WEB_NODEPARSERHANDLER__
#define __OSG4WEB_NODEPARSERHANDLER__ 1

#include <CommonCore/Scenehandlers/PickerHandler.h>


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
		/***********************************************************************
		 *
		 * CommandStubb class
		 *
		 ***********************************************************************/
		class CommandStubb : public osg::Referenced
		{
		public:
			//Costruttore
			CommandStubb() {}
			//Costruttore definisce il comando accettato
			CommandStubb(std::string cmd) : _command(cmd) {}
			//Set Comando
			void setCommand(std::string cmd) { _command = cmd; }
			//Get Comando
			std::string getCommand() { return _command; }
			//Funzione stubb da ridefinire per gestire il comando 
			virtual bool commandBridge(std::string argument, osg::Node* node, osgViewer::Viewer* viewer, const osgGA::GUIEventAdapter& ea) { return false; }
		protected:
			std::string _command;
		};

		//Inizializza il NodeParser Handler
		NodeParserHandler(osg::Node::NodeMask nm = 0xffffffff) : PickerHandler(nm) { }
		//Aggiunge un comando accettato durante un evento
		bool addCommand(osgGA::GUIEventAdapter::EventType evt, NodeParserHandler::CommandStubb* stubb);
		
	protected:
		/***********************************************************************
		 *
		 * EventCommandRegister class
		 *
		 ***********************************************************************/
		class EventCommandRegister
		{
		public:
			//Costruttore
			EventCommandRegister(osgGA::GUIEventAdapter::EventType evt) : _eventType(evt) {}
			//Distruttore
			~EventCommandRegister();

			//Ritorna l'evento gestito
			osgGA::GUIEventAdapter::EventType getEvent() { return _eventType; }
			//aggiunge il comando all'evento
			bool addCommand(NodeParserHandler::CommandStubb* stubb);
			//Ritorna lo stubb in base all'ID
			NodeParserHandler::CommandStubb* getStubbById(unsigned int id) { return _registeredStubb.at(id).get(); }
			//Ritorna il numero di comandi associati
			unsigned int getStubbVectorSize() { return _registeredStubb.size(); }
			//Ritorna il vettore dei comandi associati
			std::vector< osg::ref_ptr<NodeParserHandler::CommandStubb> > getStubbVector() { return _registeredStubb; }
			
		private:
			//Evento 
			osgGA::GUIEventAdapter::EventType _eventType;
			//Stubb registrati per l'evento
			std::vector< osg::ref_ptr<NodeParserHandler::CommandStubb> > _registeredStubb;
		};

		//Metodo di gestione degli eventi 
		virtual void callmethodqueue(osgViewer::Viewer* viewer, const osgGA::GUIEventAdapter& ea);
		//Splitta stringa di ingresso tramite delimiter
		virtual void splitCommandArgument(std::string command, std::string& largs, std::string& rargs, std::string delimiter = " ");
		//
		bool handleNodeDescriptions(osg::Node* node, osgViewer::Viewer* viewer, const osgGA::GUIEventAdapter& ea);

	private:
		//Event Registered
		std::vector< NodeParserHandler::EventCommandRegister > _registeredEvents;
	};
};

#endif //__OSG4WEB_NODEPARSERHANDLER__


