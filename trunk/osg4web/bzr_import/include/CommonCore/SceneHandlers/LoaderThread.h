#ifndef __OSG4WEB_LOADERTHREAD__
#define __OSG4WEB_LOADERTHREAD__ 1

#include <iostream>

#include <OpenThreads/Thread>
#include <OpenThreads/Mutex>

#include <osg/Referenced>
#include <osg/Node>
#include <osg/Version>
#if (OPENSCENEGRAPH_SOVERSION > 51)
	#include <osgDB/Options>
#endif

#include <osgDB/ReaderWriter>
#include <osgUtil/Optimizer>


namespace SceneHandlers
{

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

};

#endif //__OSG4WEB_LOADERTHREAD__