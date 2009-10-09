
#include <CommonCore/Scenehandlers/LoaderThread.h>

#include <osgDB/ReadFile>


using namespace SceneHandlers;

/***********************************************************************
 *
 * LoaderThread class
 *
 ***********************************************************************/

/** Costruttore */
LoaderThread::LoaderThread(std::string fname, osgDB::ReaderWriter::Options *opt) : _filename(fname),
	_loadingOptions(opt),
	_nodeLoaded(NULL),
	_optType(osgUtil::Optimizer::DEFAULT_OPTIMIZATIONS),
	_optimize(false),
	_failed(false),
	_done(false)
{
	
}

/** Distruttore */
LoaderThread::~LoaderThread()
{
	cancel();
	clear();
}

/** Setta le Opzioni di Caricamento applicate al loader del modello*/
void LoaderThread::setLoadingOptions(osgDB::ReaderWriter::Options *opt)
{
	_loadingOptions = opt;
}

/** Setta il tipo di Ottimizzazione da applicare al modello caricato */
void LoaderThread::setOptimization(osgUtil::Optimizer::OptimizationOptions optType)
{
	_optimize = true;
	_optType = optType; 
}

/** Pulisco le strutture della classe */
int LoaderThread::clear()
{
	_filename.clear();

	if(_nodeLoaded.valid())
		_nodeLoaded = NULL;

	if(_loadingOptions.valid())
		_loadingOptions = NULL;

	return 0;
}

/** Funzione RUN del Thread */
void LoaderThread::run()
{
	osg::ref_ptr<osg::Node> lnode = osgDB::readNodeFile(_filename, _loadingOptions.get());

	if(! _done)
	{
		if(lnode.valid())
		{
			if(_optimize)
			{
				osgUtil::Optimizer optimizer;
				optimizer.optimize(lnode.get(), _optType);
				optimizer.reset();
			}
		
			//Setto il nome del nodo
			lnode->setName(_filename);
			_nodeLoaded = lnode.get();
		}
		else
			_failed = true;
	}
	
	_done = true;
}

/** Cancella l'istanza di caricamento*/
int LoaderThread::cancel()
{
	int result = 0; //FIXME: capire se x questi Thread bisogna fare join
	if( isRunning() )
	{
		_done = true;

		//TODO: capire se mettere mutex... in caso resttarle qui

		//Attendo che il Thread venga rilasciato dal sistema
        while(isRunning())
            OpenThreads::Thread::YieldCurrentThread();
	}
	
	return result;
}

osg::Node* LoaderThread::getNodeLoaded()
{
	if(_nodeLoaded.valid() && _done)
	{
		//In questo modo posso rilasciare il nodo ed impostarlo a NULL senza far deallocare dal ref_ptr
		osg::ref_ptr<osg::Node> retnode = _nodeLoaded.get();
		_nodeLoaded = NULL;

		return retnode.release();
	}
	else
		return NULL;
}

void LoaderThread::setNodeLoaded(osg::Node* node)
{
	_nodeLoaded = node;
	_done = true;
}