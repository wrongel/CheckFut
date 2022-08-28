#pragma once


// ���������� ���� PowerDlg

class PowerDlg : public CDialogEx
{
	DECLARE_DYNAMIC(PowerDlg)

public:
	PowerDlg(CWnd* pParent = NULL);   // ����������� �����������
	virtual ~PowerDlg();

// ������ ����������� ����
	enum { IDD = IDD_POWER };

protected:
	CFont m_MesFont;
	BOOL  m_bRecFlag;						// ���� �������� � EditChange
	CSize m_zFontSize;						// ������ ������ ������� � TextEdit
	TCHAR m_sI[32];
	TCHAR m_sU[32];

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // ��������� DDX/DDV
	void FontCreate();

	int Check_Number(UINT nId);							// �������� �������� ��������
	int Check_Input();									// �������� ���� ������ ��� ��������� ������ ��
	int Check_Measures();

	void   SendButtonMessage(UINT nID);

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedOk();
	afx_msg void OnChangeEditI();
	afx_msg void OnChangeEditU();
};
