
#include <CommonCore/Scenehandlers/LoadThreadsHandler.h>
#include <CommonCore/Visitors/FindNodeVisitor.h>


#include <osg/Geode>
#include <osg/MatrixTransform>
#include <osg/ShapeDrawable>
#include <osg/PositionAttitudeTransform>

#include <osgDB/ReadFile>

#include <osgSim/LightPointNode>


using namespace SceneHandlers;


/***********************************************************************
 *
 * LoadThreadsHandler::AttachNodeToSceneByName class
 *
 ***********************************************************************/

/** Costruttore */
LoadThreadsHandler::AttachNodeToSceneByName::AttachNodeToSceneByName(std::string nodename, osg::Node* node, bool multipleattach) : osg::NodeVisitor(osg::NodeVisitor::TRAVERSE_ACTIVE_CHILDREN),
	_nodeName(nodename),
	_attachNode(node),
	_attached(false),
	_multipleattach(multipleattach)
{

}

/** Attacca al/ai nodi che corrispondono a nodeName */
void LoadThreadsHandler::AttachNodeToSceneByName::apply(osg::Group& grp)
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

/***********************************************************************
 *
 * LoadThreadsHandler::LoadingMarkTransformCallback class
 *
 ***********************************************************************/

/** Crea la callback di rotazione del loading Mark */
LoadThreadsHandler::LoadingMarkTransformCallback::LoadingMarkTransformCallback() : _firstTime(0.0),
	_period(0.64f)
{

}

/** Apply sulla Position Attitude Transform */
void LoadThreadsHandler::LoadingMarkTransformCallback::operator()(osg::Node* node, osg::NodeVisitor* nv)
{
	osg::PositionAttitudeTransform* pat = dynamic_cast<osg::PositionAttitudeTransform*>(node);
	const osg::FrameStamp* frameStamp = nv->getFrameStamp();
	if (pat && frameStamp)
	{
		if (_firstTime==0.0) 
			_firstTime = frameStamp->getSimulationTime();
		
		double phase = (frameStamp->getSimulationTime() - _firstTime) / _period;
		phase -= floor( phase );
		phase *= (2.0 * osg::PI);
            
		osg::Quat rotation;
		//Creo la rotazione
		rotation.makeRotate(phase,0.0f,1.0f,0.0f);

		pat->setAttitude(rotation); 
	}

	// must traverse the Node's subgraph            
	traverse(node,nv);
}



/*******************************************************************
 *
 * LoadThreadsHandler::RequestFile class
 *
 *******************************************************************/

/** Costruttore */
LoadThreadsHandler::RequestFile::RequestFile(std::string filename, std::string arguments, osgDB::ReaderWriter::Options *options) : _filename(filename),
	_arguments(arguments),
	_options(options),
	_loading(false)
{

}

/** Distruttore */
LoadThreadsHandler::RequestFile::~RequestFile()
{
	_loading = false;

	_filename.clear();
	_arguments.clear();

	if(_options.valid())
		_options = NULL;
}


/***********************************************************************
 *
 * LoadThreadsHandler::LoaderThread class
 *
 ***********************************************************************/

/** Costruttore */
LoadThreadsHandler::LoaderThread::LoaderThread(std::string fname, osgDB::ReaderWriter::Options *opt) : _filename(fname),
	_loadingOptions(opt),
	_nodeLoaded(NULL),
	_optType(osgUtil::Optimizer::DEFAULT_OPTIMIZATIONS),
	_optimize(false),
	_failed(false),
	_done(false)
{
	
}

/** Distruttore */
LoadThreadsHandler::LoaderThread::~LoaderThread()
{
	cancel();
	clear();
}

/** Setta le Opzioni di Caricamento applicate al loader del modello*/
void LoadThreadsHandler::LoaderThread::setLoadingOptions(osgDB::ReaderWriter::Options *opt)
{
	_loadingOptions = opt;
}

/** Setta il tipo di Ottimizzazione da applicare al modello caricato */
void LoadThreadsHandler::LoaderThread::setOptimization(osgUtil::Optimizer::OptimizationOptions optType)
{
	_optimize = true;
	_optType = optType; 
}

/** Pulisco le strutture della classe */
int LoadThreadsHandler::LoaderThread::clear()
{
	_filename.clear();

	if(_nodeLoaded.valid())
		_nodeLoaded = NULL;

	if(_loadingOptions.valid())
		_loadingOptions = NULL;

	return 0;
}

/** Funzione RUN del Thread */
void LoadThreadsHandler::LoaderThread::run()
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
int LoadThreadsHandler::LoaderThread::cancel()
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

osg::Node* LoadThreadsHandler::LoaderThread::getNodeLoaded()
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

void LoadThreadsHandler::LoaderThread::setNodeLoaded(osg::Node* node)
{
	_nodeLoaded = node;
	_done = true;
}



/***********************************************************************
 *
 * LoaderThreadHandler class
 *
 ***********************************************************************/

/** Costruttore */
LoadThreadsHandler::LoadThreadsHandler() : _maxThreadPerStack(3),
	_LoadingMarkSwitch(new osg::Switch),
	_xMaxResolution(500), //TODO:
	_yMaxResolution(500), //TODO:
	_optimizeModels(false),
	_optType(osgUtil::Optimizer::DEFAULT_OPTIMIZATIONS),
	_mainNode(NULL)
{

}

/** Distruttore */
LoadThreadsHandler::~LoadThreadsHandler()
{
	clear();

	if(_LoadingMarkSwitch.valid())
		_LoadingMarkSwitch = NULL;

	if(_mainNode.valid())
		_mainNode = NULL;
}

/** Crea l'HUD di Loading */
osg::Node* LoadThreadsHandler::createLoadingHUD()
{
	osg::ref_ptr<osg::Camera> camera = new osg::Camera;
	osg::ref_ptr<osg::MatrixTransform> loadmmt = new osg::MatrixTransform;
	
	//Inizializzo la camera di proiezione 2D
	camera->setName("LoadThreadsHandle_Loading_Mark_HUD_Camera 2D");
	camera->setProjectionMatrix(osg::Matrix::ortho2D(0, _xMaxResolution, 0, _yMaxResolution)); 
	camera->setReferenceFrame(osg::Transform::ABSOLUTE_RF);
	camera->setViewMatrix(osg::Matrix::identity());
	camera->setClearMask(GL_DEPTH_BUFFER_BIT);
	camera->setRenderOrder(osg::Camera::POST_RENDER);
	camera->setAllowEventFocus(false);

	loadmmt->setName("Matrice_di_posizione_Loading_Mark");
	loadmmt->setMatrix( osg::Matrix::identity() );//osg::Matrix::scale(osg::Vec3(5.0,5.0,1.0))); //FIXME: riposizionare

	//Switch su Camera 2D
	camera->addChild(loadmmt.get());
	loadmmt->addChild(_LoadingMarkSwitch.get());

	_LoadingMarkSwitch->addChild(this->createLoadingMark(), false);

	return camera.release(); // _LoadingMarkSwitch.get();
}

/** Crea il nodo Loading Mark */
osg::Node* LoadThreadsHandler::createLoadingMark()
{
	osg::ref_ptr<osg::MatrixTransform> matrtr = new osg::MatrixTransform;
	osg::ref_ptr<osg::PositionAttitudeTransform> pat = new osg::PositionAttitudeTransform;
	osg::ref_ptr<osg::Geode> rotatingeode = new osg::Geode();

	//Disabilito il modello di illuminazione per i tooltip
	osg::ref_ptr<osg::StateSet> globstateset = rotatingeode->getOrCreateStateSet();
	globstateset->setMode(GL_LIGHTING,osg::StateAttribute::OFF);

	float radius = 0.5f;
    float height = 1.0f;
    
	osg::ref_ptr<osg::TessellationHints> hints = new osg::TessellationHints;
    hints->setDetailRatio(0.5f);

	osg::ref_ptr<osg::ShapeDrawable> spherered = new osg::ShapeDrawable(new osg::Sphere(osg::Vec3(1.0, -0.1f, 0.0),radius),hints.get());
	osg::ref_ptr<osg::ShapeDrawable> spheregreen = new osg::ShapeDrawable(new osg::Sphere(osg::Vec3(cosf(osg::PI_2 + (1.0 / 3.0) * osg::PI_2), -0.1f, sinf(osg::PI_2 + (1.0 / 3.0) * osg::PI_2)),radius),hints.get());
	osg::ref_ptr<osg::ShapeDrawable> sphereblue = new osg::ShapeDrawable(new osg::Sphere(osg::Vec3(cosf(- osg::PI_2 - (1.0 / 3.0) * osg::PI_2), -0.1f, sinf(- osg::PI_2 - (1.0 / 3.0) * osg::PI_2)),radius),hints.get());

	spherered->setColor(osg::Vec4(1.0f,0.12f,0.06f,0.5f));
	spheregreen->setColor(osg::Vec4(0.21f,0.48f,0.03f,0.5f));
	sphereblue->setColor(osg::Vec4(0.20f,0.45f,0.60f,0.5f));

	rotatingeode->addDrawable(spherered.get());
	rotatingeode->addDrawable(spheregreen.get());
	rotatingeode->addDrawable(sphereblue.get());
	
	pat->setPivotPoint(osg::Vec3(0.0,-0.1,0.0));
	pat->setUpdateCallback(new LoadThreadsHandler::LoadingMarkTransformCallback());
	pat->addChild(rotatingeode.get());

	matrtr->setMatrix(osg::Matrix::rotate(osg::inDegrees(-90.0),osg::Vec3(1.0,0.0,0.0)) * osg::Matrix::scale(osg::Vec3(_xMaxResolution * 0.015, _xMaxResolution * 0.015, 0.0)) * osg::Matrix::translate(osg::Vec3(_xMaxResolution * 0.04, _yMaxResolution * 0.96,0.0))); //TODO: mettere la trasformazione corretta
	matrtr->addChild(pat.get());
	
	return matrtr.release();
}

/** Setta il metodo di ottimizzazione per i modelli caricati */
void LoadThreadsHandler::setOptimization(bool opt, osgUtil::Optimizer::OptimizationOptions optType)
{
	_optimizeModels = opt;
	_optType = optType;
}

/** True se ci sono thread di loading attivi */
bool LoadThreadsHandler::isLoading()
{
	LoaderThreadStack::iterator itr;
	
	//Controllo nei Thread istanziati
	for(itr = _loaderThreadsStack.begin(); itr != _loaderThreadsStack.end(); itr++)
	{
		//Se ho un Thread istanziato ma non è ancora in Done allora sta caricando
		if(!(*itr)->isDone()) 
			return true;
	}

	return false;
}

/** Mette in coda una richiesta per il Download */
bool LoadThreadsHandler::requestLoading(std::string filename)
{
	return this->requestLoading(filename, std::string(""));
}

/** Mette in coda una richiesta per il Download */
bool LoadThreadsHandler::requestLoading(std::string filename, std::string arguments, osgDB::ReaderWriter::Options * opt)
{
	osg::ref_ptr<RequestFile> currRequest = new RequestFile(filename, arguments, opt);

	//Se è già in coda elimino la richiesta
	RequestQueue::iterator itr;
	for(itr = _requestQueue.begin(); itr != _requestQueue.end(); itr++) //FIXME: request QUEUE va messa in mutal exclusion
		if((*itr).get() == currRequest.get())
			return false;

	//Metto in coda la richiesta
	_requestQueue.push_back(currRequest);

	return true;
}

/** Ritorna il numero di loading simultanei al momento */
unsigned int LoadThreadsHandler::getCurrentLoadings()
{
	unsigned int ret = 0;

	//Calcolo il numero di load contemporanei
	RequestQueue::iterator itr;
	for(itr = _requestQueue.begin(); itr != _requestQueue.end(); itr++) //FIXME: request QUEUE va messa in mutal exclusion
		if((*itr)->isLoading())
			ret++;

	return ret;
}

/** Killa tutti i loading correnti e uccide i thread associati */
bool LoadThreadsHandler::cancelAllLoading()
{
	std::vector<std::string> reqloadingvec;

	LoaderThreadStack::iterator itr = _loaderThreadsStack.begin();
	while( itr != _loaderThreadsStack.end() )
	{
		LoaderThreadStack::iterator next = itr;

		++next;

		reqloadingvec.push_back((*itr)->getFileToLoad());
		(*itr)->cancel();

		_loaderThreadsStack.erase(itr);

		itr = next;
	}

	RequestQueue::iterator rqitr = _requestQueue.begin();
	while(rqitr != _requestQueue.end())
	{
		RequestQueue::iterator next = rqitr;
		++next; //Pre incremento

		for(unsigned int i = 0; i < reqloadingvec.size(); i++)
		{
			if( (*rqitr)->getFileName() == reqloadingvec.at(i) )
			{
				//Elimino la richiesta in coda
				_requestQueue.erase(rqitr);
			}
		}

		rqitr = next;
	}

	return true;
}

/** Elimina iun loading in base al nome del file */
bool LoadThreadsHandler::cancelLoadingByFileName(std::string filename)
{
	LoaderThreadStack::iterator itr = _loaderThreadsStack.begin();
	while(itr != _loaderThreadsStack.end())
	{
		LoaderThreadStack::iterator next = itr;
		++next;

		if((*itr)->getFileToLoad() == filename)
		{
			//Elimino il Thread di loading
			(*itr)->cancel();
			//FIXME: forse ci va anche  = NULL
			_loaderThreadsStack.erase(itr);

			RequestQueue::iterator rqitr = _requestQueue.begin();
			while( rqitr != _requestQueue.end())
			{
				RequestQueue::iterator rqnext = rqitr;
				++rqnext;
				if((*rqitr)->getFileName() == filename)
				{
					//Elimino la richiesta in coda
					_requestQueue.erase(rqitr);
				}

				rqitr = rqnext;
			}

			return true;
		}

		itr = next;
	}

	return false;
}

/** Clear dei loading e della messageQueue */
bool LoadThreadsHandler::clear()
{
	//Cancello tutti i Thread di Load
	cancelAllLoading();

	//Clear della cache
	clearNodeCache();
	
	_address.clear();

	//Elimino tutte le richieste
	while( _requestQueue.size() > 0)
		_requestQueue.pop_back();

	return true;
}

/** Pulisce la cache */
bool LoadThreadsHandler::clearNodeCache()
{
	while( _loadedNodeStack.size() > 0 )
		_loadedNodeStack.pop_back();

	return true;
}

/** Attiva/Disattiva il Loading Mark */
void LoadThreadsHandler::refreshLoadingMark()
{
	//FIXME: ripristinare

	if( this->isLoading() )//&& this->getLoadingMarkState() != true)
		this->switchLoadingMarkOn();
	else //if(this->getLoadingMarkState() != false)
		this->switchLoadingMarkOff();
}

/** Aggiorna la coda di loading */
void LoadThreadsHandler::refreshLoadingQueue()
{
	if(_requestQueue.size() > 0)
	{
		if( this->getCurrentLoadings() < _maxThreadPerStack)
		{
			osg::ref_ptr<RequestFile> currRequest = this->findFirstRequestAvailable();

			if(currRequest.valid())
			{
				//attacca a Queue e fai partire il loading
				osg::ref_ptr<LoaderThread> instanceNewThread = new LoaderThread( currRequest->getFileName(), currRequest->getLoadingOptions() );

				if(_optimizeModels)
					instanceNewThread->setOptimization(_optType);

				_loaderThreadsStack.push_back(instanceNewThread);
				
				//Cerco in Cache
				LoadedNodeStack::iterator itrln;
				for(itrln = _loadedNodeStack.begin(); itrln != _loadedNodeStack.end(); itrln++)
					if((*itrln)->getName() == currRequest->getFileName())
						instanceNewThread->setNodeLoaded((*itrln).get()); //Trovato lo passo al LoaderThread

				instanceNewThread->start();
		
				currRequest->setLoading();
			}
		}
	}
}

/** Ritorna la prima richiesta da evadere */
LoadThreadsHandler::RequestFile* LoadThreadsHandler::findFirstRequestAvailable()
{
	RequestQueue::iterator itr = _requestQueue.begin();
	for(; itr != _requestQueue.end(); itr++)
		if( ! (*itr)->isLoading() )
			return (*itr).get();

	return NULL;
}

/** Divide una stringa in comando argomento in base al separatore */
void LoadThreadsHandler::splitCommand(std::string command, std::string& largs, std::string& rargs, const char split)
{
	std::string::size_type pos = command.find( " " );

	if( pos != std::string::npos )
	{
		largs = command.substr(0, pos);
		rargs = command.substr(pos + 1, command.size() -1);
	}
	else
	{
		largs = command;
		rargs.clear();
	}
}

/** Scansiona lo SceneGraph e carica scova i gruppi Self Loading "Self_Loading_Group" */
void LoadThreadsHandler::findSelfLoadingGroups()
{
	Visitors::FindNodeVisitor fnvslg("Self_Loading_Group");
	_mainNode->accept(fnvslg);

	if(fnvslg.getNodeFoundSize() == 0)
		return;

	for(unsigned int i = 0; i < fnvslg.getNodeFoundSize(); i++)
	{
		unsigned int npsize = fnvslg.getNodeByIndex(0).size();
		osg::ref_ptr<osg::Group> grp = dynamic_cast<osg::Group*>(fnvslg.getNodeByIndex(0).at(npsize - 1));

		if(grp.valid())
		{
			osg::Node::DescriptionList desclist = grp->getDescriptions();

			if(desclist.size() > 0 )
			{
				std::string newname = "Self_Loading_Group_" + desclist.at(0);
				std::string laddress = this->getServerPrefix() + desclist.at(0);
				grp->setName(newname);

				if(desclist.size() == 1)
					this->requestLoading(laddress, newname); //Carico sotto il mio nodo col nome modificato 
				else if(desclist.size() == 2)
					this->requestLoading(laddress, newname + " " + desclist.at(1)); //Carico sotto il mio nodo col nome modificato + argomento "MULTIPLE o INSTANCE"
				else
					this->requestLoading(laddress, newname + " " + desclist.at(1), new osgDB::ReaderWriter::Options( desclist.at(3) )); //Opzioni di caching
			}
		}
		else
			fnvslg.getNodeByIndex(0).at(npsize - 1)->setName("FAKE_Self_Loading_Group");
	}
}

/**  */
osg::Node* LoadThreadsHandler::handleLoading()
{
	this->findSelfLoadingGroups();

	this->refreshLoadingQueue();
	this->refreshLoadingMark();

	LoaderThreadStack::iterator ltiter;
	for(ltiter = _loaderThreadsStack.begin(); ltiter != _loaderThreadsStack.end(); ltiter++)
	{
		osg::ref_ptr<osg::Node> loadednode = (*ltiter)->getNodeLoaded();
		if( loadednode.valid() )
		{
			_loadedNodeStack.push_back(loadednode);

			RequestQueue::iterator reqcurritr;
			RequestQueue::iterator reqitr = _requestQueue.begin();
			for( ; reqitr != _requestQueue.end(); reqitr++)
			{
				if((*reqitr)->getFileName() == (*ltiter)->getFileToLoad())
				{
					reqcurritr = reqitr;
				}
			}
			
			std::string args = (*reqcurritr)->getArguments();

			if( !args.empty() && _mainNode.valid() )
			{
				bool multipleadd = false;

				//Splitto args tramite " "
				std::string largs, rargs;

				this->splitCommand(args, largs, rargs);
			
				if(rargs == "MULTIPLE") //Attacca in tutti i nodi che fanno match con il nome
					multipleadd = true;

				LoadThreadsHandler::AttachNodeToSceneByName nodecisit(largs, loadednode.get(), multipleadd);
                _mainNode->accept(nodecisit);

                if( nodecisit.isNodeFound() )
				{
					//Elimino il Thread di Loading associato alla richiesta
					_loaderThreadsStack.erase(ltiter);

					//Richiesta esaudita elimino la richiesta pendente
					_requestQueue.erase(reqcurritr);

					//Inserimento nello scenegraph già gestito
					return NULL;
				}
			}
			
			//Elimino il Thread di Loading associato alla richiesta
			_loaderThreadsStack.erase(ltiter);

			//Richiesta esaudita elimino la richiesta pendente
			_requestQueue.erase(reqcurritr);

			//Parent non presente... qualcuno gestirà il nodo
			return loadednode.release();
		}
	}

	//Nessun nodo trovato
	return NULL;
}


/** Rileva errori di caricamento e ritorna la lista dei file non caricati */
bool LoadThreadsHandler::handleLoadingErrors(std::vector<std::string>& filenames)
{
	bool retstr = false;

	LoaderThreadStack::iterator ltiter;
	for(ltiter = _loaderThreadsStack.begin(); ltiter != _loaderThreadsStack.end(); ltiter++)
	{
		if((*ltiter)->isFailed())
		{
			filenames.push_back((*ltiter)->getFileToLoad());
			retstr = true;
		}
	}

	return retstr;
}