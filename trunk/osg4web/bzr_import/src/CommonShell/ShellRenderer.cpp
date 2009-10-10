

#include <CommonShell/ShellRenderer.h>
#include <CommonShell/ShellBase.h>


ShellRenderer::ShellRenderer(ShellBase* rbase) :  ShellThread(PR_PRIORITY_HIGH),
	_rStubb(rbase)
{
	//Check ShellBase existance
	assert(_rStubb);
}

ShellRenderer::~ShellRenderer()
{
	if(_rStubb)
		_rStubb = NULL; //Clear della copia
}

bool ShellRenderer::closeRendering()
{
	if(this->joinThread())
		return true;

	return false;
}

void ShellRenderer::requestExclusion()
{
	this->waitCondition();
}

void ShellRenderer::releaseExclusion()
{
	this->notifyCondition();
}

bool ShellRenderer::doCallBack()
{
	this->Lock();

	for (;;)
	{
		if( this->isBusy() && !this->isShuttingDown())
			this->waitCondition();
			
		if( !this->isShuttingDown() && !this->isPaused() )
		{
			if(!_rStubb->doRenderingBridge())
			{
				this->notifyCondition();
				break;
			}
		}
				
		if( this->isShuttingDown() )
			break;
			
		this->unLock();

		if(!this->isShuttingDown())
			this->makeSleep( this->getThreadDelay() );

		this->Lock();
	}

	_rStubb->setDoneBridge();
	
	this->notifyCondition();
	this->unLock();

	return true;
}