// MainSheet.cpp: файл реализации
//

#include "stdafx.h"
#include "afxpriv.h"						// afxpriv.h и afximpl.h для переопределения Create
#include "..\src\mfc\afximpl.h"
#include "CheckFut.h"
#include "MainSheet.h"
#include "Page_GenInfo.h"
#include "Page_Type.h"
#include "Page_Archive.h"
#include "Page_ProdInfo.h"
#include "Page_List.h"
#include "CheckFut.hxx"


/////////////////////////////////////////////////////////////////////////////
// CAboutDlg dialog

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

	// Данные диалогового окна
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // поддержка DDX/DDV

														// Реализация
protected:
	DECLARE_MESSAGE_MAP()

public:
	virtual BOOL OnInitDialog();
};

///////////////////////////////////////////////////////////////////////////////////////
CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX)
{
}

///////////////////////////////////////////////////////////////////////////////////////
void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

///////////////////////////////////////////////////////////////////////////////////////
BOOL CAboutDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	TCHAR sText[100] = _T("Программа контроля\r\nтехнического состояния.\r\nВерсия ");
	TCHAR sCurVersion[64];
	theApp.GetAppVersion(sCurVersion);
	_tcscat_s(sText, sCurVersion);
	SetDlgItemText(IDC_EDIT_VERSION, sText);

	return TRUE;  // return TRUE unless you set the focus to a control
				  // Исключение: страница свойств OCX должна возвращать значение FALSE
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// MainSheet

IMPLEMENT_DYNAMIC(MainSheet, CPropertySheet)

MainSheet::MainSheet(UINT nIDCaption, CWnd* pParentWnd, UINT iSelectPage)
	:CPropertySheet(nIDCaption, pParentWnd, iSelectPage)
{
}

///////////////////////////////////////////////////////////////////////////////////////
MainSheet::MainSheet(LPCTSTR pszCaption, CWnd* pParentWnd, UINT iSelectPage)
	:CPropertySheet(pszCaption, pParentWnd, iSelectPage)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDI_ICON_HEAD);

	m_Page_GenInfo  = new Page_GenInfo ();
	m_Page_Type     = new Page_Type();
	m_Page_Archive  = new Page_Archive();
	m_Page_ProdInfo = new Page_ProdInfo();
	m_Page_List     = new Page_List();

	this->AddPage(m_Page_GenInfo);
	this->AddPage(m_Page_Type);
	this->AddPage(m_Page_Archive);
	this->AddPage(m_Page_ProdInfo);
	this->AddPage(m_Page_List);

	this->SetWizardMode();
	this->Create();
}

//============================================================================
MainSheet::~MainSheet()
{
	ClearPageMem(m_Page_GenInfo);
	ClearPageMem(m_Page_Type);
	ClearPageMem(m_Page_Archive);
	ClearPageMem(m_Page_ProdInfo);
	ClearPageMem(m_Page_List);
}

BEGIN_MESSAGE_MAP(MainSheet, CPropertySheet)
	ON_WM_CONTEXTMENU()
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// обработчики сообщений MainSheet

void MainSheet::ClearPageMem(CPropertyPage * pPage)
{
	if (pPage != NULL)
	{
		delete pPage;
		pPage = NULL;
	}
}

///////////////////////////////////////////////////////////////////////////////////////
int MainSheet::MainSheetCallback(HWND hWndDlg, UINT uMsg, LPARAM lParam)
{
	switch (uMsg)
	{
		case PSCB_PRECREATE:
		{
			/*if ((m_wFontSize > 0) && (NULL != m_pszFontFaceName))
			{*/
			LPDLGTEMPLATE pResource = (LPDLGTEMPLATE)lParam;
			CDialogTemplate dlgTemplate(pResource);
			dlgTemplate.SetFont(m_sFontName, m_wFontSize);
			memmove((void*)lParam, dlgTemplate.m_hTemplate, dlgTemplate.m_dwTemplateSize);
			//}
		}
		break;
	}
	return 0;
}

///////////////////////////////////////////////////////////////////////////////////////
void MainSheet::ErrorMessage()
{
	DWORD               dwError;
	TCHAR               lpszMsg[1024];

	dwError = (DWORD)::GetLastError();
	::FormatMessage(
		FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL,
		dwError,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
		lpszMsg,
		_countof(lpszMsg),
		NULL
	);
	::MessageBox(NULL, lpszMsg, _T("Error"), MB_OK | MB_ICONINFORMATION);
}

///////////////////////////////////////////////////////////////////////////
void MainSheet::BuildPropPageArray()
{
	CPropertySheet::BuildPropPageArray();

	/*if ((m_wFontSize > 0) && (NULL != m_pszFontFaceName))
	{*/
	LPCPROPSHEETPAGE ppsp = m_psh.ppsp;
	const int nSize = static_cast<int>(m_pages.GetSize());
	for (int nPage = 0; nPage < nSize; nPage++)
	{
		const DLGTEMPLATE* pResource = ppsp->pResource;
		CDialogTemplate dlgTemplate(pResource);
		dlgTemplate.SetFont(m_sFontName, m_wFontSize);
		memmove((void*)pResource, dlgTemplate.m_hTemplate, dlgTemplate.m_dwTemplateSize);
		(BYTE*&)ppsp += ppsp->dwSize;
	}
	//}
}

/////////////////////////////////////////////////////////////////////////////////////
BOOL MainSheet::Create(CWnd * pParentWnd, DWORD dwStyle, DWORD dwExStyle)
{
	ENSURE(0 == (m_psh.dwFlags & PSH_AEROWIZARD));

	_AFX_THREAD_STATE* pState = AfxGetThreadState();

	// Calculate the default window style.
	if (dwStyle == (DWORD)-1)
	{
		pState->m_dwPropStyle = DS_MODALFRAME | DS_3DLOOK | DS_CONTEXTHELP |
			DS_SETFONT | WS_POPUP | WS_VISIBLE | WS_CAPTION;

		// Wizards don't have WS_SYSMENU.
		if (!IsWizard())
			pState->m_dwPropStyle |= WS_SYSMENU;
	}
	else
	{
		pState->m_dwPropStyle = dwStyle;
	}
	pState->m_dwPropExStyle = dwExStyle;

	ASSERT_VALID(this);
	ASSERT(m_hWnd == NULL);

	VERIFY(AfxDeferRegisterClass(AFX_WNDCOMMCTLS_REG));
	AfxDeferRegisterClass(AFX_WNDCOMMCTLSNEW_REG);

#ifdef _UNICODE
	AfxInitNetworkAddressControl();
#endif

	// finish building PROPSHEETHEADER structure
	BuildPropPageArray();
	m_bModeless = TRUE;
	m_psh.dwFlags |= (PSH_MODELESS | PSH_USECALLBACK);
	////////////////////////////////////
	//m_psh.pfnCallback = AfxPropSheetCallback;
	m_psh.pfnCallback = MainSheetCallback;

	m_psh.hwndParent = pParentWnd->GetSafeHwnd();

	// hook the window creation process
	AfxHookWindowCreate(this);
	HWND hWnd = (HWND)PropertySheet(&m_psh);
#ifdef _DEBUG
	DWORD dwError = ::GetLastError();
#endif

	if (hWnd == (HWND)-1)
	{
		// An error occurred in the creation of the sheet, so assuming that the window
		// was destroyed and cleanup was done via PostNcDestroy, which was called.  If a
		// memory leak is reported, the window was not deleted in class::PostNcDestroy.
		return FALSE;
	}

	// cleanup on failure, otherwise return TRUE
	if (!AfxUnhookWindowCreate())
		PostNcDestroy();    // cleanup if Create fails

							// setting a custom property in our window
	HGLOBAL hMem = GlobalAlloc(GPTR, sizeof(BOOL));
	BOOL* pBool = static_cast<BOOL*>(GlobalLock(hMem));
	if (pBool != NULL)
	{
		*pBool = TRUE;
		GlobalUnlock(hMem);
		if (SetProp(this->m_hWnd, _T("AfxClosePending"), hMem) == 0)
		{
			GlobalFree(hMem);
			this->DestroyWindow();
			return FALSE;
		}
	}
	else
	{
		this->DestroyWindow();
		return FALSE;
	}

	if (hWnd == NULL || hWnd == (HWND)-1)
	{
#ifdef _DEBUG
		TRACE(traceAppMsg, 0, "PropertySheet() failed: GetLastError returned %d\n", dwError);
#endif
		return FALSE;
	}

	ASSERT(hWnd == m_hWnd);
	return TRUE;
}

///////////////////////////////////////////////////////////////////////////////////////
BOOL MainSheet::OnInitDialog()
{
	BOOL bResult = CPropertySheet::OnInitDialog();

	GetDlgItem(IDHELP)->ShowWindow(0);
	SetDlgItemText(IDCANCEL, _T("ВЫХОД"));

	// Изменение размеров кнопок управления визардом
	int ids[] = { ID_WIZBACK, ID_WIZNEXT, IDCANCEL };
	CRect rectWnd;
	CRect rectBtn;

	GetWindowRect(rectWnd);
	GetDlgItem(ID_WIZBACK)->GetWindowRect(rectBtn);

	int btnOffset = rectWnd.bottom - rectBtn.bottom;
	int btnWidth = ((rectWnd.Width() - btnOffset * 4) / 3);
	ScreenToClient(rectBtn);

	for (int i = 0; i < _countof(ids); i++)
	{		
		rectBtn.left  = (i + 1) * btnOffset + btnWidth * i;
		rectBtn.right = rectBtn.left + btnWidth;
		GetDlgItem(ids[i])->MoveWindow(rectBtn);
	}

	// Иконка
	//SetIcon(m_hIcon, TRUE);			// Крупный значок
	SetIcon(m_hIcon, FALSE);		// Мелкий значок

	// Add "About..." menu item to system menu.
	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		CString strSetMenu; 
		strSetMenu.LoadString(IDS_SETTINGS);
		if (!strSetMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_SETTINGS, strSetMenu);
		}

		CString strAboutMenu;
		strAboutMenu.LoadString(IDS_ABOUTBOX);
		if (!strAboutMenu.IsEmpty())
		{
			//pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	return bResult;
}

///////////////////////////////////////////////////////////////////////////////////////
BOOL MainSheet::OnCommand(WPARAM wParam, LPARAM lParam)
{
	UINT nMessage = wParam & 0xFFF0;
	switch (nMessage)
	{
		case IDM_ABOUTBOX: {
			CAboutDlg dlgAbout;
			dlgAbout.DoModal();
		}
		break;
		case IDM_SETTINGS:
			m_Page_List->RunSettings();
			break;
	}
	return CPropertySheet::OnCommand(wParam, lParam);
}

///////////////////////////////////////////////////////////////////////////////////////
void MainSheet::OnContextMenu(CWnd* /*pWnd*/, CPoint point)
{
	RECT rcWin, rcBar;
	GetWindowRect(&rcWin);

	rcBar = rcWin;
	rcBar.bottom = rcWin.top + GetSystemMetrics(SM_CYCAPTION);

	if (PtInRect(&rcBar, point))
	{
		CMenu* pSysMenu = GetSystemMenu(FALSE);
		pSysMenu->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, point.x, point.y, this);

	}
}

///////////////////////////////////////////////////////////////////////////////////////
void MainSheet::ClearResults()
{
	for (int i = 0; i < NUMCHECKS; i++)
		m_Page_List->PrintResult(i, 2, -88);
}

//=====================================================================================
void MainSheet::PrintCheckResult(int nCurTest)
{
	m_Page_List->PrintResult(nCurTest, 2, g_zExeInfo[*g_pCurCheck].nResult);
	//m_Page_List->PrintResult(nCurTest, 2, 1);		//Если надо, чтобы все годны
}

//=====================================================================================
void MainSheet::SetCurTest(int nCurTest)
{
	m_Page_List->SetActiveTest(nCurTest);
}

LRESULT MainSheet::WindowProc(UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
		// Set
		case WM_SB_SET_CONTROL: // Получения структуры управления от класса RemoteLambdaZ
			theApp.SetControlMsg(lParam);
			return 0;
			break;

		// Get
		case WM_SB_CYCLE_GET_MSG:
		{
			theApp.GetReplyMsg();
			return 0;
			break;
		}
		break;
	}

	return CPropertySheet::WindowProc(message, wParam, lParam);
}
