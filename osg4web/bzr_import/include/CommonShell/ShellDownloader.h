#ifndef __OSG4WEB_SHELLDOWNLOADER__
#define __OSG4WEB_SHELLDOWNLOADER__ 1

#include <CommonShell/Defines.h>
#include <CommonShell/ShellThread.h>
#include <CommonShell/ShellUnpacker.h>
#include <CommonShell/ShellCURL.h>
#include <CommonShell/ShellUnpacker.h>
#include <CommonShell/ShellBaseController.h>

class ShellBase;

#include <queue>


#define		DOWNLOADER_CONDITION_DELAY			1000

class ShellDownloader : public ShellThread
{
public:

	class RequestDownload
	{
	public:
		enum FileType
		{
			SIMPLE_FILE = 0,
			CORE_FILE,
			DEPCORE_FILE
		};

		RequestDownload() : _unpackflag(false), _type(SIMPLE_FILE), _dlurl(""), _dltempdir(""),	_unpackdir(""), _securitystr("") { }
		
		FileType getFileType() { return _type; }	
		bool getUnpackFlag() { return _unpackflag; }	
		std::string getUnpackDirectory() { return _unpackdir; }
		std::string getSecurityString() { return _securitystr; }
		std::string getDownloadTempDir() { return _dltempdir; }
		std::string getDownloadURL() { return _dlurl; }

		void setFileType(FileType type) { _type = type; }	
		void setUnpackFlag(bool flag = true) { _unpackflag = flag; }	
		void setSecurityString( std::string secstr ) { _securitystr = secstr; }
		void setUnpackDirectory(std::string unpackdir) { _unpackdir = unpackdir; _unpackflag = true; }
		void setDownloadTempDir(std::string dltempdir) { _dltempdir = dltempdir; }
		void setDownloadURL(std::string dlurl, FileType type = SIMPLE_FILE) { _dlurl = dlurl; _type = type; }

		void clear() { _dlurl.clear(); _dltempdir.clear(); _unpackdir.clear(); _securitystr.clear(); _unpackflag = false;  _type = SIMPLE_FILE; }

	private:
		std::string _dlurl, _dltempdir, _unpackdir, _securitystr;
		FileType _type;
		bool _unpackflag;
	};

	ShellDownloader(ShellBase* rbase);
	~ShellDownloader();
	
	void addDownloadRequest(ShellDownloader::RequestDownload reqdl);
	void clearDownloadRequestQueue();

	void setProxy(std::string hostname, std::string port = "8080");

	bool closeDownloader();

protected:

	void handleCoreDownload(RequestDownload curreq);
	void handleDepCoreDownload(RequestDownload curreq);
	void handleSimpleFile(RequestDownload curreq);

	bool performDownload(RequestDownload curreq, bool timing = true);

	int downloadRequestedFile(RequestDownload curreq);
	int checkSecurityString(RequestDownload curreq);
	int unpackRequestedFile(RequestDownload curreq);

	bool doCallBack();

private:
	std::string _proxyhost, _proxyport;

	std::queue<RequestDownload> _requestqueue;

	ShellCURL* _curlctr;
	ShellUnpacker* _unpackerctr;

	ShellBaseController* _dwnlcontrol;
};

#endif //__OSG4WEB_SHELLDOWNLOADER__