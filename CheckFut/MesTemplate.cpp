// MesTemplate.cpp: ���� ����������
//

#include "stdafx.h"
#include "CheckFut.h"
#include "MesTemplate.h"
#include "afxdialogex.h"


// ���������� ���� MesTemplate

IMPLEMENT_DYNAMIC(MesTemplate, CDialogEx)

MesTemplate::MesTemplate(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_MESSAGE, pParent)
{
}

MesTemplate::~MesTemplate()
{
}

void MesTemplate::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

void MesTemplate::FontCreate()
{
	LOGFONT pz;
	CRect rectWind;

	GetWindowRect(&rectWind);	
	pz.lfHeight = rectWind.Height() / 9;			// ������ � ���.��������
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


BEGIN_MESSAGE_MAP(MesTemplate, CDialogEx)
END_MESSAGE_MAP()


// ����������� ��������� MesTemplate


BOOL MesTemplate::OnInitDialog()
{
	CDialogEx::OnInitDialog();
	
	FontCreate();
	GetDlgItem(IDC_MESSAGE_EDIT)->SetFont(&m_MesFont);
	GetDlgItem(IDC_MESSAGE_EDIT2)->SetFont(&m_MesFont);

	SetDlgItemText(IDC_MESSAGE_EDIT,  sString1);
	SetDlgItemText(IDC_MESSAGE_EDIT2, sString2);

	return TRUE;  // return TRUE unless you set the focus to a control
				  // ����������: �������� ������� OCX ������ ���������� �������� FALSE
}


void MesTemplate::OnOK()
{
	CDialogEx::OnOK();
}

void MesTemplate::SetStrings(TCHAR * sStringTop, TCHAR * sStringBottom)
{
	_tcscpy_s(sString1, sStringTop);
	_tcscpy_s(sString2, sStringBottom);	
}
