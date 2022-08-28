#pragma once


// диалоговое окно Page_ProdInfo

struct tagProdPageInfo
{
	DWORD nProduct;
	WORD  nStorage;					// 0 - без контейнера, 1 - в контейнере
	TCHAR tcsProdNum[256];
};

class Page_ProdInfo : public CPropertyPage
{
	DECLARE_DYNAMIC(Page_ProdInfo)

public:
	Page_ProdInfo();
	virtual ~Page_ProdInfo();

// Данные диалогового окна
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_PAGE_PRODINFO };
#endif

private:
	struct tagProdPageInfo m_zInfo;			// Информация о контроле
	TCHAR m_tcsStorage[2][100];
	CSize m_zFontSize;
	BOOL m_bRecFlag;						// Флаг рекурсии в EditChange

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // поддержка DDX/DDV

	void InitData();											// Инициализация полей
	int Check_Number(UINT nId);									// Проверка вводимых символов
	int Check_Input();											// Проверка всех данных для активации кнопки ОК
	int Reflect_Source_Data(UINT nId, TCHAR *pData, int nLen);	// Занесение данных в CEdit

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	virtual BOOL OnSetActive();
	afx_msg void OnSelchangeComboProduct();
	afx_msg void OnEditchangeComboProduct();
	afx_msg void OnChangeEditNumber();
	virtual LRESULT OnWizardNext();
	virtual LRESULT OnWizardBack();
	afx_msg void OnCbnSelchangeComboStorage();
	afx_msg void OnEditchangeComboStorage();
};
