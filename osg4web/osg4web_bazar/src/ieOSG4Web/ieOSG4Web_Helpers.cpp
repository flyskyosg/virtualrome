// Helpers.cpp : Implementation of of object safety category helper functions
// Copied from ActiveX SDK

#include <ieOSG4Web/ieOSG4Web_stdafx.h>
#include <ieOSG4Web/ieOSG4Web_Helpers.h>

/////////////////////////////////////////////////////////////////////////////
// CreateComponentCategory - Ensures component category exists in registry
// (Copied from ActiveX SDK docs)

HRESULT CreateComponentCategory(CATID catid, WCHAR* catDescription)
{
	/* NUOVO CODICE... da correggere 


	#include "comcat.h"
#include "strsafe.h"
#include "objsafe.h"


	
	ICatRegister *pcr = NULL ;
    HRESULT hr = S_OK ;
 
    hr = CoCreateInstance(CLSID_StdComponentCategoriesMgr, 
            NULL, CLSCTX_INPROC_SERVER, IID_ICatRegister, (void**)&pcr);
    if (FAILED(hr))
        return hr;
 
    // Make sure the HKCR\Component Categories\{..catid...}
    // key is registered.
    CATEGORYINFO catinfo;
    catinfo.catid = catid;
    catinfo.lcid = 0x0409 ; // english
    size_t len;
    // Make sure the provided description is not too long.
    // Only copy the first 127 characters if it is.
    // The second parameter of StringCchLength is the maximum
    // number of characters that may be read into catDescription.
    // There must be room for a NULL-terminator. The third parameter
    // contains the number of characters excluding the NULL-terminator.
    hr = StringCchLength(catDescription, STRSAFE_MAX_CCH, &len);
    if (SUCCEEDED(hr))
        {
        if (len>127)
          {
            len = 127;
          }
        }   
    else
        {
          // TODO: Write an error handler;
        }
    // The second parameter of StringCchCopy is 128 because you need 
    // room for a NULL-terminator.
    hr = StringCchCopy(catinfo.szDescription, len + 1, catDescription);
    // Make sure the description is null terminated.
    catinfo.szDescription[len + 1] = '\0';
 
    hr = pcr->RegisterCategories(1, &catinfo);
    pcr->Release();
 
    return hr;

*/


    ICatRegister* pcr = NULL ;	// interface pointer
    HRESULT hr = S_OK ;

    hr = CoCreateInstance(CLSID_StdComponentCategoriesMgr, 
			NULL, CLSCTX_INPROC_SERVER, IID_ICatRegister, (void**)&pcr);
	if (FAILED(hr))
		return hr;

    // Make sure the HKCR\Component Categories\{..catid...}
    // key is registered
    CATEGORYINFO catinfo;
    catinfo.catid = catid;
    catinfo.lcid = 0x0409 ; // english

	// Make sure the provided description is not too long.
	// Only copy the first 127 characters if it is
	int len = (int)(wcslen(catDescription));
	if (len>127)
		len = 127;

#if _MSC_VER >= 1400 //Shut up Visual Studio 2005
	wcsncpy_s(catinfo.szDescription, catDescription, len);
#else
	wcsncpy(catinfo.szDescription, catDescription, len);
#endif

	// Make sure the description is null terminated
	catinfo.szDescription[len] = '\0';

    hr = pcr->RegisterCategories(1, &catinfo);
	pcr->Release();

	return hr;
}



/////////////////////////////////////////////////////////////////////////////
// RegisterCLSIDInCategory - Registers class clsid as implementing category catid
// (Copied from ActiveX SDK docs)

HRESULT RegisterCLSIDInCategory(REFCLSID clsid, CATID catid)
{
// Register your component categories information.
    ICatRegister* pcr = NULL ;
    HRESULT hr = S_OK ;
    hr = CoCreateInstance(CLSID_StdComponentCategoriesMgr, 
			NULL, CLSCTX_INPROC_SERVER, IID_ICatRegister, (void**)&pcr);
    if (SUCCEEDED(hr))
    {
       // Register this category as being "implemented" by
       // the class.
       CATID rgcatid[1] ;
       rgcatid[0] = catid;
       hr = pcr->RegisterClassImplCategories(clsid, 1, rgcatid);
    }

    if (pcr != NULL)
        pcr->Release();
  
	return hr;
}

/////////////////////////////////////////////////////////////////////////////
// HRESULT UnRegisterCLSIDInCategory - Remove entries from the registry
// (Copied from ActiveX SDK docs)

HRESULT UnRegisterCLSIDInCategory(REFCLSID clsid, CATID catid)
{
    ICatRegister *pcr = NULL ;
    HRESULT hr = S_OK ;
 
    hr = CoCreateInstance(CLSID_StdComponentCategoriesMgr, 
            NULL, CLSCTX_INPROC_SERVER, IID_ICatRegister, (void**)&pcr);
    if (SUCCEEDED(hr))
    {
       // Unregister this category as being "implemented" by the class.
       CATID rgcatid[1] ;
       rgcatid[0] = catid;
       hr = pcr->UnRegisterClassImplCategories(clsid, 1, rgcatid);
    }
 
    if (pcr != NULL)
        pcr->Release();
 
    return hr;
}
