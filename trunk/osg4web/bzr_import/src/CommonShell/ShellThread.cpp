

#include <CommonShell/ShellThread.h>


ShellThread::ShellThread() : mLock( PR_NewLock() ),
	mThread(NULL),
	mShutdown(false)
{
	//Check di esistenza del lock
	assert(mLock);

}

ShellThread::ShellThread(void (PR_CALLBACK *callfunction)(void *arg)) : mLock( PR_NewLock() ),
	mThread(NULL),
	mShutdown(false)
{
	//Check di esistenza del lock
	assert(mLock);

	this->createThread(callfunction);

	//Check di esistenza del thread
	assert(mThread);
}

ShellThread::~ShellThread()
{
	if (mThread)
		this->joinThread();

	if (mLock)
	{
		PR_DestroyLock(mLock);
		mLock = NULL;
	}
}

bool ShellThread::createThread(void (PR_CALLBACK *callfunction)(void *arg))
{
	if(!mThread)
	{
		mShutdown = false;
		mThread = PR_CreateThread(PR_USER_THREAD, callfunction, this, PR_PRIORITY_NORMAL, PR_GLOBAL_THREAD, PR_JOINABLE_THREAD, 0);
		//Check di esistenza del thread
		if(!mThread)
			return false;

		return true;
	}

	return false;
}

bool ShellThread::joinThread()
{
	if(mThread)
	{
		mShutdown = true;

		PR_JoinThread(mThread);
		PR_Sleep(JOIN_THREAD_DELAY);

		mShutdown = false;
		mThread = NULL;

		return true;
	}

	return false;
}