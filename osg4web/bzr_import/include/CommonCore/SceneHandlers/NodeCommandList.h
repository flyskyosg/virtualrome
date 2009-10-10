#ifndef __OSG4WEB_NODECOMMANDLIST__
#define __OSG4WEB_NODECOMMANDLIST__ 1

#include <vector>
#include <iostream>

#include <osg/Node>



namespace SceneHandlers
{
	/*******************************************
	 *
	 * NodeCommandList Class
	 *
	 *******************************************/

	class NodeCommandList
	{
	public:

		/*******************************************
		 *
		 * NodeCommand Class
		 *
		 *******************************************/
		class NodeCommand
		{
		public:
			//Costruisce un NodeCommand Vuoto
			NodeCommand() : _node(NULL) { }
			//Costruisce un NodeCommand settando il nodo
			NodeCommand(osg::Node* node, osg::NodePath::iterator npitr) : _node(node), _npitr(npitr) { }
			//Distruttore
			~NodeCommand();

			// Ritorna il nodo associato valido
			osg::Node* getNode();
			// Ritorna il nodo associato valido
			osg::NodePath::iterator getNodePathIterator() { return _npitr; }
			// Ritorna il comando nella description list, false se nodo non valido
			bool getCommandInDescList(std::string &nodecommand);
			// Ritorna la description list del node
			bool getDescList(std::vector<std::string> &desclist);
			// Ritorna la description list del node senza la prima entry di comando
			bool getDescListWithoutCommand(std::vector<std::string> &desclist);
			//Controlla la validità del Nodo
			bool isEmptyCommand();
			
			//Setta il node di comando ed il NodePath iterator relativo
			void setNode(osg::Node* node, osg::NodePath::iterator npitr);
		
		private: 
			osg::ref_ptr<osg::Node> _node;

			osg::NodePath::iterator _npitr;
		};


		//Search Method FLAG
		enum SearchOptions 
		{
			DOWN_TO_UP = 0,
			UP_TO_DOWN,
		};

		//Aggiunge un NodeCommand alla Lista
		virtual void addNodeCommand(NodeCommand nd); //TIPS: questa funzione può essere redifinita per fare il checking di validità dei comandi ---
		//Crea un NodeCommand e lo aggiunge alla Lista
		void addNodeCommand(osg::Node* node, osg::NodePath::iterator npitr);

		//Setta NodeCommand se esiste il command string
		bool findNodeCommand(std::string commandstr, NodeCommandList::NodeCommand &ndc, NodeCommandList::SearchOptions searchOption = NodeCommandList::DOWN_TO_UP);
		//Setta il primo NodeCommand incontrato sul NodePath partendo dalla foglia
		bool findNodeCommand(NodeCommandList::NodeCommand &ndc, NodeCommandList::SearchOptions searchOption = NodeCommandList::DOWN_TO_UP);
		//Setta NodeCommand direttamente tramite index della lista
		bool getNodeCommand(unsigned int i, NodeCommandList::NodeCommand &ndc);
		//Setta command string direttamente tramite index di lista
		bool getNodeCommandName(unsigned int i, std::string &commandstr);
		//Setta command string direttamente tramite index di lista
		bool getNodeCommandDescList(unsigned int i, std::vector<std::string> &commanddesclist);
		//Setta command string direttamente tramite index di lista, elimina str di comando
		bool getNodeCommandDescListWithoutCommand(unsigned int i, std::vector<std::string> &commanddesclist);
		//Ritorna nodo direttamente tramite index di lista
		osg::Node* getNodeCommandNode(unsigned int i);

		//Ritorna il numero di comandi nella lista
		unsigned int getNodeCommandListSize() { return _nodecommandlist.size(); }
		//Azzera la lista di comandi
		void clearNodeCommandList() { _nodecommandlist.clear(); }
		//Controlla se la lista è vuota
		bool isEmptyNodeCommandList() { return _nodecommandlist.empty(); }

		//Registra i comandi accettati dal parser
		void registerCommandInCommandList(std::string command);
		//Pulisce i comandi accettati dal parser
		void clearCommandList();

	private:
		//Vettore dei NodeCommand trovati nell'attraversamento
		std::vector<NodeCommand> _nodecommandlist;

		//Vettore dei comandi registrati
		std::vector<std::string> _registeredCommandList;
	};

};

#endif //__OSG4WEB_NODECOMMANDLIST__