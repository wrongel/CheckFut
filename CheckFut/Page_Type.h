#pragma once
#include "afxwin.h"


// ���������� ���� Page_Type

class Page_Type : public CPropertyPage
{
	DECLARE_DYNAMIC(Page_Type)

public:
	Page_Type();
	virtual ~Page_Type();

// ������ ����������� ����
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_PAGE_TYPE };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // ��������� DDX/DDV

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnSetActive();
	afx_msg void OnBnClickedRadioControl();
	afx_msg void OnBnClickedRadioReport();
	virtual LRESULT OnWizardNext();
};
