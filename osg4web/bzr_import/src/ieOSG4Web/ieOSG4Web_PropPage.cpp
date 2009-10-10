#include <ieOSG4Web/ieOSG4Web_stdafx.h>
#include <ieOSG4Web/ieOSG4Web.h>
#include <ieOSG4Web/ieOSG4Web_PropPage.h>


// IMPLEMENT_DYNCREATE
IMPLEMENT_DYNCREATE(PropPage, COlePropertyPage)
// Message map
BEGIN_MESSAGE_MAP(PropPage, COlePropertyPage)
END_MESSAGE_MAP()
// Initialize class factory and guid
IMPLEMENT_OLECREATE_EX(PropPage, "IEOSG4WEB.ieOSG4WebPropPage.1",	0x6809db26, 0x3fd5, 0x4508, 0x9e, 0x3d, 0x99, 0xbb, 0x83, 0xc3, 0xbd, 0x91)



BOOL PropPage::PropPageFactory::UpdateRegistry(BOOL bRegister)
{
	if (bRegister)
		return AfxOleRegisterPropertyPageClass(AfxGetInstanceHandle(),
			m_clsid, IDS_IEOSG4WEB_PPG);
	else
		return AfxOleUnregisterClass(m_clsid, NULL);
}

PropPage::PropPage() : COlePropertyPage(IDD, IDS_IEOSG4WEB_PPG_CAPTION),
	m_ADVCore(_T("")),
	m_ADVStartOptions(_T("")),
	m_ADVInitOptions(_T("")),
	m_ADVCoreSHA1Hash(_T("")),
	m_ADVCoreDep(_T("")),
	m_ADVCoreDepSHA1Hash(_T("")),
	m_LoadCBOptions(_T("")),
	m_EnableDiskLogs(_T(""))
{
}

void PropPage::DoDataExchange(CDataExchange* pDX)
{
	DDP_Text(pDX, IDC_ADVCORE, m_ADVCore, _T("ADVCore"));
	DDX_Text(pDX, IDC_ADVCORE, m_ADVCore);

	DDP_Text(pDX, IDC_ADVSTARTOPTIONS, m_ADVStartOptions, _T("ADVStartOptions"));
	DDX_Text(pDX, IDC_ADVSTARTOPTIONS, m_ADVStartOptions);

	DDP_Text(pDX, IDC_ADVINITOPTIONS, m_ADVInitOptions, _T("ADVInitOptions"));
	DDX_Text(pDX, IDC_ADVINITOPTIONS, m_ADVInitOptions);

	DDP_Text(pDX, IDC_ADVCORESHA1HASH, m_ADVCoreSHA1Hash, _T("ADVCoreSHA1Hash"));
	DDX_Text(pDX, IDC_ADVCORESHA1HASH, m_ADVCoreSHA1Hash);
		
	DDP_Text(pDX, IDC_ADVCOREDEP, m_ADVCoreDep, _T("ADVCoreDep"));
	DDX_Text(pDX, IDC_ADVCOREDEP, m_ADVCoreDep);

	DDP_Text(pDX, IDC_ADVCOREDEPSHA1HASH, m_ADVCoreDepSHA1Hash, _T("ADVCoreDepSHA1Hash"));
	DDX_Text(pDX, IDC_ADVCOREDEPSHA1HASH, m_ADVCoreDepSHA1Hash);

	DDP_Text(pDX, IDC_LOADCBOPTIONS, m_LoadCBOptions, _T("LoadCBOptions"));
	DDX_Text(pDX, IDC_LOADCBOPTIONS, m_LoadCBOptions);

	DDP_Text(pDX, IDC_ENABLEDISKLOGS, m_EnableDiskLogs, _T("EnableDiskLogs"));
	DDX_Text(pDX, IDC_ENABLEDISKLOGS, m_EnableDiskLogs);

	DDP_PostProcessing(pDX);
}