// ResDlg.cpp : implementation file
//

#include "stdafx.h"
#include "resource.h"
#include "ResDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CResultDlg dialog

CResultDlg::CResultDlg(TCHAR *ptcsHead, TCHAR *ptcsText, int nColor, CWnd* pParent /*=NULL*/)
	: CDialog(CResultDlg::IDD, pParent)
{
	_stprintf_s(m_tcsTextHead,     _T("%s"), ptcsHead);
	_stprintf_s(m_tcsTextMain,     _T("%s"), ptcsText);
	_stprintf_s(m_tcsButtonOKName, _T("%s"), _T("���������"));
	m_nColor      = nColor;
	//{{AFX_DATA_INIT(CResultDlg)
	//}}AFX_DATA_INIT
}


void CResultDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CResultDlg)
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CResultDlg, CDialog)
	//{{AFX_MSG_MAP(CResultDlg)
	ON_WM_PAINT()
	ON_WM_TIMER()
	//}}AFX_MSG_MAP
    ON_WM_CTLCOLOR()
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CResultDlg message handlers

BOOL CResultDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	// TODO: Add extra initialization here
//----------------------------------------------------------------------------	
	Font_Create();	
	m_FontText.CreateFontIndirect(&m_zFontTextLog);
	m_FontButtonOK.CreateFontIndirect(&m_zFontButtonOKLog);
	m_FontResult.CreateFontIndirect(&m_zFontResultLog);
//---------------------------------------------------------------------------
	(GetDlgItem(IDOK))->SetFont(&m_FontButtonOK);
	SetDlgItemText(IDOK, m_tcsButtonOKName);
//---------------------------------------------------------------------------
	(GetDlgItem(IDC_ST_NAME))->SetFont(&m_FontText);
	(GetDlgItem(IDC_ST_NAME))->SetWindowText(m_tcsTextHead);
//---------------------------------------------------------------------------
    CWnd *pWnd = GetDlgItem(IDC_ST_RESULT);

    pWnd->SetFont( &m_FontResult);
	switch(m_nColor)
	{	case  0: m_clrResColor = RGB(250,  0,  0); break;
		case  1: m_clrResColor = RGB(  0,150, 50); break;
		default: //m_clrResColor = RGB(250,  0,  0); break;
				 m_clrResColor = RGB(0, 150, 50); break;
	}
    pWnd->SetWindowTextW(m_tcsTextMain);
//---------------------------------------------------------------------------
	uDelayTimer  = 5000;
	uTimerNumber = 101; // ����� ����� - � ��� �������
	SetTimer(uTimerNumber, uDelayTimer, NULL);
	nIsTimerEnable = 1;
//---------------------------------------------------------------------------	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}
/////////////////////////////////////////////////////////////////////////////
// CResultDlg message handlers
void CResultDlg::OnPaint() 
{
	CPaintDC dc(this); // device context for painting

	// Do not call CDialog::OnPaint() for painting messages
}
/////////////////////////////////////////////////////////////////////////////
// CResultDlg message handlers
void CResultDlg::OnOK() 
{
	// TODO: Add extra validation here
	if(nIsTimerEnable != NULL) KillTimer(uTimerNumber);	
	CDialog::OnOK();
}
/////////////////////////////////////////////////////////////////////////////
// CResultDlg message handlers
void CResultDlg::Font_Create()
{
	CRect rectWind;
	GetWindowRect(&rectWind);

	LOGFONT *pzLF;

	pzLF = &m_zFontTextLog;
	pzLF->lfHeight = rectWind.Height() / 12; /*25;*/		// ������ � ���.��������
	pzLF->lfWidth          = NULL;		// ������� ������ (0-����������)
	pzLF->lfEscapement     = NULL;		// ���� �������
	pzLF->lfOrientation    = NULL;		// ���� �������
	pzLF->lfWeight         = FW_BOLD;	// ������� ����� ����������
	pzLF->lfItalic         = FALSE;		// ������
	pzLF->lfUnderline      = FALSE;		// �������������
	pzLF->lfStrikeOut      = FALSE;		// ��������������
	pzLF->lfCharSet        = DEFAULT_CHARSET;		// ������� ���������
	pzLF->lfOutPrecision   = OUT_DEFAULT_PRECIS;	// �������� ������ ������
	pzLF->lfClipPrecision  = CLIP_DEFAULT_PRECIS;	// �������� ��������� ��������
	pzLF->lfQuality        = DEFAULT_QUALITY;		// �������� ������
	pzLF->lfPitchAndFamily = DEFAULT_PITCH;			// ��� � ��������� �������
	_tcscpy_s(pzLF->lfFaceName, _countof(pzLF->lfFaceName), _T("Arial"));		// ��� ������ (<= 32 �������)

	pzLF = &m_zFontButtonOKLog;
	pzLF->lfHeight         = rectWind.Height() / 12; /*25;*/		// ������ � ���.��������
	pzLF->lfWidth          = NULL;		// ������� ������ (0-����������)
	pzLF->lfEscapement     = NULL;		// ���� �������
	pzLF->lfOrientation    = NULL;		// ���� �������
	pzLF->lfWeight         = FW_HEAVY;	// ������� ����� ����������
	pzLF->lfItalic         = FALSE;		// ������
	pzLF->lfUnderline      = TRUE;		// �������������
	pzLF->lfStrikeOut      = FALSE;		// ��������������
	pzLF->lfCharSet        = DEFAULT_CHARSET;		// ������� ���������
	pzLF->lfOutPrecision   = OUT_DEFAULT_PRECIS;	// �������� ������ ������
	pzLF->lfClipPrecision  = CLIP_DEFAULT_PRECIS;	// �������� ��������� ��������
	pzLF->lfQuality        = DEFAULT_QUALITY;		// �������� ������
	pzLF->lfPitchAndFamily = DEFAULT_PITCH;			// ��� � ��������� �������
	_tcscpy_s(pzLF->lfFaceName, _countof(pzLF->lfFaceName), _T("Arial"));		// ��� ������ (<= 32 �������)

	pzLF = &m_zFontResultLog;
	pzLF->lfHeight         = rectWind.Height() / 7; /*25;*/	// ������ � ���.��������
	pzLF->lfWidth          = NULL;		// ������� ������ (0-����������)
	pzLF->lfEscapement     = NULL;		// ���� �������
	pzLF->lfOrientation    = NULL;		// ���� �������
	pzLF->lfWeight         = FW_HEAVY;	// ������� ����� ����������
	pzLF->lfItalic         = FALSE;		// ������
	pzLF->lfUnderline      = TRUE;		// �������������
	pzLF->lfStrikeOut      = FALSE;		// ��������������
	pzLF->lfCharSet        = DEFAULT_CHARSET;		// ������� ���������
	pzLF->lfOutPrecision   = OUT_DEFAULT_PRECIS;	// �������� ������ ������
	pzLF->lfClipPrecision  = CLIP_DEFAULT_PRECIS;	// �������� ��������� ��������
	pzLF->lfQuality        = DEFAULT_QUALITY;		// �������� ������
	pzLF->lfPitchAndFamily = DEFAULT_PITCH;			// ��� � ��������� �������
	_tcscpy_s(pzLF->lfFaceName, _countof(pzLF->lfFaceName), _T("Arial"));		// ��� ������ (<= 32 �������)
	return;
};
/////////////////////////////////////////////////////////////////////////////
// CResultDlg message handlers
void CResultDlg::OnTimer(UINT nIDEvent) 
{
	// TODO: Add your message handler code here and/or call default
	KillTimer(uTimerNumber);	
	nIsTimerEnable = NULL;
	CDialog::OnTimer(nIDEvent);
	OnOK();
}
/////////////////////////////////////////////////////////////////////////////
// CResultDlg message handlers
HBRUSH CResultDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
    HBRUSH hbr = CDialog::OnCtlColor(pDC, pWnd, nCtlColor);

    // TODO:  �������� ����� �������� DC
    if (pWnd->GetDlgCtrlID() == IDC_ST_RESULT)
          {  pDC->SetTextColor(m_clrResColor); }

    // TODO:  ������� ������ �������� ����������� �����, ���� ��� �� ���������� �� ���������
    return hbr;
}
/////////////////////////////////////////////////////////////////////////////
// CResultDlg message handlers
