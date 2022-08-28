// SettingsDlg.cpp: файл реализации
//

#include "stdafx.h"
#include "CheckFut.h"
#include "SettingsDlg.h"
#include "ReportCtrl.h"
#include "Config.h"
#include "CheckFut.hxx"
#include "afxdialogex.h"

/////////////////////////////////////////////////////////////////////////////
// диалоговое окно SettingsDlg

IMPLEMENT_DYNAMIC(SettingsDlg, CDialogEx)

SettingsDlg::SettingsDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_SETTINGS, pParent)
{
	m_ListCtrl = NULL;

	m_sFilesPaths = new TCHAR*[NUMCHECKS];
	for (int i = 0; i < NUMCHECKS; i++)
	{
		m_sFilesPaths[i] = new TCHAR[1024];
		m_sFilesPaths[i][0] = '\0';
	}
}

SettingsDlg::~SettingsDlg()
{
	if (m_ListCtrl != NULL)
	{
		delete m_ListCtrl;
		m_ListCtrl = NULL;
	}
}

void SettingsDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(SettingsDlg, CDialogEx)
	ON_BN_CLICKED(IDOK, &SettingsDlg::OnBnClickedOk)
	ON_BN_CLICKED(IDCANCEL, &SettingsDlg::OnBnClickedCancel)
	ON_WM_DESTROY()
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// обработчики сообщений SettingsDlg

BOOL SettingsDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	InitListCtrl();
	PrintItems();

	return TRUE;  // return TRUE unless you set the focus to a control
				  // Исключение: страница свойств OCX должна возвращать значение FALSE
}

void SettingsDlg::InitListCtrl()
{
	CRect rectCtrl;
	TCHAR sFormatHeader[100];
	int nColumsWidths[4];

	m_ListCtrl = (CReportCtrl*) new class CReportCtrl();
	m_ListCtrl->AssignList(this, IDC_LIST_SETTINGS);

	m_ListCtrl->GetClientRect(rectCtrl);
	nColumsWidths[0] = (int)(rectCtrl.Width() * 0.24);
	nColumsWidths[1] = (int)(rectCtrl.Width() * 0.35);
	nColumsWidths[2] = (int)(rectCtrl.Width() * 0.205);
	nColumsWidths[3] = (int)(rectCtrl.Width() * 0.205);
	_stprintf_s(sFormatHeader, _T("Программа! %d! 1;Путь к exe-файлу! %d! 1! 1;Параметры запуска! %d! 1;Имя протокола! %d! 1"), 
		nColumsWidths[0], nColumsWidths[1], nColumsWidths[2], nColumsWidths[2]);

	m_ListCtrl->SetColumnHeader(sFormatHeader);
	//m_ListCtrl->SetColumnHeader(_T("Программа, 350, 1;Путь к exe-файлу, 515, 1, 1;Параметры запуска, 300, 1;Имя протокола, 300, 1"));

	m_ListCtrl->SetGridLines(TRUE); // SHow grid lines
	m_ListCtrl->SetEditable(TRUE);  // Allow sub-text edit
	m_ListCtrl->SetSortable(0);
}

/////////////////////////////////////////////////////////////////////////////
void SettingsDlg::PrintItems()
{
	CFileFind Find;
	TCHAR tcsName[128] = _T("");

	for (int i = 0; i < NUMCHECKS; i++)
	{
		const int IDX = m_ListCtrl->InsertItem(i, _T(""));

		_tcscpy_s(tcsName, g_zExeInfo[i].lpszCheckName);
		m_ListCtrl->SetItemText(IDX, 0, tcsName);
		
		if (Find.FindFile(g_zExeInfo[i].lpszPath) == 0)
			m_ListCtrl->SetItemText(IDX, 1, _T("Некоррекный путь"));
		else
			m_ListCtrl->SetItemText(IDX, 1, g_zExeInfo[i].lpszPath);

		m_ListCtrl->SetItemText(IDX, 2, g_zExeInfo[i].lpszCmdPar[0]);		// Выводить только первые параметры ком строки
		m_ListCtrl->SetItemText(IDX, 3, g_zExeInfo[i].lpszRepName);
	}
}

/////////////////////////////////////////////////////////////////////////////
void SettingsDlg::OnBnClickedOk()
{	
	TCHAR tcsExePath[MAX_PATH];

	m_ListCtrl->EndEdit(1);						// Убирает необходимость выходить из CEdit после редактирования
	for (int i = 0; i < NUMCHECKS; i++)
	{
		_tcscpy_s(tcsExePath, m_ListCtrl->GetItemText(i, 1));
		if (tcsExePath[_tcslen(tcsExePath) - 1] != _T(' '))
			_tcscat_s(tcsExePath, _T(" "));

		_tcscpy_s(g_zExeInfo[i].lpszCheckName, m_ListCtrl->GetItemText(i, 0));
		_tcscpy_s(g_zExeInfo[i].lpszPath,   tcsExePath);
		_tcscpy_s(g_zExeInfo[i].lpszCmdPar[0], m_ListCtrl->GetItemText(i, 2));
		_tcscpy_s(g_zExeInfo[i].lpszRepName,   m_ListCtrl->GetItemText(i, 3));
	}

	g_pCFG->SaveConfig();

	CDialogEx::OnOK();
}

/////////////////////////////////////////////////////////////////////////////
void SettingsDlg::OnBnClickedCancel()
{
	// TODO: добавьте свой код обработчика уведомлений
	CDialogEx::OnCancel();
}

/////////////////////////////////////////////////////////////////////////////
void SettingsDlg::OnDestroy()
{
	CDialogEx::OnDestroy();

	if (m_sFilesPaths != NULL)
	{
		for (int i = 0; i < NUMCHECKS; i++)
		{
			delete[] m_sFilesPaths[i];
			m_sFilesPaths[i] = NULL;
		}
		delete[] m_sFilesPaths;
		m_sFilesPaths = NULL;
	}
}
