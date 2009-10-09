#ifndef __OSG4WEB_SHELLCURL__
#define __OSG4WEB_SHELLCURL__ 1

#include <CommonShell/Defines.h>

//#include <curl/curl.h>
#include <string>
#include <sstream>

class ShellCURL
{
public:

	class CURLBridge
	{
	public:
		CURLBridge(ShellCURL* scurlptr, void* dataptr) : _scurlptr(scurlptr), _dataptr(dataptr) { }

		ShellCURL* getCURLBridge() { return _scurlptr; }
		void* getDataBridge() { return _dataptr; }
	
	private:
		ShellCURL* _scurlptr;
		void* _dataptr;
	};

	class ProgressDownloadStubb
	{
	public:
		virtual int progressDLStatus(double downtot, double downnow, double ultotal, double ulnow) { return -1; }
	};

	ShellCURL();
	ShellCURL(std::string downloadurl, std::string tempdir);
	~ShellCURL();

	bool setProxy(std::string url, std::string port = "8080");
	bool setProgressDLStatus(ProgressDownloadStubb* dlcallback);

	void setDownloadURL(std::string dlurl) { _dlurl = dlurl; }
	void setTempDirectory(std::string tempdir) { _dltempdir = tempdir; }

	static size_t writeStreamMemoryCallback(void*, size_t, size_t, void*);
	static size_t readStreamMemoryCallback(void*, size_t, size_t, void*);

	static int callProgressDLStatus(void*, double , double, double, double);

	int performDownload();

	void stopDownloading() { _stopdownloading = true; }

protected:
	virtual size_t writeDLStream( void*, size_t, size_t, void* );
	virtual size_t readDLStream( void*, size_t, size_t, void* );
	
	void initCURL();
	bool checkingHeader();
	bool writeFileOnDisk();

	std::string getCacheFileName();
	bool checkOnCache();

private:
	std::string _dlurl, _dltempdir;

	std::stringstream _buffer;
	std::stringstream _headerbuffer;

	bool _stopdownloading;
	bool _downloaded;

	void* _curl;
};



#endif //__OSG4WEB_SHELLCURL__