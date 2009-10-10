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
	ShellThread();
	~ShellThread();

	static void threadCallBack(void* instance);

protected:
	const PRThread* getThread() { return _Thread; }
	bool joinThread();

	void Lock();
	void unLock();

	void waitCondition();
	void notifyCondition();

	bool isRunning() { return (_Thread != NULL); }
	bool isShuttingDown() { return _Shutdown; }

	bool isBusy() { return _NeedWait; }

	void makeSleep(unsigned int sleeptime) { PR_Sleep(sleeptime); }
	
	virtual bool doCallBack() { return false; }

private:
	void destroyLock();
	void destroyCondition();

	PRLock* _Lock;
	PRCondVar* _CondVar;
	PRThread* _Thread;

	bool _NeedWait;

	bool _Shutdown;
};



#endif //__OSG4WEB_SHELLTHREAD__