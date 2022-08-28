#pragma once
#include "afxcmn.h"

// диалоговое окно SettingsDlg

class SettingsDlg : public CDialogEx
{
	DECLARE_DYNAMIC(SettingsDlg)

public:
	SettingsDlg(CWnd* pParent = NULL);   // стандартный конструктор
	virtual ~SettingsDlg();

private:
	class CReportCtrl* m_ListCtrl;		// Указатель на CReportCtrl
	TCHAR**		  m_sFilesPaths;		// Пути к exe-файлам программ
	int m_nSetItem;						// Индекс строки ListCtrl для изменения

	void InitListCtrl();				// Задание начальных значений для ListCtrl
	void PrintItems();

// Данные диалогового окна
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_SETTINGS };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // поддержка DDX/DDV

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedCancel();
	afx_msg void OnDestroy();
};
