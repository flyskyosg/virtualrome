#ifndef __OSG4WEB_SHELLRENDERER__
#define __OSG4WEB_SHELLRENDERER__ 1

#include <CommonShell/Defines.h>
#include <CommonShell/ShellThread.h>


class ShellBase;

class ShellRenderer : public ShellThread
{
public:
	ShellRenderer(ShellBase* rbase);
	~ShellRenderer();

	bool closeRendering();

	void requestExclusion();
	void releaseExclusion();

protected:
	bool doCallBack();

private:
	//Condizione di stop
	bool _needawait;

	ShellBase* _rStubb;
	
}; 

#endif //__OSG4WEB_SHELLRENDERER__