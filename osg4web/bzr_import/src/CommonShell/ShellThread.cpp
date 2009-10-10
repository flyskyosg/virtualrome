

#include <CommonShell/ShellThread.h>



ShellThread::ShellThread() : _Lock(NULL),
	_CondVar(NULL),
	_Thread(NULL),
	_NeedWait(false),
	_Shutdown(false)
{
	//Lock Creation
	_Lock = PR_NewLock();
	//Check di esistenza del lock
	assert(_Lock);

	//Contion Variable creation. It will be asociated to the previous lock
	_CondVar = PR_NewCondVar(_Lock);
	//Check di esistenza del lock
	assert(_CondVar);

	//Thread creation
	_Thread = PR_CreateThread(PR_USER_THREAD, ShellThread::threadCallBack, this, PR_PRIORITY_NORMAL, PR_GLOBAL_THREAD, PR_JOINABLE_THREAD, 0);
	//Check di esistenza del thread
	assert(_Thread);
}

ShellThread::~ShellThread()
{
	this->joinThread();
	//Clearing lock and condition
	this->destroyCondition();
	this->destroyLock(); 
}

void ShellThread::destroyLock()
{
	if(_Lock)
	{
		PR_DestroyLock(_Lock);
		_Lock = NULL;
	}
}

void ShellThread::destroyCondition()
{
	if(_CondVar)
	{
		PR_DestroyCondVar(_CondVar);
		_CondVar = NULL;
	}
}

bool ShellThread::joinThread()
{
	if(_Thread)
	{
		_Shutdown = true;
		
		this->notifyCondition();

		PR_JoinThread(_Thread);
		PR_Sleep( JOIN_THREAD_DELAY );

		_Thread = NULL;

		return true;
	}

	return false;
}

void ShellThread::waitCondition()
{
	this->Lock();
	while(_NeedWait)
		PR_WaitCondVar(_CondVar, PR_INTERVAL_MIN);

	_NeedWait = true;
}

void ShellThread::notifyCondition()
{
	_NeedWait = false;
	PR_NotifyCondVar(_CondVar);
	this->unLock();
}

void ShellThread::Lock() 
{
	PR_Lock(_Lock);
}

void ShellThread::unLock() 
{ 
	PR_Unlock(_Lock); 
}

void ShellThread::threadCallBack(void* instance)
{
	ShellThread* shellinstance = (ShellThread*) instance;

	if(instance)
		shellinstance->doCallBack();
	else
	{
		//TODO: ....
	}
}