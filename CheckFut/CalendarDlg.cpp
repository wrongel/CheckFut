// CalendarDlg.cpp: файл реализации
//

#include "stdafx.h"
#include "CheckFut.h"
#include "CalendarDlg.h"
#include "afxdialogex.h"


// диалоговое окно CalendarDlg
/////////////////////////////////////////////////////////////////////////////
IMPLEMENT_DYNAMIC(CalendarDlg, CDialog)

CalendarDlg::CalendarDlg(CWnd* pParent /*=NULL*/)
	: CDialog(IDD_CALENDAR_DLG, pParent)
{
	m_pTime = NULL;
}

CalendarDlg::~CalendarDlg()
{
}

void CalendarDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_MONTHCALENDAR, m_Calendar);
}


BEGIN_MESSAGE_MAP(CalendarDlg, CDialog)
	ON_BN_CLICKED(IDOK, &CalendarDlg::OnBnClickedOk)
END_MESSAGE_MAP()


// обработчики сообщений CalendarDlg
/////////////////////////////////////////////////////////////////////////////

void CalendarDlg::GetTime(SYSTEMTIME * pTime)
{
	m_pTime = pTime;
}

void CalendarDlg::OnBnClickedOk()
{
	m_Calendar.GetCurSel(m_pTime);
	CDialog::OnOK();
}
