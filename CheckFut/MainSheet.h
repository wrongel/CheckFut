#pragma once

// Параметры шрифта
static TCHAR m_sFontName[64] = _T("Courier New");
static WORD  m_wFontSize = 16;

// MainSheet

class MainSheet : public CPropertySheet
{
	DECLARE_DYNAMIC(MainSheet)

public:
	MainSheet(UINT nIDCaption, CWnd* pParentWnd = NULL, UINT iSelectPage = 0);
	MainSheet(LPCTSTR pszCaption, CWnd* pParentWnd = NULL, UINT iSelectPage = 0);
	virtual ~MainSheet();

protected:
	DECLARE_MESSAGE_MAP()

private:
	HICON m_hIcon;
	// Указатели на страницы
	class Page_GenInfo*  m_Page_GenInfo;
	class Page_Type*     m_Page_Type;
	class Page_Archive*  m_Page_Archive;
	class Page_ProdInfo* m_Page_ProdInfo;
	class Page_List*     m_Page_List;	

	void ClearPageMem(CPropertyPage* pPage);	// Очистить память, выделенную под страницы

	// Перегруженная функция, вызывается сразу после создания окна, нужна для задания параметров шрифта главного окна
	static int CALLBACK MainSheetCallback(HWND hWndDlg, UINT uMsg, LPARAM lParam);	

public:
	// Перегруженная функция, вызывается перед созданием окна, нужна для задания общих параметров дочерних страниц
	virtual void BuildPropPageArray();
	// Из-за переопределения Callback функции (одна строка) приходится перегружать всю функцию Create (копипаста), возможно есть более адекватный способ
	// m_psh.pfnCallback = MainSheetCallback;
	virtual BOOL Create(CWnd* pParentWnd = NULL, DWORD dwStyle = (DWORD)-1, DWORD dwExStyle = 0);	

	virtual BOOL OnInitDialog();
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	afx_msg void OnContextMenu(CWnd* /*pWnd*/, CPoint point);

	void ErrorMessage();			// Выдача сообщения об ошибке при работе функции CCheckFutApp::RunProcess
	void ClearResults();			// Очистка резльтатов контроля
	void PrintCheckResult(int nCurTest);		// Печать результатов	
	void SetCurTest(int nCurTest);		// Печать результатов	
	virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
};


