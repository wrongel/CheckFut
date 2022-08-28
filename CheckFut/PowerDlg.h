#pragma once


// диалоговое окно PowerDlg

class PowerDlg : public CDialogEx
{
	DECLARE_DYNAMIC(PowerDlg)

public:
	PowerDlg(CWnd* pParent = NULL);   // стандартный конструктор
	virtual ~PowerDlg();

// Данные диалогового окна
	enum { IDD = IDD_POWER };

protected:
	CFont m_MesFont;
	BOOL  m_bRecFlag;						// Флаг рекурсии в EditChange
	CSize m_zFontSize;						// Ширина одного символа в TextEdit
	TCHAR m_sI[32];
	TCHAR m_sU[32];

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // поддержка DDX/DDV
	void FontCreate();

	int Check_Number(UINT nId);							// Проверка вводимых символов
	int Check_Input();									// Проверка всех данных для активации кнопки ОК
	int Check_Measures();

	void   SendButtonMessage(UINT nID);

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedOk();
	afx_msg void OnChangeEditI();
	afx_msg void OnChangeEditU();
};
