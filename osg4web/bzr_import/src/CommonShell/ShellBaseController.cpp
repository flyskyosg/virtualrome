
#include <CommonShell/ShellBaseController.h>

#include <CommonShell/ShellBase.h>


/*********************************************************************************
 *
 * ShellBaseController
 *
 *********************************************************************************/

ShellBaseController::ShellBaseController() : _rbase(NULL) 
{

}

ShellBaseController::ShellBaseController(ShellBase* rbase) : _rbase(rbase) 
{

}

ShellBaseController::~ShellBaseController()
{
	if(_rbase)
		_rbase = NULL; //Clear della copia
}

int ShellBaseController::progressDLStatus(double downtot, double downnow, double ultotal, double ulnow)
{
	return _rbase->doProgressDLStatus(downtot, downnow, ultotal, ulnow);
}

int ShellBaseController::progressUnpackStubb(unsigned int cicleno, int filenumber)
{
	return _rbase->doProgressUnpackStatus(cicleno, filenumber);
}

bool ShellBaseController::startDownloadedCore(bool timing)
{
	return _rbase->startDownloadedCore(timing);
}

bool ShellBaseController::checkAdvCorePresence()
{
	return _rbase->checkAdvCorePresence();
}

void ShellBaseController::startDownloading(bool timing)
{
	_rbase->startDownloading(timing);
}

void ShellBaseController::checkSecurityString(bool timing)
{
	_rbase->checkSecurityString(timing);
}

void ShellBaseController::startUnPackSession(bool timing)
{
	_rbase->startUnPackSession(timing);
}

void ShellBaseController::downloadFinished(bool timing)
{
	_rbase->downloadFinished(timing);
}

void ShellBaseController::downloadError(int error)
{
	_rbase->downloadError(error);
}

void ShellBaseController::securityStringError()
{
	_rbase->securityStringError();
}

void ShellBaseController::unpackError(int error)
{
	_rbase->unpackError(error);
}