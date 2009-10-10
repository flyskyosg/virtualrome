
#include <curl/curl.h>
#include <CommonShell/ShellCURL.h>

#include <Utilities/FileUtils.h>

#include <fstream>
#include <cstdio>


ShellCURL::ShellCURL() : _curl(NULL),
	_dlurl(""), 
	_dltempdir(""),
	_stopdownloading(false),
	_downloaded(false)
{
	this->initCURL();
}

ShellCURL::ShellCURL(std::string downloadurl, std::string tempdir) : _curl(NULL),
	_dlurl(downloadurl) ,
	_dltempdir(tempdir),
	_stopdownloading(false),
	_downloaded(false)
{
	this->initCURL();
}

ShellCURL::~ShellCURL()
{
	if(_curl)
	{
		curl_easy_cleanup(_curl);
		_curl = NULL;
	}

	_headerbuffer.clear();
	_buffer.clear();
}

void ShellCURL::initCURL()
{
	_curl = curl_easy_init();

#if defined(_DEBUG)
	curl_easy_setopt(_curl, CURLOPT_VERBOSE, 1);
#endif
	curl_easy_setopt(_curl, CURLOPT_USERAGENT, "libcurl-agent/1.0");      
	curl_easy_setopt(_curl, CURLOPT_WRITEFUNCTION, &ShellCURL::writeStreamMemoryCallback);
	curl_easy_setopt(_curl, CURLOPT_READFUNCTION, &ShellCURL::readStreamMemoryCallback);
}

bool ShellCURL::setProxy(std::string url, std::string port)
{
	std::string proxy;

	if(!url.empty())
	{
		proxy = url + ":" + port;
		curl_easy_setopt(_curl, CURLOPT_PROXY, proxy.c_str()); 

		return true;
	}

	return false;
}

size_t ShellCURL::writeStreamMemoryCallback(void *ptr, size_t size, size_t nmemb, void *data)
{
	CURLBridge* bridge = (CURLBridge*) data;

	if(!bridge)
		return 0L;

	return bridge->getCURLBridge()->writeDLStream(ptr, size, nmemb, bridge->getDataBridge());
}

size_t ShellCURL::readStreamMemoryCallback(void *ptr, size_t size, size_t nmemb, void *data)
{
	CURLBridge* bridge = (CURLBridge*) data;

	if(!bridge)
		return 0L;

	return bridge->getCURLBridge()->readDLStream(ptr, size, nmemb, bridge->getDataBridge());
}

size_t ShellCURL::writeDLStream(void *ptr, size_t size, size_t nmemb, void *data)
{
	size_t realsize = size * nmemb;
	std::ostream* buffer = (std::ostream*)data;

	if(_stopdownloading)
		return 0L; //Stoppa il download

	buffer->write((const char*)ptr, realsize);
	return realsize;
}

size_t ShellCURL::readDLStream(void *ptr, size_t size, size_t nmemb, void *data)
{
	size_t realsize = size * nmemb;

	if(_stopdownloading)
		return 0L; //Stoppa l'upload

	//TODO:
	return realsize;
}

int ShellCURL::callProgressDLStatus(void* classptr, double downtot, double downnow, double ultotal, double ulnow)
{
	ProgressDownloadStubb *instance = (ProgressDownloadStubb *) classptr;

	if(instance)
		return instance->progressDLStatus(downtot, downnow, ultotal, ulnow);
	else
		return -1;
}

bool ShellCURL::setProgressDLStatus(ProgressDownloadStubb* pdlstubb)
{
	if(!pdlstubb)
		return false;

	curl_easy_setopt(_curl, CURLOPT_NOPROGRESS, FALSE);
	curl_easy_setopt(_curl, CURLOPT_PROGRESSFUNCTION, &ShellCURL::callProgressDLStatus);
	curl_easy_setopt(_curl, CURLOPT_PROGRESSDATA, pdlstubb);

	return true;
}

int ShellCURL::performDownload()
{
	CURLcode res = CURLE_OK;

	if(_dlurl.empty())
		return -1;//URL not specified

	if(this->checkOnCache())
		return 2;

	CURLBridge headerbridge(this, (void *)&_headerbuffer);
	CURLBridge dldatabridge(this, (void *)&_buffer);

	curl_easy_setopt(_curl, CURLOPT_URL, _dlurl.c_str());
	curl_easy_setopt(_curl, CURLOPT_WRITEDATA,  (void *)&dldatabridge);
	curl_easy_setopt(_curl, CURLOPT_WRITEHEADER,  (void *)&headerbridge);

	res = curl_easy_perform(_curl);
	
	curl_easy_setopt(_curl, CURLOPT_WRITEDATA,  (void *)0);
	curl_easy_setopt(_curl, CURLOPT_WRITEHEADER,  (void *)0);
	
	if(res != CURLE_OK)
		return 0;//Download error

	_downloaded = true;

	if(!checkingHeader())
		return -2; //Server error

	if(!writeFileOnDisk())
		return -3;	//Error writing file to disk

	return 1; //Download OK
}

bool ShellCURL::checkingHeader()
{
	bool ret = true;

	if(_downloaded)
	{
		std::string headline;
		if(std::getline( _headerbuffer, headline ))
		{
			std::string::size_type pos = headline.find("200");
			if(pos == std::string::npos) //Download failed.
				ret = false;
		}
	}
	else
		ret = false;

	return ret;
}


bool ShellCURL::writeFileOnDisk()
{
	bool ret = true;

	if(_downloaded)
	{
		std::string compfname = this->getCacheFileName();
		std::ofstream outfile(compfname.c_str(), std::ofstream::binary);
		outfile << _buffer.str();
		outfile.close();
	}
	else
		ret = false;

	return ret;
}

//TODO: Fare gestione della cache con senso 
std::string ShellCURL::getCacheFileName()
{
	std::string cachefname;
	
	cachefname = Utilities::FileUtils::convertFileNameToNativeStyle( _dltempdir + "/" + Utilities::FileUtils::getSimpleFileName(_dlurl) );

	return cachefname;
}

bool ShellCURL::checkOnCache()
{
	std::string fname = this->getCacheFileName();
	if( Utilities::FileUtils::fileExists(fname))
		return true;
	
	return false;
}