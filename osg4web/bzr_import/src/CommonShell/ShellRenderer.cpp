

#include <CommonShell/ShellRenderer.h>
#include <CommonShell/ShellBase.h>


ShellRenderer::ShellRenderer(ShellBase* rbase) : _rStubb(rbase)
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
		if( this->isBusy() )
			this->waitCondition();
			
		if( !_rStubb->doRenderingBridge() )
		{
			this->notifyCondition();
			return false;
		}
				
		if( this->isShuttingDown() )
			break;
			
		this->unLock();
		this->makeSleep( RENDER_DELAY );
		this->Lock();
	}

	_rStubb->setDoneBridge();
	
	this->notifyCondition();
	this->unLock();

	return true;
}