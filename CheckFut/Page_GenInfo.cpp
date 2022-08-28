// Page_GenInfo.cpp: файл реализации
//

#include "stdafx.h"
#include "CheckFut.h"
#include "Page_GenInfo.h"
#include "afxdialogex.h"
#include "CheckFut.hxx"

/////////////////////////////////////////////////////////////////////////////
// диалоговое окно Page_GenInfo

IMPLEMENT_DYNAMIC(Page_GenInfo, CPropertyPage)

Page_GenInfo::Page_GenInfo()
	: CPropertyPage(IDD_PAGE_GENINFO)
{
	m_bRecFlag = 0;
	InitData();
}

Page_GenInfo::~Page_GenInfo()
{
}

void Page_GenInfo::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(Page_GenInfo, CPropertyPage)
	ON_CBN_SELCHANGE(IDC_COMBO_LOCATE, &Page_GenInfo::OnSelchangeComboLocate)
	ON_CBN_EDITCHANGE(IDC_COMBO_LOCATE, &Page_GenInfo::OnEditchangeComboLocate)
	ON_EN_CHANGE(IDC_EDIT_OPER, &Page_GenInfo::OnChangeEditOper)
	ON_EN_CHANGE(IDC_EDIT_OTK, &Page_GenInfo::OnChangeEditOtk)
	ON_EN_CHANGE(IDC_EDIT_VP, &Page_GenInfo::OnChangeEditVp)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// обработчики сообщений Page_GenInfo

BOOL Page_GenInfo::OnInitDialog()
{
	CPropertyPage::OnInitDialog();

	/*CFont gg; 
	gg.CreateFont(18, 0, 0, 0, FW_BOLD, FALSE, FALSE, 0,
		RUSSIAN_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
		DEFAULT_QUALITY, DEFAULT_PITCH | FF_ROMAN, _T("Courier New"));
	GetDlgItem(IDC_EDIT_VP)->SetFont(&gg);*/

	// Получение ширины и высоты шрифта
	CEdit* pEdit = (CEdit*)GetDlgItem(IDC_EDIT_OPER);
	CClientDC dc(pEdit);
	CFont* pFont = dc.SelectObject(pEdit->GetFont());
	m_zFontSize = dc.GetTextExtent(_T("A"));

	CComboBox* pComboLoc = (CComboBox*)GetDlgItem(IDC_COMBO_LOCATE);
	for (int i = 0; i < NUMLOCATE; i++)
		pComboLoc->InsertString(i, m_szLocate[i]);

	memcpy(&m_zInfo, &g_zGenInfo, sizeof(m_zInfo));
	if (g_zGenInfo.nPosition != -1)
	{
		_tcscpy_s(m_zInfo.lpszPosition, m_szLocate[0]);
		pComboLoc->SetCurSel(g_zGenInfo.nPosition);
	}

	Reflect_Source_Data(IDC_EDIT_OPER, m_zInfo.lpszOperator, _tcslen(m_zInfo.lpszOperator));
	Reflect_Source_Data(IDC_EDIT_OTK, m_zInfo.lpszOtkMan, _tcslen(m_zInfo.lpszOtkMan));
	Reflect_Source_Data(IDC_EDIT_VP, m_zInfo.lpszVpMan, _tcslen(m_zInfo.lpszVpMan));

	/*SetDlgItemText(IDC_EDIT_OPER, _T("aa"));
	SetDlgItemText(IDC_EDIT_OTK, _T("aa"));
	SetDlgItemText(IDC_EDIT_VP, _T("aa"));
	SetDlgItemText(IDC_COMBO_LOCATE, _T("aa"));*/

	return TRUE;  // return TRUE unless you set the focus to a control
				  // Исключение: страница свойств OCX должна возвращать значение FALSE
}

/////////////////////////////////////////////////////////////////////////////
BOOL Page_GenInfo::OnSetActive()
{
	Check_Input();

	return CPropertyPage::OnSetActive();
}

/////////////////////////////////////////////////////////////////////////////
void Page_GenInfo::InitData()
{
	_tcscpy_s(m_szLocate[0], _T("Завод-изготовитель"));
	_tcscpy_s(m_szLocate[1], _T("Техническая позиция флота"));

	m_zInfo.nPosition = -1;
	m_zInfo.lpszPosition[0] = '\0';
	m_zInfo.lpszOperator[0] = '\0';
	m_zInfo.lpszOtkMan[0] = '\0';
	m_zInfo.lpszVpMan[0] = '\0';
}

/////////////////////////////////////////////////////////////////////////////
int Page_GenInfo::Reflect_Source_Data(UINT nId, TCHAR * pData, int nLen)
{
	CWnd *pWnd = (CWnd*)GetDlgItem(nId);

	if (pWnd != NULL && nLen > 0)
	{
		//m_bRecFlag = 1;		Не знаю для чего добавил, ведь здесь не может быть рекурсии
		SetDlgItemText(nId, pData);
		//m_bRecFlag = 0;
	}
	return 1;
}

/////////////////////////////////////////////////////////////////////////////
void Page_GenInfo::OnSelchangeComboLocate()
{
	CComboBox* ComboLocate = (CComboBox*)GetDlgItem(IDC_COMBO_LOCATE);
	int nPos = ComboLocate->GetCurSel();

	m_zInfo.nPosition = nPos;
	_tcscpy_s(m_zInfo.lpszPosition, m_szLocate[nPos]);
	Check_Input();
}

//===========================================================================
void Page_GenInfo::OnEditchangeComboLocate()
{
	// При написании всегда выбирается 0 элемент. Можно придумать что-то получше, если потребуется
	CComboBox* ComboLocate = (CComboBox*)GetDlgItem(IDC_COMBO_LOCATE);
	ComboLocate->SetCurSel(0);

	m_zInfo.nPosition = 0;
	_tcscpy_s(m_zInfo.lpszPosition, m_szLocate[0]);
	Check_Input();
}

//===========================================================================
void Page_GenInfo::OnChangeEditOper()
{
	Check_Char(IDC_EDIT_OPER);
	GetDlgItemText(IDC_EDIT_OPER, m_zInfo.lpszOperator, sizeof(m_zInfo.lpszOperator));
	Check_Input();
}

//===========================================================================
void Page_GenInfo::OnChangeEditOtk()
{
	Check_Char(IDC_EDIT_OTK);
	GetDlgItemText(IDC_EDIT_OTK, m_zInfo.lpszOtkMan, sizeof(m_zInfo.lpszOtkMan));
	Check_Input();
}

//===========================================================================
void Page_GenInfo::OnChangeEditVp()
{
	Check_Char(IDC_EDIT_VP);
	GetDlgItemText(IDC_EDIT_VP, m_zInfo.lpszVpMan, sizeof(m_zInfo.lpszVpMan));
	Check_Input();
}

/////////////////////////////////////////////////////////////////////////////
int Page_GenInfo::Check_Char(UINT nId)
{
	if (m_bRecFlag == 1)	// Если рекурсия после SetDlgItemText
		return 0;

	TCHAR  cText[1024];
	int    nLen;
	int    nret = 0;
	int	   nPos = 0;		// Позиция недопустимого символа
	BOOL   bPosFlag = 0;	// 0 - недопустимый символ в конце строки, 1 - в середине

	CEdit* a = (CEdit*)GetDlgItem(nId);
	CPoint b = a->GetCaretPos();					// При рекурсии не обновляется, для этого введен m_bRecFlag

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

	if (cText[nPos] == '0')                 nret++;
	if (cText[nPos] == '1')                 nret++;
	if (cText[nPos] == '2')                 nret++;
	if (cText[nPos] == '3')                 nret++;
	if (cText[nPos] == '4')                 nret++;
	if (cText[nPos] == '5')                 nret++;
	if (cText[nPos] == '6')                 nret++;
	if (cText[nPos] == '7')                 nret++;
	if (cText[nPos] == '8')                 nret++;
	if (cText[nPos] == '9')                 nret++;

	if (cText[nPos] == '!')                 nret++;
	if (cText[nPos] == '?')                 nret++;
	if (cText[nPos] == '@')                 nret++;
	if (cText[nPos] == '#')                 nret++;
	if (cText[nPos] == '$')                 nret++;
	if (cText[nPos] == '%')                 nret++;
	if (cText[nPos] == '^')                 nret++;
	if (cText[nPos] == '&')                 nret++;
	if (cText[nPos] == '*')                 nret++;
	if (cText[nPos] == '(')                 nret++;
	if (cText[nPos] == ')')                 nret++;
	if (cText[nPos] == ':')                 nret++;
	if (cText[nPos] == ';')                 nret++;
	if (cText[nPos] == ',')                 nret++;
	//if (cText[nPos] == '.')                 nret++;
	if (cText[nPos] == '<')                 nret++;
	if (cText[nPos] == '>')                 nret++;
	if (cText[nPos] == '\\')                nret++;
	if (cText[nPos] == '/')                 nret++;
	if (cText[nPos] == '"')                 nret++;
	if (cText[nPos] == '=')                 nret++;

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

/////////////////////////////////////////////////////////////////////////////
int Page_GenInfo::Check_Input()
{
	CPropertySheet* pSheet;
	int   nErr = 0;

	if (_tcslen(m_zInfo.lpszOperator) < 2)
		nErr++;
	if (_tcslen(m_zInfo.lpszOtkMan)   < 2)
		nErr++;
	if (_tcslen(m_zInfo.lpszVpMan)    < 2)
		nErr++;
	if (m_zInfo.nPosition == -1)
		nErr++;

	pSheet = (CPropertySheet*)GetParent();
	if (nErr == 0) // Все поля введены		
		pSheet->SetWizardButtons(PSWIZB_NEXT);
	else
		pSheet->SetWizardButtons(0);

	return nErr;
}

/////////////////////////////////////////////////////////////////////////////
LRESULT Page_GenInfo::OnWizardNext()
{
	memcpy(&g_zGenInfo, &m_zInfo, sizeof(m_zInfo));
	return CPropertyPage::OnWizardNext();
}