#pragma once

#define NUMLOCATE 2
// диалоговое окно Page_GenInfo

struct tagGenPageInfo
{
	int   nPosition;
	TCHAR lpszPosition[256];
	TCHAR lpszStorage[256];				// Добавлено только для того, чтобы в next можно было использовать memcpy вместо кучи _tcscpy_s
	TCHAR lpszOperator[256];
	TCHAR lpszOtkMan[256];
	TCHAR lpszVpMan[256];
};

class Page_GenInfo : public CPropertyPage
{
	DECLARE_DYNAMIC(Page_GenInfo)

public:
	Page_GenInfo();
	virtual ~Page_GenInfo();

// Данные диалогового окна
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_PAGE_GENINFO };
#endif

private:
	struct tagGenPageInfo m_zInfo;			// Информация о контроле
	CSize m_zFontSize;						// Ширина и высота шрифта
	BOOL m_bRecFlag;						// Флаг рекурсии в EditChange
	TCHAR m_szLocate[NUMLOCATE][128];		// Названия мест проведения

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // поддержка DDX/DDV

	void InitData();											// Инициализация полей
	int Check_Char(UINT nId);									// Проверка вводимых символов, bFlagNum - 0 для текстового CEdit, 1 - для числового
	int Check_Input();											// Проверка всех данных для активации кнопки ОК
	int Reflect_Source_Data(UINT nId, TCHAR *pData, int nLen);	// Занесение данных в CEdit

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	virtual BOOL OnSetActive();

	afx_msg void OnSelchangeComboLocate();
	afx_msg void OnEditchangeComboLocate();
	afx_msg void OnChangeEditOper();
	afx_msg void OnChangeEditOtk();
	afx_msg void OnChangeEditVp();
	virtual LRESULT OnWizardNext();
};
