// ieOSG4Web.cpp : Implementation of CieOSG4WebApp and DLL registration.

#include <ieOSG4Web/ieOSG4Web_stdafx.h>
#include "comcat.h"
#include "strsafe.h"
#include "objsafe.h"
#include <ieOSG4Web/ieOSG4Web_Helpers.h>
#include <ieOSG4Web/ieOSG4Web.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


CieOSG4WebApp NEAR theApp;

const GUID CDECL BASED_CODE _tlid =
		{ 0x5B3E8016, 0x185D, 0x47AB, { 0x84, 0x73, 0x40, 0x8C, 0x8B, 0x4C, 0x29, 0x78 } };
const WORD _wVerMajor = 1;
const WORD _wVerMinor = 0;

// CLSID_SafeItem - Necessary for safe ActiveX control
// Id taken from IMPLEMENT_OLECREATE_EX function in xxxCtrl.cpp

const CATID CLSID_SafeItem =
{ 0x559f0dcd, 0x759e, 0x48ee,{ 0xb1, 0xf2, 0xc9, 0x17, 0xaa, 0x6f, 0xb7, 0xea}};

// CieOSG4WebApp::InitInstance - DLL initialization

BOOL CieOSG4WebApp::InitInstance()
{
	BOOL bInit = COleControlModule::InitInstance();

	if (bInit)
	{
		// TODO: Add your own module initialization code here.
	}

	return bInit;
}


// CieOSG4WebApp::ExitInstance - DLL termination

int CieOSG4WebApp::ExitInstance()
{
	// TODO: Add your own module termination code here.

	return COleControlModule::ExitInstance();
}


// DllRegisterServer - Adds entries to the system registry

STDAPI DllRegisterServer(void)
{
	HRESULT  hr;    // return for safety functions

	AFX_MANAGE_STATE(_afxModuleAddrThis);

	if (!AfxOleRegisterTypeLib(AfxGetInstanceHandle(), _tlid))
		return ResultFromScode(SELFREG_E_TYPELIB);

	if (!COleObjectFactoryEx::UpdateRegistryAll(TRUE))
		return ResultFromScode(SELFREG_E_CLASS);

	// Mark the control as safe for initializing.
                                             
    hr = CreateComponentCategory(CATID_SafeForInitializing, 
         L"Controls safely initializable from persistent data!");
    if (FAILED(hr))
      return hr;
 
    hr = RegisterCLSIDInCategory(CLSID_SafeItem, 
         CATID_SafeForInitializing);
    if (FAILED(hr))
        return hr;
 
    // Mark the control as safe for scripting.
 
    hr = CreateComponentCategory(CATID_SafeForScripting, 
                                 L"Controls safely  scriptable!");
    if (FAILED(hr))
        return hr;
 
    hr = RegisterCLSIDInCategory(CLSID_SafeItem, 
                        CATID_SafeForScripting);
    if (FAILED(hr))
        return hr;

	return NOERROR;
}



// DllUnregisterServer - Removes entries from the system registry

STDAPI DllUnregisterServer(void)
{
	HRESULT  hr;    // return for safety functions

	AFX_MANAGE_STATE(_afxModuleAddrThis);

	if (!AfxOleUnregisterTypeLib(_tlid, _wVerMajor, _wVerMinor))
		return ResultFromScode(SELFREG_E_TYPELIB);

	if (!COleObjectFactoryEx::UpdateRegistryAll(FALSE))
		return ResultFromScode(SELFREG_E_CLASS);

	// Remove entries from the registry.
 
    hr=UnRegisterCLSIDInCategory(CLSID_SafeItem, 
                     CATID_SafeForInitializing);
    if (FAILED(hr))
      return hr;
 
    hr=UnRegisterCLSIDInCategory(CLSID_SafeItem, 
                        CATID_SafeForScripting);
    if (FAILED(hr))
      return hr;

	return NOERROR;
}
