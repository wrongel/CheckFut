// MesTemplate.cpp: файл реализации
//

#include "stdafx.h"
#include "CheckFut.h"
#include "MesTemplate.h"
#include "afxdialogex.h"


// диалоговое окно MesTemplate

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
	pz.lfHeight = rectWind.Height() / 9;			// Высота в лог.единицах
	pz.lfWidth = NULL;			// Средняя ширина (0-подходящий)
	pz.lfEscapement = NULL;	// Угол наклона
	pz.lfOrientation = NULL;	// Угол наклона
	pz.lfWeight = FW_HEAVY;	// Толщина линий начертания
	pz.lfItalic = FALSE;		// Курсив
	pz.lfUnderline = FALSE;	// Подчеркивание
	pz.lfStrikeOut = FALSE;	// Перечеркивание
	pz.lfCharSet = DEFAULT_CHARSET;			// Таблица кодировки
	pz.lfOutPrecision = OUT_DEFAULT_PRECIS;	// Механизм выбора шрифта
	pz.lfClipPrecision = CLIP_DEFAULT_PRECIS;	// Механизм отсечения символов
	pz.lfQuality = DEFAULT_QUALITY;			// Качество вывода
	pz.lfPitchAndFamily = DEFAULT_PITCH;		// Тип и семейство шрифтов
	_tcscpy_s(pz.lfFaceName, _countof(pz.lfFaceName), _T("Courier New"));		// Имя шрифта (<= 32 символа)
	m_MesFont.CreateFontIndirect(&pz);
}


BEGIN_MESSAGE_MAP(MesTemplate, CDialogEx)
END_MESSAGE_MAP()


// обработчики сообщений MesTemplate


BOOL MesTemplate::OnInitDialog()
{
	CDialogEx::OnInitDialog();
	
	FontCreate();
	GetDlgItem(IDC_MESSAGE_EDIT)->SetFont(&m_MesFont);
	GetDlgItem(IDC_MESSAGE_EDIT2)->SetFont(&m_MesFont);

	SetDlgItemText(IDC_MESSAGE_EDIT,  sString1);
	SetDlgItemText(IDC_MESSAGE_EDIT2, sString2);

	return TRUE;  // return TRUE unless you set the focus to a control
				  // Исключение: страница свойств OCX должна возвращать значение FALSE
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
