#pragma once

#define NUMLOCATE 2
// ���������� ���� Page_GenInfo

struct tagGenPageInfo
{
	int   nPosition;
	TCHAR lpszPosition[256];
	TCHAR lpszStorage[256];				// ��������� ������ ��� ����, ����� � next ����� ���� ������������ memcpy ������ ���� _tcscpy_s
	TCHAR lpszOperator[256];
	TCHAR lpszOtkMan[256];
	TCHAR lpszVpMan[256];
};

class Page_GenInfo : public CPropertyPage
{
	DECLARE_DYNAMIC(Page_GenInfo)

public:
	Page_GenInfo();
	virtual ~Page_GenInfo();

// ������ ����������� ����
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_PAGE_GENINFO };
#endif

private:
	struct tagGenPageInfo m_zInfo;			// ���������� � ��������
	CSize m_zFontSize;						// ������ � ������ ������
	BOOL m_bRecFlag;						// ���� �������� � EditChange
	TCHAR m_szLocate[NUMLOCATE][128];		// �������� ���� ����������

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // ��������� DDX/DDV

	void InitData();											// ������������� �����
	int Check_Char(UINT nId);									// �������� �������� ��������, bFlagNum - 0 ��� ���������� CEdit, 1 - ��� ���������
	int Check_Input();											// �������� ���� ������ ��� ��������� ������ ��
	int Reflect_Source_Data(UINT nId, TCHAR *pData, int nLen);	// ��������� ������ � CEdit

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	virtual BOOL OnSetActive();

	afx_msg void OnSelchangeComboLocate();
	afx_msg void OnEditchangeComboLocate();
	afx_msg void OnChangeEditOper();
	afx_msg void OnChangeEditOtk();
	afx_msg void OnChangeEditVp();
	virtual LRESULT OnWizardNext();
};
