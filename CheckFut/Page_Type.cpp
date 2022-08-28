// Page_Type.cpp: файл реализации
//

#include "stdafx.h"
#include "CheckFut.h"
#include "Page_Type.h"
#include "afxdialogex.h"


// диалоговое окно Page_Type
///////////////////////////////////////////////////////////////////////////////////////
IMPLEMENT_DYNAMIC(Page_Type, CPropertyPage)

Page_Type::Page_Type()
	: CPropertyPage(IDD_PAGE_TYPE)
{
}

Page_Type::~Page_Type()
{
}

void Page_Type::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(Page_Type, CPropertyPage)
	ON_BN_CLICKED(IDC_RADIO_CONTROL, &Page_Type::OnBnClickedRadioControl)
	ON_BN_CLICKED(IDC_RADIO_REPORT, &Page_Type::OnBnClickedRadioReport)
END_MESSAGE_MAP()


// обработчики сообщений Page_Type
///////////////////////////////////////////////////////////////////////////////////////
BOOL Page_Type::OnSetActive()
{
	CPropertySheet* pSheet = GetParentSheet();
	if (IsDlgButtonChecked(IDC_RADIO_CONTROL) || IsDlgButtonChecked(IDC_RADIO_REPORT))
		pSheet->SetWizardButtons(PSWIZB_BACK | PSWIZB_NEXT);
	else
		pSheet->SetWizardButtons(PSWIZB_BACK);

	return CPropertyPage::OnSetActive();
}

///////////////////////////////////////////////////////////////////////////////////////
void Page_Type::OnBnClickedRadioControl()
{
	CPropertySheet* pSheet = (CPropertySheet*)GetParent();
	pSheet->SetWizardButtons(PSWIZB_BACK | PSWIZB_NEXT);
}

///////////////////////////////////////////////////////////////////////////////////////
void Page_Type::OnBnClickedRadioReport()
{
	CPropertySheet* pSheet = (CPropertySheet*)GetParent();
	pSheet->SetWizardButtons(PSWIZB_BACK | PSWIZB_NEXT);
}

///////////////////////////////////////////////////////////////////////////////////////
LRESULT Page_Type::OnWizardNext()
{
	CPropertySheet* pSheet = (CPropertySheet*)GetParent();
	if (IsDlgButtonChecked(IDC_RADIO_CONTROL))
		pSheet->SetActivePage(2);					// След страница контроля
	else
		pSheet->SetActivePage(1);					// След страница архива

	return CPropertyPage::OnWizardNext();
}