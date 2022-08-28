#pragma once
#include "afxcmn.h"

// Максимальная длина номера изделия
#define   _MAX_NAME_  10
#define   _MAX_NUM_   20
#define   _MAX_DATA_  20

typedef  struct  tagArhivePageFiles
{
	TCHAR  sName[_MAX_NAME_];			// Тип изделия
	TCHAR  sNum[_MAX_NUM_];     // номер изделия
	TCHAR  sData[_MAX_DATA_];   // дата  в формате [год|мес|день|час|мин|сек: 20091224012334]
	TCHAR  sFile[MAX_PATH + 1]; // имя файла
} ARHIVEPAGE_FILES, *PARHIVEPAGE_FILES;

// диалоговое окно Page_Archive
/////////////////////////////////////////////////////////////////////////////
class Page_Archive : public CPropertyPage
{
	DECLARE_DYNAMIC(Page_Archive)

public:
	Page_Archive();
	virtual ~Page_Archive();

// Данные диалогового окна
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_PAGE_ARCHIVE };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // поддержка DDX/DDV

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnSetActive();

private:
	CFont m_cListFont;					// Объект шрифта ListCtrl
	class CReportCtrl* m_ListCtrl;

	PARHIVEPAGE_FILES    m_pFiles;      // найденные файлы
	int                  m_nCntFiles;   // количество найденых файлов

	int CreateListFont();								// Задание шрифта для ListCtrl
	void InitListCtrl();								// Задание начальных значений для ListCtrl

	// Инициализировать список протоколов
	void                 InitListProtocol();
	// Получить индекс в массиве найденных файлов
	int                  GetIdxFiles(int  iList);
	// Проверить номер изделия на соответствие маске
	BOOL                 bCheckNum(LPCTSTR  strNum, LPCTSTR strMask);
	// Проверить дату на соответствие маске
	int                  nCheckData(LPCTSTR  strData, LPCTSTR strMask);
	// Сортировать по дате
	void                 SortFiles();
	// Обменять данные в массиве
	void                 ChangeFiles(int i, int j, PARHIVEPAGE_FILES  pFiles);
	// Показать список протоколов
	void                 ShowProtocols();
	// Обновить список файлов
	void                 RefreshFiles();
	// Найти файлы с протоколами + логи
	void                 SearchFiles(LPCTSTR  strDir, int* pnCnt, PARHIVEPAGE_FILES  pArchFiles);
	// Проверить формат имени файла: "имя_изделия[_MAX_NAME_] - номер[_MAX_NUM_] - дата время[_MAX_DATA_]"
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
