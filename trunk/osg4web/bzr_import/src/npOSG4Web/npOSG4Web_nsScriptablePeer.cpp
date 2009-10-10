// ==============================
// ! Scriptability related code !
// ==============================

#include <npOSG4Web/npOSG4Web_plugin.h>
#include "nsMemory.h"

static NS_DEFINE_IID(kIScriptableIID, NS_ISCRIPTABLEPLUGINOSG_IID);
static NS_DEFINE_IID(kIClassInfoIID, NS_ICLASSINFO_IID);
static NS_DEFINE_IID(kISupportsIID, NS_ISUPPORTS_IID);

nsScriptablePeer::nsScriptablePeer(nsPluginInstance* aPlugin)
{
	mPlugin = aPlugin;
	mRefCnt = 0;
}

nsScriptablePeer::~nsScriptablePeer()
{

}

// AddRef, Release and QueryInterface are common methods and must 
// be implemented for any interface
NS_IMETHODIMP_(nsrefcnt) nsScriptablePeer::AddRef() 
{ 
	++mRefCnt; 
	return mRefCnt; 
} 

NS_IMETHODIMP_(nsrefcnt) nsScriptablePeer::Release() 
{ 
	--mRefCnt; 
	if (mRefCnt == 0) 
	{ 
		delete this;
		return 0; 
	} 
	return mRefCnt; 
} 

// here nsScriptablePeer should return three interfaces it can be asked for by their iid's
// static casts are necessary to ensure that correct pointer is returned
NS_IMETHODIMP nsScriptablePeer::QueryInterface(const nsIID& aIID, void** aInstancePtr) 
{ 
	if(!aInstancePtr) 
		return NS_ERROR_NULL_POINTER; 

	if(aIID.Equals(kIScriptableIID)) 
	{
		*aInstancePtr = static_cast<nsIScriptablePluginOSG*>(this); 
		AddRef();
		return NS_OK;
	}

	if(aIID.Equals(kIClassInfoIID)) 
	{
		*aInstancePtr = static_cast<nsIClassInfo*>(this); 
		AddRef();
		return NS_OK;
	}

	if(aIID.Equals(kISupportsIID)) 
	{
		*aInstancePtr = static_cast<nsISupports*>(static_cast<nsIScriptablePluginOSG*>(this)); 
		AddRef();
		return NS_OK;
	}

	return NS_NOINTERFACE; 
}

void nsScriptablePeer::SetInstance(nsPluginInstance* plugin)
{
	mPlugin = plugin;
}

//
// the following methods will be callable from JavaScript
//

NS_IMETHODIMP nsScriptablePeer::ExecCoreCommand(const char *line, char **data)
{
	std::string result;

	if (!line)
		return NS_ERROR_NULL_POINTER;

	if (mPlugin)
	{
		result = mPlugin->execCoreCommand(std::string( line ));
		
		if(!data)
			return NS_ERROR_NULL_POINTER;

		*data = nsnull;
		*data = (char*) NS_Alloc((result.size() + 1) * sizeof(char));

		unsigned int i;
		for(i = 0; i < result.size(); i++)
		{
			(*data)[i] = result.at(i);
		}

		(*data)[i] = '\0';
    
		return *data ? NS_OK : NS_ERROR_OUT_OF_MEMORY;
	}
	else
		return NS_ERROR_NOT_INITIALIZED;
}

NS_IMETHODIMP nsScriptablePeer::ExecShellCommand(const char *line, char **data)
{
	std::string result;

	if (!line)
		return NS_ERROR_NULL_POINTER;

	if (mPlugin)
	{
		result = mPlugin->execShellCommand(std::string( line ));
		
		if(!data)
			return NS_ERROR_NULL_POINTER;

		*data = nsnull;
		*data = (char*) NS_Alloc((result.size() + 1) * sizeof(char));

		unsigned int i;
		for(i = 0; i < result.size(); i++)
		{
			(*data)[i] = result.at(i);
		}

		(*data)[i] = '\0';
    
		return *data ? NS_OK : NS_ERROR_OUT_OF_MEMORY;
	}
	else
		return NS_ERROR_NOT_INITIALIZED;
}

NS_IMETHODIMP nsScriptablePeer::GetEvent(const char *line)
{
	return NS_ERROR_NOT_IMPLEMENTED; //Non deve essere implementata... serve per allineare l'interfaccia con l'activex
}