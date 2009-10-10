
#include <Utilities/SHA1.h>
#include <Utilities/FileUtils.h>

#include <CommonShell/ShellDownloader.h>

#include <CommonShell/ShellBase.h>



/*********************************************************************************
 *
 * ShellDownloader
 *
 *********************************************************************************/

ShellDownloader::ShellDownloader(ShellBase* rbase) : _proxyhost(""),
	_proxyport("8080"),
	_curlctr(NULL),
	_unpackerctr(NULL)
{
	//Check ShellBase existance
	assert(rbase);

	_dwnlcontrol = new ShellBaseController(rbase);
}

ShellDownloader::~ShellDownloader()
{
	clearDownloadRequestQueue();

	if(_curlctr)
	{
		_curlctr->stopDownloading();
		delete _curlctr;
		_curlctr = NULL;
	}

	if(_unpackerctr)
	{
		_unpackerctr->stopUnpacking();
		delete _unpackerctr;
		_unpackerctr = NULL;
	}

	if(_dwnlcontrol)
	{
		delete _dwnlcontrol;
		_dwnlcontrol = NULL;
	}
}

bool ShellDownloader::closeDownloader() //FIXME: controllare 
{
	if(_unpackerctr)
		_unpackerctr->stopUnpacking();

	if(_curlctr)
		_curlctr->stopDownloading();

	if(this->joinThread())
		return true;

	return false;
}

void ShellDownloader::addDownloadRequest(ShellDownloader::RequestDownload reqdl)
{
	_requestqueue.push(reqdl); 
}

void ShellDownloader::clearDownloadRequestQueue()
{
	if(_unpackerctr)
		_unpackerctr->stopUnpacking();

	if(_curlctr)
		_curlctr->stopDownloading();

	while( _requestqueue.size() != 0 )
		_requestqueue.pop();
}

void ShellDownloader::setProxy(std::string hostname, std::string port)
{
	_proxyhost = hostname;
	_proxyport = port;
}

bool ShellDownloader::doCallBack()
{
	this->Lock();

	for (;;)
	{
		while(_requestqueue.size() == 0 && !this->isShuttingDown()) //freezing until new requests are available
			this->makeSleep(DOWNLOADER_CONDITION_DELAY);

		if (this->isShuttingDown()) //FIXME: controllare 
			break;
		
		RequestDownload req = _requestqueue.front();
		_requestqueue.pop();
	
		switch( req.getFileType() )
		{
		case RequestDownload::CORE_FILE:
				this->handleCoreDownload(req);
			break;
		case RequestDownload::DEPCORE_FILE:
				 this->handleDepCoreDownload(req);
			break;
		default:
				this->handleSimpleFile(req);
			break;
		}
	}

	this->unLock();

	return true;
}

void ShellDownloader::handleCoreDownload(RequestDownload curreq)
{
	if(_dwnlcontrol->checkAdvCorePresence())
	{
		if(_dwnlcontrol->startDownloadedCore())
			return;
	}

	if(this->performDownload(curreq))
		_dwnlcontrol->startDownloadedCore(); //Starting Downloaded Core
}

void ShellDownloader::handleDepCoreDownload(RequestDownload curreq)
{
	if(_dwnlcontrol->checkAdvCorePresence())
		return; //Discarding downloading because adv core is found

	this->performDownload(curreq);
}

void ShellDownloader::handleSimpleFile(RequestDownload curreq)
{
	this->performDownload(curreq, false);
}

bool ShellDownloader::performDownload(RequestDownload curreq, bool timing)
{
	_dwnlcontrol->startDownloading(timing);
	int downloaderr = this->downloadRequestedFile(curreq);
	if(downloaderr <= 0)
	{
		_dwnlcontrol->downloadError(downloaderr);
		return false;
	}

	if(!curreq.getSecurityString().empty())
	{
		_dwnlcontrol->checkSecurityString(timing);
		/*if(!this->checkSecurityString(curreq))
		{
			_dwnlcontrol->securityStringError();
			return false;
		}*/
	}

	if(curreq.getUnpackFlag())
	{
		_dwnlcontrol->startUnPackSession(timing);
		int error = this->unpackRequestedFile(curreq);
		if(error != 1)
		{
			_dwnlcontrol->unpackError( error );
			return false;
		}
	}

	_dwnlcontrol->downloadFinished(timing);

	return true;
}

int ShellDownloader::downloadRequestedFile(RequestDownload curreq)
{
	int ret = 0;

	_curlctr = new ShellCURL(curreq.getDownloadURL(), curreq.getDownloadTempDir());
	
	if(!_proxyhost.empty())
		_curlctr->setProxy(_proxyhost, _proxyport);

	_curlctr->setProgressDLStatus((ShellCURL::ProgressDownloadStubb*) _dwnlcontrol);

	ret = _curlctr->performDownload();

	delete _curlctr;
	_curlctr = NULL;

	return ret;
}

int ShellDownloader::checkSecurityString(RequestDownload curreq)
{
	int ret = -1; //Error State
	bool opensucc = false;

	std::string fname = Utilities::FileUtils::convertFileNameToNativeStyle( curreq.getDownloadTempDir() + "/" + Utilities::FileUtils::getSimpleFileName(curreq.getDownloadURL()) );
	Utilities::CryptoAlg::SHA1 sha1filecont;

	sha1filecont.Reset();
	opensucc = sha1filecont.HashFile((char*) fname.c_str());
	sha1filecont.Final();

	std::string report = sha1filecont.ReportHash(Utilities::CryptoAlg::SHA1::REPORT_HEX);

	if(opensucc)
	{
		if(report == curreq.getSecurityString())
			ret = 1; //Check OK
		else
			ret = 0; //Check Failed
	}
	
	return ret;
}

int ShellDownloader::unpackRequestedFile(RequestDownload curreq)
{
	int error = 0;

	std::string fname = Utilities::FileUtils::convertFileNameToNativeStyle( curreq.getDownloadTempDir() + "/" + Utilities::FileUtils::getSimpleFileName(curreq.getDownloadURL()) );

	_unpackerctr = new ShellUnpacker( fname, curreq.getUnpackDirectory() );
	_unpackerctr->setProgressUnpackStubb((ShellUnpacker::ProgressUnpackStubb*) _dwnlcontrol);

	//FIXME: gestire in ciclo per fare il controllo di stop
	error = _unpackerctr->unpackAll();

	delete _unpackerctr;
	_unpackerctr = NULL;

	return error;
}
