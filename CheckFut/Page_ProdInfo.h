#pragma once


// ���������� ���� Page_ProdInfo

struct tagProdPageInfo
{
	DWORD nProduct;
	WORD  nStorage;					// 0 - ��� ����������, 1 - � ����������
	TCHAR tcsProdNum[256];
};

class Page_ProdInfo : public CPropertyPage
{
	DECLARE_DYNAMIC(Page_ProdInfo)

public:
	Page_ProdInfo();
	virtual ~Page_ProdInfo();

// ������ ����������� ����
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_PAGE_PRODINFO };
#endif

private:
	struct tagProdPageInfo m_zInfo;			// ���������� � ��������
	TCHAR m_tcsStorage[2][100];
	CSize m_zFontSize;
	BOOL m_bRecFlag;						// ���� �������� � EditChange

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // ��������� DDX/DDV

	void InitData();											// ������������� �����
	int Check_Number(UINT nId);									// �������� �������� ��������
	int Check_Input();											// �������� ���� ������ ��� ��������� ������ ��
	int Reflect_Source_Data(UINT nId, TCHAR *pData, int nLen);	// ��������� ������ � CEdit

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	virtual BOOL OnSetActive();
	afx_msg void OnSelchangeComboProduct();
	afx_msg void OnEditchangeComboProduct();
	afx_msg void OnChangeEditNumber();
	virtual LRESULT OnWizardNext();
	virtual LRESULT OnWizardBack();
	afx_msg void OnCbnSelchangeComboStorage();
	afx_msg void OnEditchangeComboStorage();
};
