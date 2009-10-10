#ifndef __OSG4WEB_SHELLTHREAD__
#define __OSG4WEB_SHELLTHREAD__ 1

#include <CommonShell/Defines.h>

#include <assert.h>

#include "prlock.h"
#include "prthread.h"


class ShellThread
{
public:
	ShellThread();
	ShellThread(void (PR_CALLBACK *callfunction)(void *arg));

	~ShellThread();

	bool createThread(void (PR_CALLBACK *callfunction)(void *arg));
	bool joinThread();

	bool isShuttingDown() { return mShutdown; }
	bool isRunning() { return (mThread != NULL); }

	void Lock() { PR_Lock(mLock); }
	void unLock() { PR_Unlock(mLock); }

	const PRThread* getThread() { return mThread; }

private:
	PRLock* mLock;
	PRThread* mThread;

	bool mShutdown;
};

#endif //__OSG4WEB_SHELLTHREAD__