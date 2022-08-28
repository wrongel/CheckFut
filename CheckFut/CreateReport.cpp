#include "stdafx.h"
#include "CreateReport.h"
#include "CheckFut.hxx"
#include "HaruClass.h"
#include "MainSheet.h"

/////////////////////////////////////////////////////////////////////////////
CreateReport::CreateReport()
{
	m_pPdf = new HaruClass();
	m_nNumReports  = 0;
	m_sReportPaths = new CString[NUMCHECKS + 2];		// NUMCHECKS + 3 ������� ����� �������� �������� - 1. ������� ����� ������� �������
	m_sReportData = NULL;
}

//============================================================================
CreateReport::~CreateReport()
{
	if (m_pPdf != NULL)
	{
		delete m_pPdf;
		m_pPdf = NULL;
	}
	if (m_sReportPaths != NULL)
	{
		delete[] m_sReportPaths;
		m_sReportPaths = NULL;
	}
	if (m_sReportData != NULL)
	{
		delete[] m_sReportData;
		m_sReportData = NULL;
	}
}

/////////////////////////////////////////////////////////////////////////////
int CreateReport::ReadPaths()
{
	int   iIndex = 0;
	int   iCountRun;
	TCHAR sRepFolder[1024];
	TCHAR tcsRepName[64];
	TCHAR sErrorMessage[256];
	CFileFind Find;

	// ������ ����� ������� � ���� ����� � ����������, ��. ������� ������

	for (int i = 0; i < g_nNumTests/*NUMCHECKS*/; i++)
	{
		iIndex = g_nSetChecks[g_nTestIndex[i]/*i*/];
		if (iIndex == -1)
			continue;

		iCountRun = g_zExeInfo[iIndex].iCountRun;
		int nPos = _tcschr(g_zExeInfo[iIndex].lpszRepName, '.') - g_zExeInfo[iIndex].lpszRepName;
		while (iCountRun > 0)			// ��������� ��� ������ ����������� ��������� ���
		{
			TCHAR* sCmdPar = NULL;
			_tcscpy_s(tcsRepName, g_zExeInfo[iIndex].lpszRepName);

			// ���������� ������� ���� �� ��� ����� � �����
			_tcscpy_s(sRepFolder, g_zExeInfo[iIndex].lpszPath);
			sRepFolder[_tcsrchr(sRepFolder, '\\') - sRepFolder + 1] = '\0';

			if (g_zExeInfo[iIndex].nPostAction == 2)		// ������� ���� ��
			{
				// ���������� ��������� /pres �� ��� ������
				sCmdPar = _tcsstr(g_zExeInfo[iIndex].lpszCmdPar[iCountRun - 1], _T("/pres"));
				if (sCmdPar != NULL)
				{
					sCmdPar = &sCmdPar[5];
					tcsRepName[nPos] = '\0';
					_stprintf_s(tcsRepName, _T("%s%s%s"), tcsRepName, sCmdPar, _T(".Rep"));
				}
			}

			if (_tcscmp(g_zExeInfo[iIndex].lpszRepName, _T("RsChannelTest.Rep")) == 0)		// Rs ����
			{
				// ���������� ��������� /type �� ��� ������
				sCmdPar = _tcsstr(g_zExeInfo[iIndex].lpszCmdPar[iCountRun - 1], _T("/type"));
				if (sCmdPar != NULL)
				{
					sCmdPar = &sCmdPar[5];
					tcsRepName[nPos] = '\0';
					_stprintf_s(tcsRepName, _T("%s%s%s"), tcsRepName, sCmdPar, _T(".Rep"));
				}
			}

			if (_tcscmp(g_zExeInfo[iIndex].lpszRepName, _T("MkioIsaTest.Rep")) == 0)		// TAM ����
			{
				// ���������� ��������� /TAM �� ��� ������
				sCmdPar = _tcsstr(g_zExeInfo[iIndex].lpszCmdPar[iCountRun - 1], _T("/TAM"));
				if (sCmdPar != NULL)
				{
					sCmdPar = &sCmdPar[4];
					tcsRepName[nPos] = '\0';
					_stprintf_s(tcsRepName, _T("%s%s%s"), tcsRepName, sCmdPar, _T(".Rep"));
				}
			}
			
			_tcscat_s(sRepFolder, tcsRepName);

			int nRet = Find.FindFile(sRepFolder);
			// �������� ������� ������ �����, ����������� ����������� ���������� �� g_nSetChecks, ������ ��� ��� �������� �����
			FILETIME zFileTime, zLocFileTime, zBegTime;

			if (nRet != 0)
			{
				Find.FindNextFile();
				Find.GetLastWriteTime(&zFileTime);
				FileTimeToLocalFileTime(&zFileTime, &zLocFileTime);
				SystemTimeToFileTime(&g_zTime, &zBegTime);
			}

			// ��� �����
			if (nRet == 0 || CompareFileTime(&zLocFileTime, &zBegTime) < 0)		// ���� ���� �� ������ ��� ���� ��������� ����� ������ ���� ������ �������� (������ ��������)
			{
				_stprintf_s(sErrorMessage, _T("�� ������ ���� ��������� %s ����� %s ��� ��������� � ����� ��������!"), tcsRepName, g_zExeInfo[iIndex].lpszCheckName);
				MessageBox(NULL, sErrorMessage, _T("������"), MB_OK | MB_ICONERROR);
			}
			else
			{
				m_sReportPaths[m_nNumReports] = sRepFolder;
				m_nNumReports++;
			}

			iCountRun--;
		}
	}

	return 1;
}

/////////////////////////////////////////////////////////////////////////////
int CreateReport::ReadFiles()
{
	CStdioFile RepFile;
	CString sLine;
	TCHAR sErrorMessage[256];
	m_sReportData = new CString[m_nNumReports];

	for (int i = 0; i < m_nNumReports; i++)
	{
		if (RepFile.Open(m_sReportPaths[i], CFile::modeRead | CFile::typeText) == 0)
		{
			_stprintf_s(sErrorMessage, _T("������ �������� ����� ��������� %s"), m_sReportPaths[i].GetString());
			MessageBox(NULL, sErrorMessage, _T("������"), MB_OK | MB_ICONERROR);
			continue;
		}

		RepFile.SeekToBegin();
		while (RepFile.ReadString(sLine) == 1)
		{
			m_sReportData[i] += sLine;
			m_sReportData[i] += "\n";
		}
		RepFile.Close();
	}

	return 1;
}

/////////////////////////////////////////////////////////////////////////////
void CreateReport::CreateHeader()
{
	CString  m_sHeadFoot;		// ����� � �������
	TCHAR sString[100], sTemp[25];
	
	// ���������, ����� ������� ��� ������������
	m_pPdf->AddString("�������� �������� ������������ ��������� " + (CStringA)g_zProdTypes[g_zGenInfo.nProduct].szProdExt, HPDF_TALIGN_CENTER);
	m_pPdf->AddString((CStringA)m_sHeadFoot);
	/*m_sHeadFoot += "�������� �������� ������������ ��������� ";
	m_sHeadFoot += g_zProdTypes[g_zGenInfo.nProduct].szProdExt;*/
	// ����
	int nPos = _tcschr(&g_sTime[0], ' ') - &g_sTime[0];
	_tcsncpy_s(sTemp, &g_sTime[0], nPos);
	// ��� �����
	//_stprintf_s(sString, _T("\n%-23s: %s"), _T("���� ��������"), _T("17.07.2019"));
	_stprintf_s(sString, _T("\n%-23s: %s"), _T("���� ��������"), sTemp);
	m_sHeadFoot += sString;
	// �����
	//_stprintf_s(sString, _T("\n%-23s: %s"), _T("����� ��������"), _T("14:21:03"));	// ��� �����
	_stprintf_s(sString, _T("\n%-23s: %s"), _T("����� ��������"), &g_sTime[nPos + 1]);	
	m_sHeadFoot += sString;
	// ����� ���������� �����
	_stprintf_s(sString, _T("\n%-23s: %s"), _T("����� ����������"), g_zGenInfo.lpszPosition);
	m_sHeadFoot += sString;
	// ��� ��������. ��� ����� ������, �.�. � ������ 1.2.0.0 ����� �� ����
	_stprintf_s(sString, _T("\n%-23s: %s"), _T("��� ��������"), g_zGenInfo.lpszStorage);
	m_sHeadFoot += sString;
	// ��������
	_stprintf_s(sString, _T("\n%-23s: %s"), _T("��������"), g_zGenInfo.lpszOperator);
	m_sHeadFoot += sString;
	// ������������� ���
	_stprintf_s(sString, _T("\n%-23s: %s"), _T("������������� ���"), g_zGenInfo.lpszOtkMan);
	m_sHeadFoot += sString;
	// ������������� �� �� ��
	_stprintf_s(sString, _T("\n%-23s: %s"), _T("������������� �� �� ��"), g_zGenInfo.lpszVpMan);
	m_sHeadFoot += sString;
	// ��������� �����
	_stprintf_s(sString, _T("\n%-23s: %s"), _T("��������� �����"), g_zGenInfo.lpszProdNum);
	m_sHeadFoot += sString;
	// ��������� ��������
	_stprintf_s(sString, _T("\n%-23s: %s"), _T("����������� ���������"), g_zGenInfo.lpszResult);		// ��� ����� _T("���������������"));
	m_sHeadFoot += sString;
	// ������ ���������
	_stprintf_s(sString, _T("\n%-23s: %s"), _T("������ ��"), g_sVersFromConfig);
	m_sHeadFoot += sString;

	m_sHeadFoot += "\n\n";

	m_pPdf->AddString((CStringA)m_sHeadFoot);

	// �������� �������
	CStringArray TableData;

	TableData.Add(_T("��������� �����"));
	TableData.Add(_T("����������� ���������"));
	for (int i = 0; i < g_nNumTests /*NUMCHECKS*/; i++)
	{
		TableData.Add(g_zExeInfo[g_nTestIndex[i]].lpszCheckName);

		//TableData.Add(_T("���������������"));			// ��� �����

		if (g_zExeInfo[g_nTestIndex[i]].nResult == 1)
			TableData.Add(_T("���������������"));
		else if (g_zExeInfo[g_nTestIndex[i]].nResult == -88)
			TableData.Add(_T("�� �����������"));
		else
			TableData.Add(_T("�����������������"));
	}
	m_pPdf->CreateTable(0, 0, m_pPdf->GetPageWidth(1), m_pPdf->GetPageHeight(1) /*/ 2.f*/, g_nNumTests + 1 /*NUMCHECKS*/, 2, TableData, HPDF_TALIGN_CENTER | HPDF_TALIGN_MIDDLE);

	CreateFooter(&m_sHeadFoot);
	m_pPdf->AddString((CStringA)m_sHeadFoot, HPDF_TALIGN_RIGHT);
}

/////////////////////////////////////////////////////////////////////////////
void CreateReport::CreateFooter(CString* sFooter)
{
	int nMaxCount = 0;
	int nMaxSymb = 22;		// ����� �������� 26 ��������, 26 - /  / = 22
	CString sFormOper = _T("\n��������:              ");
	CString sFormCont = _T("\n������������ ���:      ");
	CString sFormCust = _T("\n������������� �� �� ��:");
	TCHAR sTemp[250];
	sFooter->Empty();

	// ���������� ������������ ������ ��������
	int nOper = _tcslen(g_zGenInfo.lpszOperator);
	int nCont = _tcslen(g_zGenInfo.lpszOtkMan);
	int nCust = _tcslen(g_zGenInfo.lpszVpMan);

	if (nOper >= nCont)
		if (nOper >= nCust)
			nMaxCount = nOper;
		else
			nMaxCount = nCust;
	else if (nCont >= nCust)
		nMaxCount = nCont;
	else
		nMaxCount = nCust;

	if (nMaxCount > nMaxSymb)
		nMaxSymb = nMaxCount + 15;		// ����� ��� �������

	// �������
	_tmemset(sTemp, _T(' '), nMaxSymb - nMaxCount);
	sTemp[nMaxSymb - nMaxCount] = '\0';
	sFormOper += sTemp;
	sFormCont += sTemp;
	sFormCust += sTemp;
	// ������
	_stprintf_s(sTemp, _T("/ %%%ds /"), nMaxCount);
	sFormOper += sTemp;
	sFormCont += sTemp;
	sFormCust += sTemp;

	_stprintf_s(sTemp, sFormOper, g_zGenInfo.lpszOperator);
	*sFooter += sTemp;

	_stprintf_s(sTemp, sFormCont, g_zGenInfo.lpszOtkMan);
	*sFooter += sTemp;

	_stprintf_s(sTemp, sFormCust, g_zGenInfo.lpszVpMan);
	*sFooter += sTemp;
}

/////////////////////////////////////////////////////////////////////////////
void CreateReport::CreateFootString(char* sFootString, int nSize)
{
	long long lMask = 0x18051248913;
	CString sNumber(g_sTime);
	sNumber.Remove(' ');
	sNumber.Remove('.');
	sNumber.Remove(':');
	sNumber.MakeReverse();

	long long lNumber = _ttoi64(sNumber);
	lNumber -= lMask;
	
	// ��������������
	/*lNumber += lMask;
	sNumber.Format(_T("%lld"), lNumber);
	sNumber.MakeReverse();*/

	sprintf_s(sFootString, nSize, "%lld", lNumber);
}

/////////////////////////////////////////////////////////////////////////////
int CreateReport::CreatePdf(TCHAR* sReportPath)
{
	char szFontPath[MAX_PATH];		// �� TCHAR, ������ ��� ����� �������� � HaruClass, ��� ������ char

	if (m_pPdf->InitDll("libhpdf.dll") == 0)
		return 0;

	if (m_pPdf->InitPdf(HPDF_PAGE_SIZE_A4, HPDF_PAGE_LANDSCAPE) == 0)
		return 0;

	GetWindowsDirectoryA(szFontPath, _countof(szFontPath));
	strcat_s(szFontPath, "\\Fonts\\cour.ttf");
	if (m_pPdf->InitFont(szFontPath, 12, 1, 15, 20, 25, 15) == 0)
		return 0;

	char sFootString[15];
	CreateFootString(sFootString, _countof(sFootString));
	m_pPdf->SetFooter(1, sFootString);

	// ������ �������� ����� �������
	if (m_pPdf->InitPage() == 0)
		return 0;

	CreateHeader();

	if (m_nNumReports != 0)
	{
		// �������� ��������� �������� � ����� ��������
		m_pPdf->InitPage();
		m_pPdf->AddString("��������� �������� �������� ������������ ��������� " + (CStringA)g_zProdTypes[g_zGenInfo.nProduct].szProdExt + "\n\n", HPDF_TALIGN_CENTER);
	}

	// ��������� ��������
	for (int i = 0; i < m_nNumReports; i++)
	{
		BOOL bFlag = 0;			// ��������� ���� ��� ��������� � 2 ���������

		if (m_sReportData[i].IsEmpty() == 1)
			continue;

		if (_tcsstr(m_sReportData[i], _T("������ � ���������")))
			bFlag = 1;

		// ������ ������ ����� �� ������� ������ \x1c - ������ � ����� �������, \x1d - ������ �������, \x1e - ������� �� ����� ��������
		while (m_sReportData[i] != "")
		{
			int nPos, nItems = 0, nColumn = 0;
			CString sText;
			CStringArray sDataArr;

			nPos = m_sReportData[i].Find('\x1c');
			//if (nPos == -1)		// ��� ������
			//{
			//	// �������� ���������� �����
			//	m_pPdf->AddString((CStringA)m_sReportData[i]);
			//	break;
			//}

			// ��������� �������� ������ �� �������
			sText = m_sReportData[i].Left(nPos);
			m_pPdf->AddString((CStringA)sText);
			m_sReportData[i].Delete(0, nPos);

			do {
				if (sText.Find(_T("---------")) == -1)
					nItems = StringSplit(sText, sDataArr, _T('\x1d'));			// ���������� ���������
				else
					nColumn = nItems;						// ������ ������ �������� ���������� ��������, �������� ������ ���� ����������� ���� ����� ��������

				nPos = m_sReportData[i].Find('\n');
				sText = m_sReportData[i].Left(nPos);
				m_sReportData[i].Delete(0, nPos + 1);

			} while ((sText.Find(_T("\x1c")) == -1) || (nColumn == 0));			// ���� �� ����������� ������� (�� ������ ������), nColumn == 0 ��� ������ �� ������ ����� ��������� 1 ������

			// ��������� ����� ������� ���-�� �������, ����������� ������ ������� �� ��������� �����
			if (bFlag)
				m_pPdf->CreateTable(0, 0, m_pPdf->GetPageWidth(1), m_pPdf->GetPageHeight(1) / 5.f, nItems / nColumn, nColumn, sDataArr, HPDF_TALIGN_MIDDLE | HPDF_TALIGN_CENTER);
			else if (nItems / nColumn > 4)
				m_pPdf->CreateTable(0, 0, m_pPdf->GetPageWidth(1), m_pPdf->GetPageHeight(1) / 1.75f, nItems / nColumn, nColumn, sDataArr, HPDF_TALIGN_MIDDLE | HPDF_TALIGN_CENTER);
			else
				m_pPdf->CreateTable(0, 0, m_pPdf->GetPageWidth(1), m_pPdf->GetPageHeight(1) / 2.5f, nItems / nColumn, nColumn, sDataArr, HPDF_TALIGN_MIDDLE | HPDF_TALIGN_CENTER);

			// ���� � ��������� ������ ����� �������
			nPos = m_sReportData[i].Find('\x1c');
			if (nPos != -1)
				continue;

			// ������ ������ ����� �������, ����������� ���������� �� ��� ������
			nPos = m_sReportData[i].Find('\x1e');
			m_pPdf->AddString((CStringA)m_sReportData[i].Left(nPos));					
			if (nPos != -1 && i != m_nNumReports - 1)		// ���� ������ ������ �������� �������� � �������� �� ���������
				m_pPdf->InitPage();
			m_sReportData[i].Delete(0, nPos + 2);			// ������� ������ \x1e\n

			// �������� �� ����� ��������
			//nPos = m_sReportData[i].Find('\x1e');
			//if (nPos != -1 && i != m_nNumReports - 1)		// ���� ������ ������ �������� �������� � �������� �� ���������
			//	m_pPdf->InitPage(sFootString);
			//m_sReportData[i].Delete(0, nPos + 2);			// ������� ������ \x1e\n

			// ������ ������ �������
			/*nPos = m_sReportData[i].Find('\n');
			sText = m_sReportData[i].Left(nPos);
			m_sReportData[i].Delete(0, nPos + 1);

			if (sText.Find('\x1e') != -1 && i != m_nNumReports - 1)		// ���� ������ ������ �������� �������� � �������� �� ���������
				m_pPdf->InitPage(sFootString);

			CStringA m_sReportDataA(sText.Right(m_sReportData[i].GetLength() - 1));		// �������� ��� ������� \x1e
			m_pPdf->AddString(m_sReportDataA);*/
		}
	}

	m_pPdf->SetPermission("owner");
	if (m_pPdf->SavePdf(CT2A(sReportPath)) == 0)
		return 0;
	_tcscpy_s(g_sReportPath, sReportPath);

	return 1;
}

/////////////////////////////////////////////////////////////////////////////
int CreateReport::StringSplit(const CString & str, CStringArray & arr, TCHAR chDelimitior)
{
	int nStart = 0, nEnd = 0;

	while (nEnd < str.GetLength())
	{
		nEnd = str.Find(chDelimitior, nStart);
		if (nEnd == -1)
		{
			//nEnd = str.GetLength();
			break;							// �� ������������� ����� ������ ����� ���������� chDelimitior
		}

		CString s = str.Mid(nStart, nEnd - nStart);
		if (s.IsEmpty() == 0)
		{
			s.Remove(_T('|'));
			s.Trim(_T(' '));
			arr.Add(s);
		}

		nStart = nEnd + 1;
	}

	return arr.GetSize();
}
