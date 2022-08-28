#pragma once
class CreateReport
{
public:
	CreateReport();
	~CreateReport();

	int ReadPaths();		// �������� ���� � ������ ���������� ��������� ������
	int ReadFiles();		// ������� ���������� ��������� ���������� � ������ sReportPaths
	int CreatePdf(TCHAR* sReportPath);		// �������� ������ ���������

private:
	class HaruClass* m_pPdf;
	int m_nNumReports;			// ���������� ����������

	CString* m_sReportPaths;	// ������ ����� � ����������
	CString* m_sReportData;		// ������ ����������� ����������

	void CreateHeader();					// ������� �����
	void CreateFooter(CString* sFooter);	// ������� ������� ������
	void CreateFootString(char* sFootString, int nSize);				// ������� ������ ������� ������ ��������
	int StringSplit(const CString &str, CStringArray &arr, TCHAR chDelimitior);			// ���������� ������ �� ��������� � ������� CStringArray
};

