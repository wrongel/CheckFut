// PowerDlg.cpp: ���� ����������
//

#include "stdafx.h"
#include "CheckFut.h"
#include "PowerDlg.h"
#include "afxdialogex.h"
#include "MesTemplate.h"

// ���������� ���� PowerDlg

IMPLEMENT_DYNAMIC(PowerDlg, CDialogEx)

PowerDlg::PowerDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_POWER, pParent)
{
	m_sI[0] = '\0';
	m_sU[0] = '\0';
}

PowerDlg::~PowerDlg()
{
}

void PowerDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(PowerDlg, CDialogEx)
	ON_BN_CLICKED(IDOK, &PowerDlg::OnBnClickedOk)
	ON_EN_CHANGE(IDC_EDIT_I, &PowerDlg::OnChangeEditI)
	ON_EN_CHANGE(IDC_EDIT_U, &PowerDlg::OnChangeEditU)
END_MESSAGE_MAP()


// ����������� ��������� PowerDlg


BOOL PowerDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();
	FontCreate();

	GetDlgItem(IDC_STATIC1)->SetFont(&m_MesFont);
	GetDlgItem(IDC_STATIC2)->SetFont(&m_MesFont);
	GetDlgItem(IDC_STATIC3)->SetFont(&m_MesFont);
	GetDlgItem(IDC_STATIC4)->SetFont(&m_MesFont);
	GetDlgItem(IDC_STATIC5)->SetFont(&m_MesFont);
	GetDlgItem(IDC_EDIT_I)->SetFont(&m_MesFont);
	GetDlgItem(IDC_EDIT_U)->SetFont(&m_MesFont);
	GetDlgItem(IDOK)->SetFont(&m_MesFont);

	CClientDC dc(GetDlgItem(IDC_EDIT_I));
	CFont* pFont = dc.SelectObject(&m_MesFont);
	m_zFontSize = dc.GetTextExtent(_T("A"));

	return TRUE;  // return TRUE unless you set the focus to a control
				  // ����������: �������� ������� OCX ������ ���������� �������� FALSE
}

void PowerDlg::OnBnClickedOk()
{
	MesTemplate MessageDlg;

	if (Check_Measures() == 0)
	{
		MessageDlg.SetStrings(_T("���������� ���������� �����������!"), _T("��������� �������� �������!"));
		MessageDlg.DoModal();
		SendButtonMessage(IDCANCEL);
	}
	else
		CDialogEx::OnOK();
}

void PowerDlg::FontCreate()
{
	LOGFONT pz;
	CRect rectWind;

	GetWindowRect(&rectWind);
	pz.lfHeight = rectWind.Height() / 15;			// ������ � ���.��������
	pz.lfWidth = NULL;			// ������� ������ (0-����������)
	pz.lfEscapement = NULL;	// ���� �������
	pz.lfOrientation = NULL;	// ���� �������
	pz.lfWeight = FW_HEAVY;	// ������� ����� ����������
	pz.lfItalic = FALSE;		// ������
	pz.lfUnderline = FALSE;	// �������������
	pz.lfStrikeOut = FALSE;	// ��������������
	pz.lfCharSet = DEFAULT_CHARSET;			// ������� ���������
	pz.lfOutPrecision = OUT_DEFAULT_PRECIS;	// �������� ������ ������
	pz.lfClipPrecision = CLIP_DEFAULT_PRECIS;	// �������� ��������� ��������
	pz.lfQuality = DEFAULT_QUALITY;			// �������� ������
	pz.lfPitchAndFamily = DEFAULT_PITCH;		// ��� � ��������� �������
	_tcscpy_s(pz.lfFaceName, _countof(pz.lfFaceName), _T("Courier New"));		// ��� ������ (<= 32 �������)
	m_MesFont.CreateFontIndirect(&pz);
}

int PowerDlg::Check_Number(UINT nId)
{
	if (m_bRecFlag == 1)	// ���� �������� ����� SetDlgItemText
		return 0;

	TCHAR  cText[1024];
	int    nLen;
	int    nret = 0;
	int	   nPos = 0;		// ������� ������������� �������
	BOOL   bPosFlag = 0;	// 0 - ������������ ������ � ����� ������, 1 - � ��������

	CEdit* a = (CEdit*)GetDlgItem(nId);
	CPoint b = a->GetCaretPos();					// ��� �������� �� �����������, ��� ����� ������ m_bRecFlag

	GetDlgItemText(nId, cText, sizeof(cText));
	nLen = _tcslen(cText);

	if (nLen < 1)
		return 0;

	nPos = b.x / m_zFontSize.cx;
	if (nPos == nLen)
	{
		nPos = nLen - 1;
		bPosFlag = 0;
	}
	else
	{
		nPos -= 1;
		bPosFlag = 1;
	}

	if (_istdigit(cText[nPos]) == NULL)
	{
		if ((cText[nPos] != '.') && (cText[nPos] != '-'))
			nret++;
	}

	if (nret != 0)
	{
		if (bPosFlag == 1)
			_tcsmove(&cText[nPos], &cText[nPos + 1], _tcslen(cText) - nPos);
		else
			cText[nPos] = '\0';
		m_bRecFlag = 1;
		SetDlgItemText(nId, cText);
		m_bRecFlag = 0;
		a->SetFocus();
		a->SetSel(-1);
	}

	return 1;
}

int PowerDlg::Check_Input()
{
	GetDlgItem(IDOK)->EnableWindow(0);
	if (_tcslen(m_sI) > 0 && _tcslen(m_sU) > 0)
		GetDlgItem(IDOK)->EnableWindow(1);
	
	return 1;
}

int PowerDlg::Check_Measures()
{
	double fI, fU;
	int    nResU, nResI, nRes;

	if (_tcslen(m_sI) > 0 && _tcslen(m_sU) > 0)
	{
		fI = _ttof(m_sI);
		fU = _ttof(m_sU);

		if (fU >= 22 && fU <= 33.6)	   nResU = 1;		// -10%, +15%, �������� ��
		else						   nResU = 0;
		if (fI >= 0.3 && fI <= 6)	   nResI = 1;
		else						   nResI = 0;

		if (nResU*nResI != 0)	nRes = 1;
		else					nRes = 0;
	}

	return nRes;
}

void PowerDlg::SendButtonMessage(UINT nID)
{
	DWORD dwWParam, dwLParam;

	dwWParam = (BN_CLICKED << 16) | (nID & 0xFFFF);
	dwLParam = (DWORD)::GetDlgItem(m_hWnd, nID);
	SendMessage(WM_COMMAND, dwWParam, dwLParam);
	return;
}


void PowerDlg::OnChangeEditI()
{
	Check_Number(IDC_EDIT_I);
	GetDlgItemText(IDC_EDIT_I, m_sI, sizeof(m_sI));
	Check_Input();
}


void PowerDlg::OnChangeEditU()
{
	Check_Number(IDC_EDIT_U);
	GetDlgItemText(IDC_EDIT_U, m_sU, sizeof(m_sU));
	Check_Input();
}
