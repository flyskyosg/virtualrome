#ifndef __OSG4WEB_LOADTHREADSHANDLER__
#define __OSG4WEB_LOADTHREADSHANDLER__ 1


#include <iostream>

#include <osg/Node>
#include <osg/Switch>

#include <osgDB/Registry>
#include <osgDB/ReaderWriter>

#include <osgUtil/Optimizer>

namespace SceneHandlers
{
	/***********************************************************************
	 *
	 * LoaderThread class
	 *
	 ***********************************************************************/

	class LoadThreadsHandler : public osg::Referenced
	{
	public:

		/***********************************************************************
		 *
		 * LoadThreadsHandler::AttachNodeToSceneByName class
		 *
		 ***********************************************************************/

		class AttachNodeToSceneByName : public osg::NodeVisitor
		{
		public:
			//Costruttore
			AttachNodeToSceneByName(std::string nodename, osg::Node* node, bool multipleattach = false);

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


		/***********************************************************************
		 *
		 * LoadThreadsHandler::LoadingMarkTransformCallback class
		 *
		 ***********************************************************************/

		class LoadingMarkTransformCallback : public osg::NodeCallback
		{
		public:
			//Crea la callback di rotazione del loading Mark
			LoadingMarkTransformCallback();
			
			//Apply sulla position Attitude transform
			virtual void operator()(osg::Node* node, osg::NodeVisitor* nv);

		protected:
			double _firstTime;
			double _period;
		};


		/*******************************************************************
		 *
		 * LoadThreadsHandler::RequestFile class
		 *
		 *******************************************************************/

		class RequestFile : public osg::Referenced
		{
		public:
			//Costruttore
			RequestFile(std::string filename, std::string arguments, osgDB::ReaderWriter::Options *options);

			//Ritorna il nome del file
			std::string getFileName() { return _filename; }
			const std::string getFileName() const { return _filename; }

			//Ritorna gli argomenti aggiuntivi passati alla richiesta
			std::string getArguments() { return _arguments; }
			const std::string getArguments() const { return _arguments; }

			//Ritorna le options di loading
			osgDB::ReaderWriter::Options* getLoadingOptions() { return _options.get(); }

			//Il file è in caricamento
			void setLoading() { _loading = true; }
			//True se la richiesta è in corso di loading, false altrimenti
			bool isLoading() { return _loading; }

			//Operatori di confronto
			inline bool operator == (const RequestFile& reqfile) const { return _filename == reqfile.getFileName() && _arguments == reqfile.getArguments(); } //Options lo lascio fuori
			inline bool operator != (const RequestFile& reqfile) const { return  _filename != reqfile.getFileName() || _arguments != reqfile.getArguments(); } //Options lo lascio fuori

		protected:
			//Distruttore
			~RequestFile();

			//Nome file da caricare
			std::string _filename;
			//Nome file da caricare
			std::string _arguments;
			//Opzioni di caricamento
			osg::ref_ptr<osgDB::ReaderWriter::Options> _options;
			
			//Specifica se la richiesta è in attesa o è in loading
			bool _loading;
		};


		/***********************************************************************
		*
		 * LoaderThread class
		 *
		 ***********************************************************************/

		class LoaderThread :  public OpenThreads::Thread, public osg::Referenced
		{
		public:
			//Costruttore
			LoaderThread(std::string fname, osgDB::ReaderWriter::Options *opt = new osgDB::ReaderWriter::Options());
		
			//Setta il file da scaricare
			void setFileToLoad(std::string fname) { _filename = fname; }
			
			//Ritorna il nome del file da scaricare
			std::string getFileToLoad() { return _filename; }

			//Setta Opzioni di Loading
			void setLoadingOptions(osgDB::ReaderWriter::Options *optstr);

			//Setta il tipo di Ottimizzazione da applicare al modello caricato
			void setOptimization(osgUtil::Optimizer::OptimizationOptions optType = osgUtil::Optimizer::DEFAULT_OPTIMIZATIONS);

			//Ritorna il nodo caricato	
			osg::Node* getNodeLoaded();

			//Setta il nodo caricato per creare i fake Loader. Da usare solo con la cache dei nodi
			void setNodeLoaded(osg::Node* node);

			//True se il LoaderThread ha finito di caricare il file
			bool isDone() { return _done; }

			//True se il LoaderThread ha finito di caricare il file
			bool isFailed() { return _failed; }

			//Parte il Download del File        
			virtual void run();
	        
			//Cancella l'istanza di Download   
			virtual int cancel();

		protected:
			//Distruttore
			~LoaderThread();

			//Cancella l'istanza di Download   
			virtual int clear();

			//Nome del file da caricare
			std::string _filename;

			//Opzioni di loading
			osg::ref_ptr<osgDB::ReaderWriter::Options> _loadingOptions;

			//Node loaded
			osg::ref_ptr<osg::Node> _nodeLoaded;

			//Controllo di ottimizzazione
			bool _optimize;

			//Metodo di ottimizzazione del modello caricato
			osgUtil::Optimizer::OptimizationOptions _optType;

			//Controlla se il LoaderThread ha compiuyo il suo compito
			bool _done;

			//Loading Failed
			bool _failed;
		};

		
		//Costruttore
		LoadThreadsHandler();
		
		//Richiede il caricamento di un file
		bool requestLoading(std::string filename);
		bool requestLoading(std::string filename, std::string arguments, osgDB::ReaderWriter::Options * opt = NULL);

		//Controlla se _lo stack di Thread è in caricamento
		bool isLoading();

		//Motore di loading e controllo delle strutture
		osg::Node* handleLoading();

		//Rileva errori di caricamento e ritorna la lista dei file non caricati
		bool handleLoadingErrors(std::vector<std::string>& name);

		//Cancella tutti i loading correnti e le richieste associate
		bool cancelAllLoading();

		//Metodo di Clean delle richiesta contramite nome
		bool cancelLoadingByFileName(std::string filename);

		//Crea il Loading Mark
		osg::Node* createLoadingHUD();

		//Setta lo SceneGraph sul quale ricercare i nodi
		void setNode(osg::Node* node) { _mainNode = node; }

		//Setta l'ottimizzatore dei modelli caricati e il metodo di ottimizzazione
		void setOptimization(bool opt, osgUtil::Optimizer::OptimizationOptions optType = osgUtil::Optimizer::DEFAULT_OPTIMIZATIONS);
		
	protected:
		//Distruttore
		~LoadThreadsHandler();

		//Pulisco lo strutture della classe, richieste e threads attivi
		bool clear();

		//pulisco la cache dei nodi caricati
		bool clearNodeCache();

		//Refresh del Loading Mark
		void refreshLoadingMark();

		//Refresh delle strutture di Loading
		void refreshLoadingQueue();

		//Crea il Loading Mark
		osg::Node* createLoadingMark();

		//Turn On/Off Loading Mark
		void switchLoadingMarkOn() { _LoadingMarkSwitch->setAllChildrenOn(); }
		void switchLoadingMarkOff() { _LoadingMarkSwitch->setAllChildrenOff(); }

		//Ritorna lo stato del Loading Mark
		bool getLoadingMarkState() { return _LoadingMarkSwitch->getValue(0); }

		//Ritorna il primo RequestFile disponibile dalla coda di richieste
		LoadThreadsHandler::RequestFile* findFirstRequestAvailable();

		//Ritorna il numero di Loading Correnti
		unsigned int getCurrentLoadings();

		//Divide il comando in command e argument
		void splitCommand(std::string command, std::string& largs, std::string& rargs, const char split = ' ');

	protected:
		//Handle the loading Message
		osg::ref_ptr<osg::Switch> _LoadingMarkSwitch;

		//Vettore dei Thread di Loading (con definizione)
		typedef std::vector< osg::ref_ptr< LoadThreadsHandler::LoaderThread > > LoaderThreadStack;
		LoaderThreadStack _loaderThreadsStack;

		//Lista dei file messi in coda per il caricamento (con definizione)
		typedef std::list< osg::ref_ptr< RequestFile > > RequestQueue;
		RequestQueue _requestQueue;

		//Vettore dei nodi caricati (Cache)
		typedef std::vector< osg::ref_ptr< osg::Node > > LoadedNodeStack;
		LoadedNodeStack _loadedNodeStack;

		//Definisce il massimo numero di Thread di loading per Stack
		unsigned int _maxThreadPerStack;

		//Nodo di Scena
		osg::ref_ptr<osg::Node> _mainNode;

		//Mantiene il tipo di ottimizzazione
		bool _optimizeModels;
		osgUtil::Optimizer::OptimizationOptions _optType;

		//Risoluzione del Camera 2D
		float _xMaxResolution, _yMaxResolution;
	};
};

#endif //__OSG4WEB_LOADTHREADSHANDLER__