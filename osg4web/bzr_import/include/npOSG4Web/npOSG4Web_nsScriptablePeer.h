#ifndef __NPOSG4WEB_SCRIPTABLEPEER__
#define __NPOSG4WEB_SCRIPTABLEPEER__ 1

#include <npOSG4Web/npOSG4Web_nsIScriptablePlugin.h>
#include "nsIProgrammingLanguage.h"
#include "nsIClassInfo.h"

class nsPluginInstance;

// We must implement nsIClassInfo because it signals the
// Mozilla Security Manager to allow calls from JavaScript.

class nsClassInfoMixin : public nsIClassInfo
{
	// These flags are used by the DOM and security systems to signal that 
	// JavaScript callers are allowed to call this object's scritable methods.
	NS_IMETHOD GetFlags(PRUint32 *aFlags)
	{
		*aFlags = nsIClassInfo::PLUGIN_OBJECT | nsIClassInfo::DOM_OBJECT;
		return NS_OK;
	}
	NS_IMETHOD GetImplementationLanguage(PRUint32 *aImplementationLanguage)
    {
		*aImplementationLanguage = nsIProgrammingLanguage::CPLUSPLUS;
		return NS_OK;
	}
	// The rest of the methods can safely return error codes...
	NS_IMETHOD GetInterfaces(PRUint32 *count, nsIID * **array)
    {
		return NS_ERROR_NOT_IMPLEMENTED;
	}
	NS_IMETHOD GetHelperForLanguage(PRUint32 language, nsISupports **_retval)
	{
		return NS_ERROR_NOT_IMPLEMENTED;
	}
	NS_IMETHOD GetContractID(char * *aContractID)
    {
		return NS_ERROR_NOT_IMPLEMENTED;
	}
	NS_IMETHOD GetClassDescription(char * *aClassDescription)
    {
		return NS_ERROR_NOT_IMPLEMENTED;
	}
	NS_IMETHOD GetClassID(nsCID * *aClassID)
    {
		return NS_ERROR_NOT_IMPLEMENTED;
	}
	NS_IMETHOD GetClassIDNoAlloc(nsCID *aClassIDNoAlloc)
    {
		return NS_ERROR_NOT_IMPLEMENTED;
	}
};

class nsScriptablePeer : public nsIScriptablePluginOSG,
                         public nsClassInfoMixin
{
public:
	nsScriptablePeer(nsPluginInstance* plugin);
	~nsScriptablePeer();

public:
	// methods from nsISupports
	NS_IMETHOD QueryInterface(const nsIID& aIID, void** aInstancePtr); 
	NS_IMETHOD_(nsrefcnt) AddRef(); 
	NS_IMETHOD_(nsrefcnt) Release(); 
 
protected: 
	nsrefcnt mRefCnt;  

public:
	// native methods callable from JavaScript
	NS_DECL_NSISCRIPTABLEPLUGINOSG

	void SetInstance(nsPluginInstance* plugin);

protected:
	nsPluginInstance* mPlugin;
};

#endif //__NPOSG4WEB_SCRIPTABLEPEER__



