// Page_ProdInfo.cpp: файл реализации
//

#include "stdafx.h"
#include "CheckFut.h"
#include "Page_ProdInfo.h"
#include "afxdialogex.h"
#include "CheckFut.hxx"

///////////////////////////////////////////////////////////////////////////////////////
// диалоговое окно Page_ProdInfo

IMPLEMENT_DYNAMIC(Page_ProdInfo, CPropertyPage)

Page_ProdInfo::Page_ProdInfo()
	: CPropertyPage(IDD_PAGE_PRODINFO)
{
	m_bRecFlag = 0;
	_tcscpy_s(m_tcsStorage[0], _T("Без контейнера"));
	_tcscpy_s(m_tcsStorage[1], _T("В контейнере"));
	InitData();
}

Page_ProdInfo::~Page_ProdInfo()
{
}

void Page_ProdInfo::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(Page_ProdInfo, CPropertyPage)
	ON_CBN_SELCHANGE(IDC_COMBO_PRODUCT, &Page_ProdInfo::OnSelchangeComboProduct)
	ON_CBN_EDITCHANGE(IDC_COMBO_PRODUCT, &Page_ProdInfo::OnEditchangeComboProduct)
	ON_CBN_SELCHANGE(IDC_COMBO_STORAGE, &Page_ProdInfo::OnCbnSelchangeComboStorage)
	ON_EN_CHANGE(IDC_EDIT_NUMBER, &Page_ProdInfo::OnChangeEditNumber)
	ON_CBN_EDITCHANGE(IDC_COMBO_STORAGE, &Page_ProdInfo::OnEditchangeComboStorage)
END_MESSAGE_MAP()

///////////////////////////////////////////////////////////////////////////////////////
// обработчики сообщений Page_ProdInfo

BOOL Page_ProdInfo::OnInitDialog()
{
	CPropertyPage::OnInitDialog();

	// Получение ширины и высоты шрифта
	CEdit* pEdit = (CEdit*)GetDlgItem(IDC_EDIT_NUMBER);
	CClientDC dc(pEdit);
	CFont* pFont = dc.SelectObject(pEdit->GetFont());
	m_zFontSize = dc.GetTextExtent(_T("A"));

	CComboBox* pComboProd = (CComboBox*)GetDlgItem(IDC_COMBO_PRODUCT);
	for (int i = 0; i < _countof(g_zProdTypes); i++)
		pComboProd->InsertString(i, g_zProdTypes[i].szProdTypes);

	CComboBox* pComboStore = (CComboBox*)GetDlgItem(IDC_COMBO_STORAGE);
	for (int i = 0; i < _countof(m_tcsStorage); i++)
		pComboStore->InsertString(i, m_tcsStorage[i]);

	// Начальные значения
	m_zInfo.nProduct = g_zGenInfo.nProduct;
	_tcscpy_s(m_zInfo.tcsProdNum, g_zGenInfo.lpszProdNum);

	if (g_zGenInfo.nProduct != -1)
		pComboProd->SetCurSel(g_zGenInfo.nProduct);

	pComboStore->SetCurSel(0);

	Reflect_Source_Data(IDC_EDIT_NUMBER, m_zInfo.tcsProdNum, _tcslen(m_zInfo.tcsProdNum));

	return TRUE;  // return TRUE unless you set the focus to a control
				  // Исключение: страница свойств OCX должна возвращать значение FALSE
}

///////////////////////////////////////////////////////////////////////////////////////
BOOL Page_ProdInfo::OnSetActive()
{
	Check_Input();

	return CPropertyPage::OnSetActive();
}

///////////////////////////////////////////////////////////////////////////////////////
void Page_ProdInfo::InitData()
{
	m_zInfo.nProduct = -1;
	m_zInfo.nStorage = 0;
	m_zInfo.tcsProdNum[0] = '\0';
}

///////////////////////////////////////////////////////////////////////////////////////
int Page_ProdInfo::Check_Number(UINT nId)
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

///////////////////////////////////////////////////////////////////////////////////////
int Page_ProdInfo::Check_Input()
{
	CPropertySheet* pSheet;
	int nErr = 0;

	GetDlgItem(IDC_COMBO_STORAGE)->EnableWindow(!m_zInfo.nProduct);

	if (_tcslen(m_zInfo.tcsProdNum) < 2)
		nErr++;
	if (m_zInfo.nProduct == -1)
		nErr++;

	// Какие-то различия между Заводом и ТПФ
	//switch (m_zInfo.nPosition)
	//{
	//case  0:  // Завод
	//	if (_tcslen(m_zInfo.lpszOtkMan)    < 2) 
	//		nErr++;
	//	break;
	//case  1:  // ТПФ
	//	if (_tcslen(m_zInfo.lpszVpMan) < 2) 
	//		nErr++;
	//	break;
	//default: 
	//	nErr++;
	//	break;
	//}

	pSheet = (CPropertySheet*)GetParent();
	if (nErr == 0) // Все поля введены		
		pSheet->SetWizardButtons(PSWIZB_BACK | PSWIZB_NEXT);
	else
		pSheet->SetWizardButtons(PSWIZB_BACK);

	return nErr;
}

///////////////////////////////////////////////////////////////////////////////////////
int Page_ProdInfo::Reflect_Source_Data(UINT nId, TCHAR * pData, int nLen)
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

///////////////////////////////////////////////////////////////////////////////////////
void Page_ProdInfo::OnSelchangeComboProduct()
{
	CComboBox* ComboProduct = (CComboBox*)GetDlgItem(IDC_COMBO_PRODUCT);

	m_zInfo.nProduct = ComboProduct->GetCurSel();
	Check_Input();
}

///////////////////////////////////////////////////////////////////////////////////////
void Page_ProdInfo::OnEditchangeComboProduct()
{
	CComboBox* ComboProduct = (CComboBox*)GetDlgItem(IDC_COMBO_PRODUCT);
	ComboProduct->SetCurSel(0);

	m_zInfo.nProduct = 0;
	Check_Input();
}

///////////////////////////////////////////////////////////////////////////////////////
void Page_ProdInfo::OnCbnSelchangeComboStorage()
{
	CComboBox* pComboStore = (CComboBox*)GetDlgItem(IDC_COMBO_STORAGE);

	m_zInfo.nStorage = pComboStore->GetCurSel();
	Check_Input();
}

///////////////////////////////////////////////////////////////////////////////////////
void Page_ProdInfo::OnEditchangeComboStorage()
{
	CComboBox* pComboStore = (CComboBox*)GetDlgItem(IDC_COMBO_STORAGE);
	pComboStore->SetCurSel(0);

	m_zInfo.nStorage = 0;
	Check_Input();
}

///////////////////////////////////////////////////////////////////////////////////////
void Page_ProdInfo::OnChangeEditNumber()
{
	Check_Number(IDC_EDIT_NUMBER);
	GetDlgItemText(IDC_EDIT_NUMBER, m_zInfo.tcsProdNum, sizeof(m_zInfo.tcsProdNum));
	Check_Input();
}

///////////////////////////////////////////////////////////////////////////////////////
LRESULT Page_ProdInfo::OnWizardNext()
{
	// Если не МА, то сбросить тип хранения
	if (m_zInfo.nProduct != 0)
	{
		m_zInfo.nStorage = 0;
		_tcscpy_s(g_zGenInfo.lpszStorage, _T("---"));
	}
	else
		_tcscpy_s(g_zGenInfo.lpszStorage, m_tcsStorage[m_zInfo.nStorage]);

	g_zGenInfo.nProduct = m_zInfo.nProduct;
	g_nMode = 1 << m_zInfo.nProduct;			// Выставлени режима в зависимости от изделия
	g_nMode |= (m_zInfo.nStorage << 3);			// Выставление режима в зависимости от типа хранения, 3 бит

	_tcscpy_s(g_zGenInfo.lpszProdNum, m_zInfo.tcsProdNum);

	return CPropertyPage::OnWizardNext();
}

///////////////////////////////////////////////////////////////////////////////////////
LRESULT Page_ProdInfo::OnWizardBack()
{
	CPropertySheet* pSheet = (CPropertySheet*)GetParent();
	pSheet->SetActivePage(2);

	return CPropertyPage::OnWizardBack();
}