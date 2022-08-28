// Page_Archive.cpp: ���� ����������
//

#include "stdafx.h"
#include "CheckFut.h"
#include "Page_Archive.h"
#include "ReportCtrl.h"
#include "CalendarDlg.h"
#include "afxdialogex.h"

// ���������� ���� Page_Archive
///////////////////////////////////////////////////////////////////////////////////////
IMPLEMENT_DYNAMIC(Page_Archive, CPropertyPage)

Page_Archive::Page_Archive()
	: CPropertyPage(IDD_PAGE_ARCHIVE)
{
	m_ListCtrl = NULL;
	// ������ ������
	m_pFiles = NULL;
	// ���������� ��������� ������
	m_nCntFiles = 0;
}

Page_Archive::~Page_Archive()
{
	// ������� ������ ������
	if (m_pFiles != NULL)
	{
		delete[] m_pFiles;
		m_pFiles = NULL;
	}

	// ������� ListCtrl
	if (m_ListCtrl != NULL)
	{
		delete m_ListCtrl;
		m_ListCtrl = NULL;
	}
}

void Page_Archive::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(Page_Archive, CPropertyPage)
	ON_EN_CHANGE(IDC_EDIT_NUM_IZD, &Page_Archive::OnChangeEditNumIzd)
	ON_EN_CHANGE(IDC_EDIT_DATA_FROM, &Page_Archive::OnChangeEditDataFrom)
	ON_EN_CHANGE(IDC_EDIT_DATA_UNTIL, &Page_Archive::OnChangeEditDataUntil)
	ON_BN_CLICKED(IDC_BT_DATA_FROM, &Page_Archive::OnClickedBtDataFrom)
	ON_BN_CLICKED(IDC_BT_DATA_UNTIL, &Page_Archive::OnClickedBtDataUntil)
	ON_BN_CLICKED(IDC_BT_SHOW, &Page_Archive::OnBnClickedBtShow)
	ON_NOTIFY(NM_DBLCLK, IDC_LIST_PROTOCOL, &Page_Archive::OnDblclkListProtocol)
	ON_EN_CHANGE(IDC_EDIT_TYPE_IZD, &Page_Archive::OnChangeEditTypeIzd)
END_MESSAGE_MAP()


// ����������� ��������� Page_Archive
///////////////////////////////////////////////////////////////////////////////////////

BOOL Page_Archive::OnInitDialog()
{
	CreateListFont();
	InitListCtrl();

	return TRUE;  // return TRUE unless you set the focus to a control
				  // ����������: �������� ������� OCX ������ ���������� �������� FALSE
}

///////////////////////////////////////////////////////////////////////////////////////
BOOL Page_Archive::OnSetActive()
{
	// ������������� ������ ����������
	InitListProtocol();

	CPropertySheet* pSheet = (CPropertySheet*)GetParent();
	pSheet->SetWizardButtons(PSWIZB_BACK);

	return CPropertyPage::OnSetActive();
}

///////////////////////////////////////////////////////////////////////////////////////
int Page_Archive::CreateListFont()
{
	LOGFONT pz;

	pz.lfHeight = 0;			// ������ � ���.��������
	pz.lfWidth = NULL;			// ������� ������ (0-����������)
	pz.lfEscapement = NULL;	// ���� �������
	pz.lfOrientation = NULL;	// ���� �������
	pz.lfWeight = 0;// FW_SEMIBOLD;	// ������� ����� ����������
	pz.lfItalic = FALSE;		// ������
	pz.lfUnderline = FALSE;	// �������������
	pz.lfStrikeOut = FALSE;	// ��������������
	pz.lfCharSet = DEFAULT_CHARSET;			// ������� ���������
	pz.lfOutPrecision = OUT_DEFAULT_PRECIS;	// �������� ������ ������
	pz.lfClipPrecision = CLIP_DEFAULT_PRECIS;	// �������� ��������� ��������
	pz.lfQuality = DEFAULT_QUALITY;			// �������� ������
	pz.lfPitchAndFamily = DEFAULT_PITCH;		// ��� � ��������� �������
	_tcscpy_s(pz.lfFaceName, _countof(pz.lfFaceName), _T("Courier New"));		// ��� ������ (<= 32 �������)
	m_cListFont.CreateFontIndirect(&pz);

	return 1;
}

///////////////////////////////////////////////////////////////////////////////////////
void Page_Archive::InitListCtrl()
{
	CRect rectCtrl;
	TCHAR sFormatHeader[50];
	int nColumsWidths[4];

	m_ListCtrl = (CReportCtrl*) new class CReportCtrl();
	m_ListCtrl->AssignList(this, IDC_LIST_PROTOCOL, &m_cListFont);

	m_ListCtrl->GetClientRect(rectCtrl);
	nColumsWidths[0] = (int)(rectCtrl.Width() * 0.1);
	nColumsWidths[1] = (int)(rectCtrl.Width() * 0.22);
	nColumsWidths[2] = (int)(rectCtrl.Width() * 0.31);
	nColumsWidths[3] = (int)(rectCtrl.Width() * 0.37);
	_stprintf_s(sFormatHeader, _T("�! %d! 1;�������! %d;�����! %d;����! %d"), nColumsWidths[0], nColumsWidths[1], nColumsWidths[2], nColumsWidths[3] - 1);

	m_ListCtrl->SetColumnHeader(sFormatHeader);
	m_ListCtrl->SetGridLines(TRUE); // SHow grid lines
	m_ListCtrl->SetSortable(0);

	// �������� ��������� ������ (� Win XP ���� ���������� ������� �� ����)
	SetWindowTheme(GetDlgItem(IDC_ST_NUM_IZD)->m_hWnd, _T(""), _T(""));
	SetWindowTheme(GetDlgItem(IDC_ST_NUM_IZD2)->m_hWnd, _T(""), _T(""));
	SetWindowTheme(GetDlgItem(IDC_ST_DATA_IZD)->m_hWnd, _T(""), _T(""));
}

///////////////////////////////////////////////////////////////////////////////////////
void Page_Archive::InitListProtocol()
{
	// �������� ������ ������
	RefreshFiles();  
	if (m_pFiles == NULL)  
		return;
	// ����������� �� ����
	SortFiles();
	// �������� ��������
	ShowProtocols();

	if (m_ListCtrl->GetItemCount() > 0)
	{
		m_ListCtrl->SetItemState(0, LVIS_SELECTED, LVIS_SELECTED);
		m_ListCtrl->SetItemState(0, 0, 0);
		m_ListCtrl->SetSelectionMark(0);

		GetDlgItem(IDC_BT_SHOW)->EnableWindow(TRUE);
	}
}

///////////////////////////////////////////////////////////////////////////////////////
int Page_Archive::GetIdxFiles(int iList)
{
	int  iFile = -1;
	if (iList        <    0)  
		return iFile;
	if (m_pFiles == NULL)  
		return iFile;
	if (m_nCntFiles <= 0)  
		return iFile;

	CString  str[3], strTemp;
	str[0] = m_ListCtrl->GetItemText(iList, 1); // ��� �������
	str[1] = m_ListCtrl->GetItemText(iList, 2); // �����
	str[2] = m_ListCtrl->GetItemText(iList, 3); // ����

	// ����
	str[2].Remove('.'); str[2].Remove(':'); str[2].Remove(' ');
	strTemp = str[2].Mid(4, 4); str[2].Delete(4, 4); str[2].Insert(0, strTemp);
	strTemp = str[2].Mid(4, 2); str[2].Delete(4, 2); str[2].Insert(6, strTemp);

	int idx = m_nCntFiles;
	do
	{
		if (str[0].Compare(m_pFiles[idx].sName) == 0 &&
			str[1].Compare(m_pFiles[idx].sNum)  == 0 &&
			str[2].Compare(m_pFiles[idx].sData) == 0)
		{
			iFile = idx;
			break;
		}
		idx--;
	} while (idx > -1);

	return  iFile;
}

///////////////////////////////////////////////////////////////////////////////////////
BOOL Page_Archive::bCheckNum(LPCTSTR strNum, LPCTSTR strMask)
{
	BOOL     bRet = FALSE;
	CString  sNum(strNum), sMask(strMask), ch1, ch2;

	if (sMask.IsEmpty() != FALSE)             
		return TRUE;
	if (sMask == "*")             
		return TRUE;
	if (sMask.Compare(sNum) == 0)             
		return TRUE;
	if (sMask.GetLength()   >  sNum.GetLength())  
		return FALSE;

	// ���� � ����� ������ *, �� ���������� �����
	int nLen = sMask.GetLength();
	int nPos = sMask.Find('*');
	if (nPos >= 0)
	{
		// * �� ������ �����
		if (nPos == 0)
		{
			ch1 = sMask.Right(nLen - 1);
			ch2 = sNum.Right(nLen - 1);
			if (ch1.Compare(ch2) == 0) 
				bRet = TRUE;
		}
		// * �� ��������� �����
		else
			if (nPos == nLen - 1)
			{
				ch1 = sMask.Left(nLen - 1);
				ch2 = sNum.Left(nLen - 1);
				if (ch1.Compare(ch2) == 0) 
					bRet = TRUE;
			}
		// * � ��������
			else
			{
				bRet = TRUE;
				// ����� �����
				ch1 = sMask.Left(nPos);
				ch2 = sNum.Left(nPos);
				if (ch1.Compare(ch2) != 0) 
					bRet *= FALSE;
				// ����� ������
				ch1 = sMask.Right(nLen - nPos - 1);
				ch2 = sNum.Right(nLen - nPos - 1);
				if (ch1.Compare(ch2) != 0) 
					bRet *= FALSE;
			}
	}

	return bRet;
}

///////////////////////////////////////////////////////////////////////////////////////
int Page_Archive::nCheckData(LPCTSTR strData, LPCTSTR strMask)
{
	int  iRet = 0;
	CString  sData(strData), sMask(strMask), ch1, ch2;

	if (sMask.IsEmpty() != FALSE)  
		return 0;
	if (sMask.GetLength() > 10)  
		return 0;
	if (sMask.GetLength() < 10)  
		return 0;

	ch1 = sData.Left(8);
	ch2.Format(_T("%s%s%s"), sMask.Mid(6, 4), sMask.Mid(3, 2), sMask.Mid(0, 2));
	iRet = ch2.Compare(ch1);

	return iRet;
}

///////////////////////////////////////////////////////////////////////////////////////
void Page_Archive::SortFiles()
{
	CString            str;
	if (m_pFiles == NULL)  
		return;

	for (int i = 0; i < m_nCntFiles; i++)
		for (int j = 0; j < m_nCntFiles - 1; j++)
		{
			str = m_pFiles[j].sData;
			if (str.Compare(m_pFiles[j + 1].sData) > 0)
				ChangeFiles(j, j + 1, m_pFiles);
		}
}

///////////////////////////////////////////////////////////////////////////////////////
void Page_Archive::ChangeFiles(int i, int j, PARHIVEPAGE_FILES pFiles)
{
	ARHIVEPAGE_FILES   file;

	memcpy((void*)&(file), (void*)(m_pFiles + i), sizeof(ARHIVEPAGE_FILES));
	memcpy((void*)(m_pFiles + i), (void*)(m_pFiles + j), sizeof(ARHIVEPAGE_FILES));
	memcpy((void*)(m_pFiles + j), (void*)&(file), sizeof(ARHIVEPAGE_FILES));
}

///////////////////////////////////////////////////////////////////////////////////////
void Page_Archive::ShowProtocols()
{
	CString  str, strCol[3];
	LV_ITEM  lvItem;
	ZeroMemory(&lvItem, sizeof(LV_ITEM));

	m_ListCtrl->DeleteAllItems();

	lvItem.mask = LVIF_TEXT | LVIF_STATE;

	int   iFiles = -1, iList = 0;
	while (iFiles++ < m_nCntFiles - 1)
	{
		// ��� �������
		GetDlgItemText(IDC_EDIT_TYPE_IZD, str);
		str.MakeUpper();
		if (str != "" && _tcscmp(m_pFiles[iFiles].sName, str) != 0)
			continue;
		strCol[0] = m_pFiles[iFiles].sName;

		// ����� �������
		GetDlgItemText(IDC_EDIT_NUM_IZD, str);
		if (bCheckNum(m_pFiles[iFiles].sNum, str) == FALSE) 
			continue;
		strCol[1] = m_pFiles[iFiles].sNum;

		// ����/�����
		// ��
		GetDlgItemText(IDC_EDIT_DATA_FROM, str);
		if (nCheckData(m_pFiles[iFiles].sData, str) > 0) 
			continue;
		GetDlgItemText(IDC_EDIT_DATA_UNTIL, str);
		if (nCheckData(m_pFiles[iFiles].sData, str) < 0) 
			continue;

		strCol[2].Format(_T("%.2s.%.2s.%.4s %.2s:%.2s:%.2s"),
			m_pFiles[iFiles].sData + 6, m_pFiles[iFiles].sData + 4, m_pFiles[iFiles].sData,
			m_pFiles[iFiles].sData + 8, m_pFiles[iFiles].sData + 10, m_pFiles[iFiles].sData + 12);

		//-----------------
		// ��������� ������
		//-----------------
		// ����� ���������
		lvItem.iItem = iList;
		lvItem.iSubItem = 0; 
		str.Format(/*_T("%5d")*/_T("%d"), iList + 1);
		lvItem.pszText = (LPTSTR)str.GetString();

		m_ListCtrl->InsertItem(&lvItem);
		m_ListCtrl->SetItemText(iList, 1, strCol[0]);
		m_ListCtrl->SetItemText(iList, 2, strCol[1]);
		m_ListCtrl->SetItemText(iList, 3, strCol[2]);

		iList++;
	}
	// �������� ������
	if (m_ListCtrl->GetItemCount() > 0)
		GetDlgItem(IDC_BT_SHOW)->EnableWindow(TRUE);
	else  
		GetDlgItem(IDC_BT_SHOW)->EnableWindow(FALSE);
}

///////////////////////////////////////////////////////////////////////////////////////
void Page_Archive::RefreshFiles()
{
	// ������� ������ ������
	if (m_pFiles != NULL)
	{
		delete[] m_pFiles;
		m_pFiles = NULL;
	}
	// ����� � ������� ���������� ������
	m_nCntFiles = 0;
	CString  strDir("Report\\");
	SearchFiles(strDir, &m_nCntFiles, NULL);
	// ���������� ������ ������
	if (m_nCntFiles > 0)
	{
		m_pFiles = new ARHIVEPAGE_FILES[m_nCntFiles];
		int nCnt = 0;
		SearchFiles(strDir, &nCnt, m_pFiles);
	}
}

///////////////////////////////////////////////////////////////////////////////////////
void Page_Archive::SearchFiles(LPCTSTR strDir, int * pnCnt, PARHIVEPAGE_FILES pArchFiles)
{
	WIN32_FIND_DATA  wfd;
	CString          strTemp;
	CString          strMask;  
	strMask.Format(_T("%s\\*"), strDir);
	HANDLE           h = FindFirstFile(strMask, &wfd);
	if (h == INVALID_HANDLE_VALUE)
		return;

	while (FindNextFile(h, &wfd))
	{
		// ����� ������ � ��������� ����������
		if ((wfd.dwFileAttributes == FILE_ATTRIBUTE_DIRECTORY) &&
			_tcscmp(wfd.cFileName, _T("..")) != 0 && _tcscmp(wfd.cFileName, _T(".")) != 0)
		{
			strTemp.Format(_T("%s\\%s"), strDir, wfd.cFileName);
			SearchFiles(strTemp, pnCnt, pArchFiles);
		}
		// ����� ������ �� �����
		if (wfd.dwFileAttributes != FILE_ATTRIBUTE_DIRECTORY)
		{
			// �������� ������� ����� �����
			if (CheckFile(wfd.cFileName) == 0)
				continue;

			strTemp = wfd.cAlternateFileName;
			strTemp = strTemp.Right(3); 
			strTemp.MakeLower();
			if ((strTemp == _T("pdf") && (_tcscmp(wfd.cFileName, _T("~last.pdf")))) != 0)
			{
				// ���������� ��������� ���������� �����
				if (pArchFiles != NULL)
				{
					//----------
					// ��� �����
					//----------
					strTemp.Format(_T("%s%s"), strDir, wfd.cFileName);
					_tcscpy_s(pArchFiles[*pnCnt].sFile, (LPCTSTR)strTemp);

					//-----------------
					// �������� �������
					//-----------------
					
					strTemp = wfd.cFileName;
					_tcscpy_s(pArchFiles[*pnCnt].sName, strTemp.Left(strTemp.Find(_T(" - "))));
					
					//--------------
					// ����� �������
					//--------------
					strTemp = strTemp.Right(strTemp.GetLength() - strTemp.Find(_T(" - ")) - 3);
					_tcscpy_s(pArchFiles[*pnCnt].sNum, strTemp.Left(strTemp.Find(_T(" - "))));

					//--------------
					// ����
					//--------------
					strTemp = strTemp.Right(strTemp.GetLength() - strTemp.Find(_T(" - ")) - 3);
					strTemp = strTemp.Left(strTemp.GetLength() - 4);
					strTemp.Remove(' ');
					strTemp.Remove('.');
					// �������������� � ������ [���|���|����|���|���|���: 20091224012334]
					strTemp.Format(_T("%s%s%s%s"), strTemp.Mid(4, 4), strTemp.Mid(2, 2), strTemp.Left(2), strTemp.Right(6));

					_tcscpy_s(pArchFiles[*pnCnt].sData, strTemp);
				}
				(*pnCnt)++;
			}
		}
	};

	FindClose(h);

	return;
}

///////////////////////////////////////////////////////////////////////////////////////
BOOL Page_Archive::CheckFile(TCHAR * sFileName)
{
	TCHAR sOldTempName[MAX_PATH];
	TCHAR sTempName[MAX_PATH];
	TCHAR* sParam = NULL;
	int nPos = 0;

	_tcscpy_s(sOldTempName, sFileName);
	// ��������� .pdf
	sParam = _tcsrchr(sOldTempName, '.');
	nPos = (int)(sParam - sOldTempName);
	sOldTempName[nPos] = '\0';

	for(int i = 0; i < 2; i++)
	{
		_tcscpy_s(sTempName, sOldTempName);
		sParam = _tcsstr(sTempName, _T(" - "));
		// ���� ��� �������
		if (sParam == NULL)
			return 0;

		nPos = (int)(sParam - sTempName);
		_tcscpy_s(sOldTempName, &sParam[3]);
		sTempName[nPos] = '\0';
		sParam = sTempName;

		switch (i)
		{
			case 0: 
				if (_tcslen(sParam) > _MAX_NAME_)
					return 0;
				break;
			case 1:
				if (_tcslen(sParam) > _MAX_NUM_)
					return 0;
				if (_tcslen(sOldTempName) > _MAX_DATA_)
					return 0;
				break;
		}
	}

	return 1;
}

///////////////////////////////////////////////////////////////////////////////////////
void Page_Archive::OnChangeEditNumIzd()
{
	ShowProtocols();
}

///////////////////////////////////////////////////////////////////////////////////////
void Page_Archive::OnChangeEditTypeIzd()
{
	ShowProtocols();	
}

///////////////////////////////////////////////////////////////////////////////////////
void Page_Archive::OnChangeEditDataFrom()
{
	CString  str;
	GetDlgItemText(IDC_EDIT_DATA_FROM, str);
	if (str.GetLength() == 0 || str.GetLength() == 10) 
		ShowProtocols();
}

//======================================================================================
void Page_Archive::OnChangeEditDataUntil()
{
	CString  str;
	GetDlgItemText(IDC_EDIT_DATA_UNTIL, str);
	if (str.GetLength() == 0 || str.GetLength() == 10)  
		ShowProtocols();
}

///////////////////////////////////////////////////////////////////////////////////////
void Page_Archive::OnClickedBtDataFrom()
{
	CString       str;
	CalendarDlg  dlg;
	SYSTEMTIME    time;

	dlg.GetTime(&time);
	if (dlg.DoModal() == IDOK)
	{
		str.Format(_T("%02d.%02d.%04d"), time.wDay, time.wMonth, time.wYear);
		SetDlgItemText(IDC_EDIT_DATA_FROM, str);
	}
}

//=======================================================================================
void Page_Archive::OnClickedBtDataUntil()
{
	CString       str;
	CalendarDlg  dlg;
	SYSTEMTIME    time;

	dlg.GetTime(&time);
	if (dlg.DoModal() == IDOK)
	{
		str.Format(_T("%02d.%02d.%04d"), time.wDay, time.wMonth, time.wYear);
		SetDlgItemText(IDC_EDIT_DATA_UNTIL, str);
	}
}

///////////////////////////////////////////////////////////////////////////////////////
void Page_Archive::OnBnClickedBtShow()
{
	TCHAR sCommand[512];
	_stprintf_s(sCommand, _T("%s%s"), _T("rundll32.exe url.dll, FileProtocolHandler "), m_pFiles[GetIdxFiles(m_ListCtrl->GetSelectionMark())].sFile);
	::WinExec(CT2A(sCommand), SW_SHOWNORMAL);

	/*char sCommand[512];
	sprintf_s(sCommand, "%s%s", "rundll32.exe url.dll, FileProtocolHandler ", "Report3.pdf");
	::WinExec(sCommand, SW_SHOWNORMAL);*/
}

//=====================================================================================
void Page_Archive::OnDblclkListProtocol(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	
	TCHAR sCommand[512];
	_stprintf_s(sCommand, _T("%s%s"), _T("rundll32.exe url.dll, FileProtocolHandler "), m_pFiles[GetIdxFiles(m_ListCtrl->GetSelectionMark())].sFile);
	::WinExec(CT2A(sCommand), SW_SHOWNORMAL);

	*pResult = 0;
}