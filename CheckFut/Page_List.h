#pragma once
#include "afxcmn.h"

// ���������� ���� Page_List

class Page_List : public CPropertyPage
{
	DECLARE_DYNAMIC(Page_List)

public:
	Page_List();
	virtual ~Page_List();

// ������ ����������� ����
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_PAGE_LIST };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // ��������� DDX/DDV

	DECLARE_MESSAGE_MAP()
private:
	int m_nChecked;							// ���������� ���������� ���������
	int m_nBorder;							// ������� ������������ ��� ������� �����
	int m_nTests;							// ���������� ����������� ������, ������� ����������� ��������� ���
	int m_iTIndex;							// ������� ������ ��� ���������
	int m_nCheckEnable;						// ���� ����������� ���������
	int m_nRecFlag;							// ���� �������� ��� ������� ���������

	class CReportCtrl*   m_ListCtrl;		// ������ CReportCtrl

	void SetProgBorder();
	void InitListCtrl();					// ������� ��������� �������� ��� ListCtrl
	void PrintCheckNames();					// ������ �������� ��������
	
public:
	virtual BOOL OnInitDialog();
	virtual BOOL OnSetActive();
	afx_msg void OnItemchangedList(NMHDR *pNMHDR, LRESULT *pResult);			// ��������� ������� �� �������, ���������� ��� ��������� ������ "������ ��������"
	afx_msg void OnBnClickedButtonStart();
	afx_msg void OnBnClickedButtonStop();
	afx_msg void OnBnClickedButtonReport();

	//afx_msg LRESULT OnSetTimer(WPARAM a, LPARAM b);		// ��������� ������� (PostMessage)
	void EnableButtons(BOOL bFlag, BOOL bWhiteFlag);	// ���������, ����������� ������
	int RunSettings();									// ������ ���� � �����������
	//void ProgressCorrect(float nPos, int nTest, int nCountRun, int nMaxRun);		// �������� ������� ���������
	//void ProgressCorrect(float nPos, int nTest, int nCountRun);		// �������� ������� ���������
	void ProgressCorrect(float nPos);		// �������� ������� ���������
	void PrintResult(int nItem, int nSubItem, int nResult);		// ������ ����������� �������� ������ � ListCtrl
	void SetActiveTest(int nItem);			// ���������� �������� �����
	void IncTestIndex();
};
