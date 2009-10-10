#ifndef __OSG4WEB_SHELLBASECONTROLLER__
#define __OSG4WEB_SHELLBASECONTROLLER__ 1

#include <CommonShell/ShellThread.h>
#include <CommonShell/ShellUnpacker.h>
#include <CommonShell/ShellCURL.h>

class ShellBase;

/*********************************************************************************
 *
 * ShellBaseController: class definition
 *
 *********************************************************************************/

class ShellBaseController : public ShellCURL::ProgressDownloadStubb, public ShellUnpacker::ProgressUnpackStubb
{
public:
	ShellBaseController();
	ShellBaseController(ShellBase* rbase);
	~ShellBaseController();
		
	void setShellBase(ShellBase* rbase) { _rbase = rbase; }

	bool startDownloadedCore(bool timing = true);
	bool checkAdvCorePresence();

	void startDownloading(bool timing = true);
	void checkSecurityString(bool timing = true);
	void startUnPackSession(bool timing = true);
	void downloadFinished(bool timing = true);
	void downloadCoreFileFinished(std::string url, bool timing = true);
	void downloadError(int error);
	void securityStringError();
	void unpackError(int error);
	
	//Progress Bar CallBacks
	int progressDLStatus(double downtot, double downnow, double ultotal, double ulnow);
	int progressUnpackStubb(unsigned int cicleno, int filenumber);

private:
	ShellBase * _rbase;		
};


#endif //__OSG4WEB_SHELLBASECONTROLLER__