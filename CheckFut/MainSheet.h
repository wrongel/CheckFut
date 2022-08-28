#pragma once

// ��������� ������
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
	// ��������� �� ��������
	class Page_GenInfo*  m_Page_GenInfo;
	class Page_Type*     m_Page_Type;
	class Page_Archive*  m_Page_Archive;
	class Page_ProdInfo* m_Page_ProdInfo;
	class Page_List*     m_Page_List;	

	void ClearPageMem(CPropertyPage* pPage);	// �������� ������, ���������� ��� ��������

	// ������������� �������, ���������� ����� ����� �������� ����, ����� ��� ������� ���������� ������ �������� ����
	static int CALLBACK MainSheetCallback(HWND hWndDlg, UINT uMsg, LPARAM lParam);	

public:
	// ������������� �������, ���������� ����� ��������� ����, ����� ��� ������� ����� ���������� �������� �������
	virtual void BuildPropPageArray();
	// ��-�� ��������������� Callback ������� (���� ������) ���������� ����������� ��� ������� Create (���������), �������� ���� ����� ���������� ������
	// m_psh.pfnCallback = MainSheetCallback;
	virtual BOOL Create(CWnd* pParentWnd = NULL, DWORD dwStyle = (DWORD)-1, DWORD dwExStyle = 0);	

	virtual BOOL OnInitDialog();
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	afx_msg void OnContextMenu(CWnd* /*pWnd*/, CPoint point);

	void ErrorMessage();			// ������ ��������� �� ������ ��� ������ ������� CCheckFutApp::RunProcess
	void ClearResults();			// ������� ���������� ��������
	void PrintCheckResult(int nCurTest);		// ������ �����������	
	void SetCurTest(int nCurTest);		// ������ �����������	
	virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
};


