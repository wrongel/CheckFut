#pragma once
#include "afxcmn.h"

// ������������ ����� ������ �������
#define   _MAX_NAME_  10
#define   _MAX_NUM_   20
#define   _MAX_DATA_  20

typedef  struct  tagArhivePageFiles
{
	TCHAR  sName[_MAX_NAME_];			// ��� �������
	TCHAR  sNum[_MAX_NUM_];     // ����� �������
	TCHAR  sData[_MAX_DATA_];   // ����  � ������� [���|���|����|���|���|���: 20091224012334]
	TCHAR  sFile[MAX_PATH + 1]; // ��� �����
} ARHIVEPAGE_FILES, *PARHIVEPAGE_FILES;

// ���������� ���� Page_Archive
/////////////////////////////////////////////////////////////////////////////
class Page_Archive : public CPropertyPage
{
	DECLARE_DYNAMIC(Page_Archive)

public:
	Page_Archive();
	virtual ~Page_Archive();

// ������ ����������� ����
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_PAGE_ARCHIVE };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // ��������� DDX/DDV

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnSetActive();

private:
	CFont m_cListFont;					// ������ ������ ListCtrl
	class CReportCtrl* m_ListCtrl;

	PARHIVEPAGE_FILES    m_pFiles;      // ��������� �����
	int                  m_nCntFiles;   // ���������� �������� ������

	int CreateListFont();								// ������� ������ ��� ListCtrl
	void InitListCtrl();								// ������� ��������� �������� ��� ListCtrl

	// ���������������� ������ ����������
	void                 InitListProtocol();
	// �������� ������ � ������� ��������� ������
	int                  GetIdxFiles(int  iList);
	// ��������� ����� ������� �� ������������ �����
	BOOL                 bCheckNum(LPCTSTR  strNum, LPCTSTR strMask);
	// ��������� ���� �� ������������ �����
	int                  nCheckData(LPCTSTR  strData, LPCTSTR strMask);
	// ����������� �� ����
	void                 SortFiles();
	// �������� ������ � �������
	void                 ChangeFiles(int i, int j, PARHIVEPAGE_FILES  pFiles);
	// �������� ������ ����������
	void                 ShowProtocols();
	// �������� ������ ������
	void                 RefreshFiles();
	// ����� ����� � ����������� + ����
	void                 SearchFiles(LPCTSTR  strDir, int* pnCnt, PARHIVEPAGE_FILES  pArchFiles);
	// ��������� ������ ����� �����: "���_�������[_MAX_NAME_] - �����[_MAX_NUM_] - ���� �����[_MAX_DATA_]"
	BOOL                 CheckFile(TCHAR* sFileName);
public:
	virtual BOOL OnInitDialog();
	afx_msg void OnChangeEditNumIzd();
	afx_msg void OnChangeEditDataFrom();
	afx_msg void OnChangeEditDataUntil();
	afx_msg void OnClickedBtDataFrom();
	afx_msg void OnClickedBtDataUntil();
	afx_msg void OnBnClickedBtShow();
	afx_msg void OnDblclkListProtocol(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnChangeEditTypeIzd();
};
