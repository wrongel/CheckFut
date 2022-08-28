#pragma once
#include "afxcmn.h"

// ���������� ���� SettingsDlg

class SettingsDlg : public CDialogEx
{
	DECLARE_DYNAMIC(SettingsDlg)

public:
	SettingsDlg(CWnd* pParent = NULL);   // ����������� �����������
	virtual ~SettingsDlg();

private:
	class CReportCtrl* m_ListCtrl;		// ��������� �� CReportCtrl
	TCHAR**		  m_sFilesPaths;		// ���� � exe-������ ��������
	int m_nSetItem;						// ������ ������ ListCtrl ��� ���������

	void InitListCtrl();				// ������� ��������� �������� ��� ListCtrl
	void PrintItems();

// ������ ����������� ����
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_SETTINGS };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // ��������� DDX/DDV

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedCancel();
	afx_msg void OnDestroy();
};
