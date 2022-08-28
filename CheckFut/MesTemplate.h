#pragma once


// ���������� ���� MesTemplate

class MesTemplate : public CDialogEx
{
	DECLARE_DYNAMIC(MesTemplate)

public:
	MesTemplate(CWnd* pParent = NULL);   // ����������� �����������
	virtual ~MesTemplate();

// ������ ����������� ����
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_MESSAGE };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // ��������� DDX/DDV
	void FontCreate();

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	virtual void OnOK();

	void SetStrings(TCHAR* sStringTop, TCHAR* sStringBottom);

protected:
	CFont m_MesFont;
	TCHAR sString1[100];
	TCHAR sString2[100];
};
