#pragma once
#include "afxdtctl.h"


// диалоговое окно CalendarDlg

class CalendarDlg : public CDialog
{
	DECLARE_DYNAMIC(CalendarDlg)

public:
	CalendarDlg(CWnd* pParent = NULL);   // стандартный конструктор
	virtual ~CalendarDlg();

// Данные диалогового окна
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_CALENDAR_DLG };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // поддержка DDX/DDV

	DECLARE_MESSAGE_MAP()
private:
	CMonthCalCtrl m_Calendar;
	SYSTEMTIME*  m_pTime;
public:
	void GetTime(SYSTEMTIME* pTime);
	afx_msg void OnBnClickedOk();
};
