#pragma once
#include "afxwin.h"
//-----------------------------------------------------------------------------
class PropPage : public COlePropertyPage
//-----------------------------------------------------------------------------
{
	DECLARE_DYNCREATE(PropPage)
	DECLARE_OLECREATE_EX(PropPage)

public:
	PropPage();

// Dialog Data
	enum { IDD = IDD_PROPPAGE_IEOSG4WEB };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

protected:
	DECLARE_MESSAGE_MAP()
public:
	CString m_ADVCore;
	CString m_ADVStartOptions;
	CString m_ADVInitOptions;
	CString m_ADVCoreSHA1Hash;
	CString m_LoadCBOptions;
};
