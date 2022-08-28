#pragma once
#include "afxcmn.h"

// диалоговое окно Page_List

class Page_List : public CPropertyPage
{
	DECLARE_DYNAMIC(Page_List)

public:
	Page_List();
	virtual ~Page_List();

// Данные диалогового окна
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_PAGE_LIST };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // поддержка DDX/DDV

	DECLARE_MESSAGE_MAP()
private:
	int m_nChecked;							// Количество отмеченных чекбоксов
	int m_nBorder;							// Граница прогрессбара для каждого теста
	int m_nTests;							// Количество запускаемых тестов, включая запускаемые несколько раз
	int m_iTIndex;							// Счетчик тестов для прогресса
	int m_nCheckEnable;						// Флаг доступности чекбоксов
	int m_nRecFlag;							// Флаг рекурсии при нажатии чекбоксов

	class CReportCtrl*   m_ListCtrl;		// Объект CReportCtrl

	void SetProgBorder();
	void InitListCtrl();					// Задание начальных значений для ListCtrl
	void PrintCheckNames();					// Печать названия проверок
	
public:
	virtual BOOL OnInitDialog();
	virtual BOOL OnSetActive();
	afx_msg void OnItemchangedList(NMHDR *pNMHDR, LRESULT *pResult);			// Обработка нажатия на чекбокс, необходима для активации кнопки "Начать контроль"
	afx_msg void OnBnClickedButtonStart();
	afx_msg void OnBnClickedButtonStop();
	afx_msg void OnBnClickedButtonReport();

	//afx_msg LRESULT OnSetTimer(WPARAM a, LPARAM b);		// Отрисовка таймера (PostMessage)
	void EnableButtons(BOOL bFlag, BOOL bWhiteFlag);	// Активация, деактивация кнопок
	int RunSettings();									// Запуск окна с настройками
	//void ProgressCorrect(float nPos, int nTest, int nCountRun, int nMaxRun);		// Изменить позицию прогресса
	//void ProgressCorrect(float nPos, int nTest, int nCountRun);		// Изменить позицию прогресса
	void ProgressCorrect(float nPos);		// Изменить позицию прогресса
	void PrintResult(int nItem, int nSubItem, int nResult);		// Печать результатов контроля тестов в ListCtrl
	void SetActiveTest(int nItem);			// Подстветка текущего теста
	void IncTestIndex();
};
