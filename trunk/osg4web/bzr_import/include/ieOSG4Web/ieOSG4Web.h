#pragma once

// ieOSG4Web.h : main header file for ieOSG4Web.DLL

#if !defined( __AFXCTL_H__ )
#error include 'afxctl.h' before including this file
#endif

#include "ieOSG4Web_res.h"       // main symbols


// CieOSG4WebApp : See ieOSG4Web.cpp for implementation.

class CieOSG4WebApp : public COleControlModule
{
public:
	BOOL InitInstance();
	int ExitInstance();
};

extern const GUID CDECL _tlid;
extern const WORD _wVerMajor;
extern const WORD _wVerMinor;

