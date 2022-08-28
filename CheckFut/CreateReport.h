#pragma once
class CreateReport
{
public:
	CreateReport();
	~CreateReport();

	int ReadPaths();		// Получить пути к файлам протоколов выбранных тестов
	int ReadFiles();		// Считать содержимое текстовыъ протоколов в массив sReportPaths
	int CreatePdf(TCHAR* sReportPath);		// Создание общего протокола

private:
	class HaruClass* m_pPdf;
	int m_nNumReports;			// Количество протоколов

	CString* m_sReportPaths;	// Массив путей к протоколам
	CString* m_sReportData;		// Массив содержимого протоколов

	void CreateHeader();					// Создать шапку
	void CreateFooter(CString* sFooter);	// Создать подпись отчета
	void CreateFootString(char* sFootString, int nSize);				// Создать строку подписи каждой страницы
	int StringSplit(const CString &str, CStringArray &arr, TCHAR chDelimitior);			// Разделение строки на подстроки в массиве CStringArray
};

