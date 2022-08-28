#if !defined(AFX_RESDLG_H__9FDE0D67_5EAA_11D7_9D2C_444553540000__INCLUDED_)
#define AFX_RESDLG_H__9FDE0D67_5EAA_11D7_9D2C_444553540000__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ResDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CResultDlg dialog

class CResultDlg : public CDialog
{
// Construction
public:
	CResultDlg(TCHAR *ptcsHead, TCHAR *ptcsText, int nColor, CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CResultDlg)
	enum { IDD = IDD_RESULT_DLG };
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CResultDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	int       m_nColor;
	void      Font_Create (void);
	LOGFONT   m_zFontButtonOKLog;
	LOGFONT   m_zFontTextLog;
	LOGFONT   m_zFontResultLog;
	CFont     m_FontButtonOK;
	CFont     m_FontText;
	CFont     m_FontResult;
	TCHAR     m_tcsButtonOKName[256];
	TCHAR     m_tcsTextHead[256];
	TCHAR     m_tcsTextMain[256];
    COLORREF  m_clrResColor;

protected:
	int       nIsTimerEnable;
	UINT      uTimerNumber;
	UINT      uDelayTimer;
	// Generated message map functions
	//{{AFX_MSG(CResultDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	virtual void OnOK();
	afx_msg void OnTimer(UINT nIDEvent);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
public:
    afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_RESDLG_H__9FDE0D67_5EAA_11D7_9D2C_444553540000__INCLUDED_)
