// Page_List.cpp: файл реализации
//

#include "stdafx.h"
#include "CheckFut.h"
#include "Page_List.h"
#include "ReportCtrl.h"
#include "SettingsDlg.h"
#include "afxdialogex.h"
#include "CheckFut.hxx"

#define PROGRESSRANGE 10000
#define PROGRESSSTEP  10

///////////////////////////////////////////////////////////////////////////////////////
// диалоговое окно Page_List

IMPLEMENT_DYNAMIC(Page_List, CPropertyPage)

Page_List::Page_List()
	: CPropertyPage(IDD_PAGE_LIST)
{
	m_ListCtrl = NULL;
	m_nChecked = 0;
	m_nCheckEnable = 1;
	m_nRecFlag = 0;
}

Page_List::~Page_List()
{
	// Удалить ListCtrl
	if (m_ListCtrl != NULL)
	{
		delete m_ListCtrl;
		m_ListCtrl = NULL;
	}
}

void Page_List::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(Page_List, CPropertyPage)
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_LIST, &Page_List::OnItemchangedList)
	ON_BN_CLICKED(IDC_BUTTON_START, &Page_List::OnBnClickedButtonStart)
	ON_BN_CLICKED(IDC_BUTTON_STOP, &Page_List::OnBnClickedButtonStop)
	ON_BN_CLICKED(IDC_BUTTON_REPORT, &Page_List::OnBnClickedButtonReport)
END_MESSAGE_MAP()

///////////////////////////////////////////////////////////////////////////////////////
// обработчики сообщений Page_List

BOOL Page_List::OnInitDialog()
{
	CPropertyPage::OnInitDialog();

	InitListCtrl();
	//PrintCheckNames();

	return TRUE;  // return TRUE unless you set the focus to a control
				  // Исключение: страница свойств OCX должна возвращать значение FALSE
}

///////////////////////////////////////////////////////////////////////////////////////
BOOL Page_List::OnSetActive()
{
	CPropertySheet* pSheet = (CPropertySheet*)GetParent();
	TCHAR szProduct[128] = _T("Контроль технического состояния ");

	pSheet->SetWizardButtons(PSWIZB_BACK);
	_tcscat_s(szProduct, g_zProdTypes[g_zGenInfo.nProduct].szProdExt);
	SetDlgItemText(IDC_EDIT_HEAD, szProduct);

	PrintCheckNames();		// В зависимости от выбранной конфигурации


	return CPropertyPage::OnSetActive();
}

///////////////////////////////////////////////////////////////////////////////////////
void Page_List::SetProgBorder()
{
	m_nTests  = 0;
	m_iTIndex = 0;

	for (int i = 0; i < g_nNumTests; i++)
		if (ListView_GetCheckState(GetDlgItem(IDC_LIST)->GetSafeHwnd(), i) > 0)
			m_nTests += g_zExeInfo[g_nTestIndex[i]].iCountRun;

	m_nBorder = PROGRESSRANGE / m_nTests;
}

///////////////////////////////////////////////////////////////////////////////////////
void Page_List::InitListCtrl()
{
	CRect rectCtrl;
	TCHAR sFormatHeader[50];
	int nColumsWidths[3];

	m_ListCtrl = (CReportCtrl*) new class CReportCtrl();
	m_ListCtrl->AssignList(this, IDC_LIST);

	m_ListCtrl->GetClientRect(rectCtrl);
	nColumsWidths[0] = (int)(rectCtrl.Width() * 0.033);
	nColumsWidths[1] = (int)(rectCtrl.Width() * 0.7);
	nColumsWidths[2] = (int)(rectCtrl.Width() * 0.267);
	_stprintf_s(sFormatHeader, _T("Все! %d! 1;Контроль! %d;Состояние! %d"), nColumsWidths[0], nColumsWidths[1], nColumsWidths[2]);

	m_ListCtrl->SetColumnHeader(sFormatHeader);
	//m_ListCtrl->ModifyStyle(WS_HSCROLL, 0);
	//m_ListCtrl->EnableScrollBar(SB_HORZ, ESB_DISABLE_BOTH);

	m_ListCtrl->SetGridLines(TRUE); // SHow grid lines
	m_ListCtrl->SetCheckboxeStyle(RC_CHKBOX_NORMAL); // Enable checkboxes
	m_ListCtrl->SetSortable(0);

	m_ListCtrl->SetHeaderImageList(IDB_BITMAP1);
	// При переходе на страницу отмечены все чекбоксы
	m_ListCtrl->SetHeaderImage(0, 1);
}

///////////////////////////////////////////////////////////////////////////////////////
void Page_List::PrintCheckNames()
{
	TCHAR tcsName[128];

	m_ListCtrl->DeleteAllItems();
	for (int i = 0; i < NUMCHECKS; i++)
	{
		if ((g_nMode & g_zExeInfo[i].nMode) == g_nMode)
		{
			const int IDX = m_ListCtrl->InsertItem(i, _T(""));
			_stprintf_s(tcsName, _T("%s"), g_zExeInfo[i].lpszCheckName);		// 31.05.2019 убрал названия перед тестами
			m_ListCtrl->SetItemText(IDX, 1, tcsName);
			// При переходе на страницу отмечены все чекбоксы
			ListView_SetCheckState(m_ListCtrl->m_hWnd, IDX, 1);
			//_tcscpy_s(sPrevName, g_zExeInfo[i].lpszCheckName);
			g_nTestIndex[IDX] = i;
		}
	}
	g_nNumTests = m_ListCtrl->GetItemCount();
}

//=====================================================================================
void Page_List::PrintResult(int nItem, int nSubItem, int nResult)
{	
	//m_ListCtrl->SetItemTextColor(nItem, -1, RGB(0, 0, 0));
	switch (nResult)
	{
		case -88:
			m_ListCtrl->SetItemText(nItem, nSubItem, _T(""));			
			m_ListCtrl->SetItemBkColor(nItem, -1, GetSysColor(COLOR_WINDOW));
			break;
		case 1:
			m_ListCtrl->SetItemText(nItem, nSubItem, _T("РАБОТОСПОСОБНОЕ"));
			m_ListCtrl->SetItemBkColor(nItem, -1, RGB(132, 255, 132));
			break;
		default:
			m_ListCtrl->SetItemText(nItem, nSubItem, _T("НЕРАБОТОСПОСОБНОЕ"));
			m_ListCtrl->SetItemBkColor(nItem, -1, RGB(255, 125, 125));
			break;
	}

	//m_ListCtrl->EnsureVisible(m_ListCtrl->GetItemCount() - 1, 0);
}

//=====================================================================================
void Page_List::SetActiveTest(int nItem)
{
	m_ListCtrl->SetItemState(nItem, LVIS_SELECTED, LVIS_SELECTED);

	//m_ListCtrl->SetItemBkColor(nItem, -1, GetSysColor(COLOR_HIGHLIGHT));
	//m_ListCtrl->SetItemTextColor(nItem, -1, GetSysColor(COLOR_HIGHLIGHTTEXT));
}

//=====================================================================================
void Page_List::IncTestIndex()
{
	m_iTIndex++;
}

//=====================================================================================
void Page_List::EnableButtons(BOOL bFlag, BOOL bWhiteFlag)
{
	//GetDlgItem(IDC_LIST)->EnableWindow(bFlag);
	m_ListCtrl->CheckBoxEnable(bFlag);
	m_nCheckEnable = bFlag;
	// Отдельно задается цвет для строк
	if (bFlag == 0)
		m_ListCtrl->SetItemBkColor(-1, -1, GetSysColor(COLOR_MENUBAR));
	
	if (bWhiteFlag == 1)
		m_ListCtrl->SetItemBkColor(-1, -1, RGB(255,255,255));

	GetDlgItem(IDC_BUTTON_START)->EnableWindow(bFlag);
	GetDlgItem(IDC_BUTTON_STOP)->EnableWindow(!bFlag);
	GetDlgItem(IDC_BUTTON_REPORT)->EnableWindow(bFlag);
	GetParent()->GetDlgItem(ID_WIZBACK)->EnableWindow(bFlag);
	GetParent()->GetDlgItem(IDCANCEL)->EnableWindow(bFlag);
}

///////////////////////////////////////////////////////////////////////////////////////
int Page_List::RunSettings()
{
	int nRet;

	SettingsDlg dlgSet;
	nRet = dlgSet.DoModal();

	return nRet;
}

///////////////////////////////////////////////////////////////////////////////////////
//void Page_List::ProgressCorrect(float nPos, int nTest, int nCountRun /*int nMaxRun*/)
//{
//	CProgressCtrl* pProgress = (CProgressCtrl*)GetDlgItem(IDC_PROGRESS);
//
//	//int nBorder = PROGRESSRANGE / m_nChecked;		// бордер должен учитывать и колво запусков теста
//	//float fRun = (float)nCountRun / nMaxRun;
//
//	int nNewPos = (int)(m_nBorder * (nTest + nCountRun + nPos /** fRun*/));			//nTest * nBorder + nPos * nBorder;
//
//	pProgress->SetPos(nNewPos);
//}

void Page_List::ProgressCorrect(float nPos)
{
	CProgressCtrl* pProgress = (CProgressCtrl*)GetDlgItem(IDC_PROGRESS);

	int nNewPos = (int)(m_nBorder * (m_iTIndex + nPos));
	if (m_iTIndex == m_nTests)
		m_iTIndex = 0;

	POSITION a = m_ListCtrl->GetFirstSelectedItemPosition();
	

	pProgress->SetPos(nNewPos);
}

///////////////////////////////////////////////////////////////////////////////////////
void Page_List::OnItemchangedList(NMHDR *pNMHDR, LRESULT *pResult)
{
	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;
	*pResult = 0;

	// Добавка для отсутствия лишних рекурсий
	if (m_nRecFlag)
	{
		m_nRecFlag = 0;
		return;
	}
	/////////////////////////////////////////

	if (pNMListView->uOldState == 0 && pNMListView->uNewState == 0)
		return;	// No change

	BOOL bPrevState = (BOOL)(((pNMListView->uOldState &
		LVIS_STATEIMAGEMASK) >> 12) - 1);   // Old check box state
	if (bPrevState < 0)	// On startup there's no previous state 
	{
		return;
		//bPrevState = 0;		// so assign as false (unchecked), раньше было так, изменено для недоступности чекбоксов во время проверки и отсутствия рекурсии
	}

	// Если идет контроль, чекбоксы не изменять
	if (m_nCheckEnable == 0)
	{
		m_nRecFlag = 1;
		ListView_SetCheckState(pNMListView->hdr.hwndFrom, pNMListView->iItem, bPrevState);
		return;
	}

	// New check box state
	BOOL bChecked = (BOOL)(((pNMListView->uNewState & LVIS_STATEIMAGEMASK) >> 12) - 1);
	if (bChecked < 0) // On non-checkbox notifications assume false
		bChecked = 0;

	if (bPrevState == bChecked) // No change in check box
		return;

	if (bChecked == 1)
		m_nChecked++;
	else if (bPrevState == 1)
		m_nChecked--;

	if (m_nChecked != 0)
		GetDlgItem(IDC_BUTTON_START)->EnableWindow(1);
	else
		GetDlgItem(IDC_BUTTON_START)->EnableWindow(0);
}

///////////////////////////////////////////////////////////////////////////////////////
void Page_List::OnBnClickedButtonStart()
{
	// Проверка существования exe-файлов
	TCHAR sCheckNames[1024] = _T("\0");				// Названия тестов с некорректными путями
	TCHAR sMes[1024];
	int nRet = theApp.CheckExeExist(sCheckNames, _countof(sCheckNames));

	while (nRet != 0)
	{
		_stprintf_s(sMes, _T("Не найдены исполняющие файлы для %d программ:\n%s%s"), nRet, sCheckNames, _T("Перейти в настройки для выбора файлов?"));

		nRet = AfxMessageBox(sMes, MB_YESNO | MB_ICONQUESTION);
		if (nRet == IDNO)
			return;

		nRet = RunSettings();
		if (nRet == IDCANCEL)
			return;

		_tcscpy_s(sCheckNames, _T("\0"));
		nRet = theApp.CheckExeExist(sCheckNames, _countof(sCheckNames));
	}

	CProgressCtrl* pProgress = (CProgressCtrl*)GetDlgItem(IDC_PROGRESS);
	pProgress->SetRange(0, PROGRESSRANGE);
	pProgress->SetStep(PROGRESSSTEP);
	pProgress->SetPos(0);

	m_ListCtrl->CheckBoxEnable(0);
	EnableButtons(0, 0);
	SetProgBorder();
	g_bProcessEnd = 0;
	//g_iCurProgCheck = 0;
	g_iRun = 14;	//1

}

///////////////////////////////////////////////////////////////////////////////////////
void Page_List::OnBnClickedButtonStop()
{
	g_bProcessEnd = 1;
}

///////////////////////////////////////////////////////////////////////////////////////
void Page_List::OnBnClickedButtonReport()
{
	CFileFind FileFind;
	if (FileFind.FindFile(g_sReportPath) == 0)			// Если файл протокола не найден
	{
		MessageBox(_T("Не найден файл протокола"), _T("ОШИБКА"), MB_OK | MB_ICONERROR);
		return;
	}

	TCHAR sFile[MAX_PATH];
	// Вызовы командной строки 
	// rundll32.exe shell32.dll, OpenAs_RunDLL C:\\MyFile.pdf - диалог Открыть с помощью
	// rundll32.exe url.dll,FileProtocolHandler C:\\MyFile.pdf - открыть ассоциированным приложением
	_stprintf_s(sFile, _T("%s%s"), _T("rundll32.exe url.dll, FileProtocolHandler "), g_sReportPath);
	::WinExec(CT2A(sFile), SW_SHOWNORMAL);
}
