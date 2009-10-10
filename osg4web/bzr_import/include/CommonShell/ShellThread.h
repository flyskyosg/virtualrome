#ifndef __OSG4WEB_SHELLTHREAD__
#define __OSG4WEB_SHELLTHREAD__ 1

#include <CommonShell/Defines.h>

#include <assert.h>

#include "prerror.h"
#include "prlock.h"
#include "prcvar.h"
#include "prthread.h"


class ShellThread
{
public:
	ShellThread(PRThreadPriority tpriority);
	~ShellThread();

	static void threadCallBack(void* instance);

	bool isRunning() { return (_Thread != NULL); }
	bool isShuttingDown() { return _Shutdown; }

	bool isPaused() { return _pPause; }
	bool isBusy() { return _NeedWait; }

	void setPausePlay() { _pPause = !_pPause; }

protected:
	const PRThread* getThread() { return _Thread; }
	bool joinThread();

	void Lock();
	void unLock();

	void waitCondition();
	void notifyCondition();

	void makeSleep(unsigned int sleeptime) { PR_Sleep(sleeptime); }

	void setThreadDelay(unsigned int delay) { _ThreadDelay = delay; }
	unsigned int getThreadDelay() { return _ThreadDelay; }
	
	virtual bool doCallBack() { return false; }

private:
	void destroyLock();
	void destroyCondition();

	PRLock* _Lock;
	PRCondVar* _CondVar;
	PRThread* _Thread;

	unsigned int _ThreadDelay;

	bool _NeedWait;

	bool _pPause;

	bool _Shutdown;
};



#endif //__OSG4WEB_SHELLTHREAD__