
// CheckFut.cpp : Определяет поведение классов для приложения.
//

#include "stdafx.h"
#include "CheckFut.h"
#include "MainSheet.h"
#include "Config.h"
#include "CheckFut.hpp"
//#include "SettingsDlg.h"
#include "Page_List.h"
#include "CreateReport.h"
#include "MesTemplate.h"
#include "PowerDlg.h"
#include "ResDlg.h"
#include <locale.h>

/////////////////////////////////////////////////////////////////////////////
// CCheckFutApp

BEGIN_MESSAGE_MAP(CCheckFutApp, CWinApp)
	ON_COMMAND(ID_HELP, &CWinApp::OnHelp)
END_MESSAGE_MAP()


// создание CCheckFutApp

CCheckFutApp::CCheckFutApp()
{
	m_nTestCount = 0;
	m_hSwapping = NULL;
	m_pSwap = NULL;
}

CString temp_str;
// Единственный объект CCheckFutApp

CCheckFutApp theApp;

/////////////////////////////////////////////////////////////////////////////
// инициализация CCheckFutApp

BOOL CCheckFutApp::InitInstance()
{
	// InitCommonControlsEx() требуется для Windows XP, если манифест
	// приложения использует ComCtl32.dll версии 6 или более поздней версии для включения
	// стилей отображения.  В противном случае будет возникать сбой при создании любого окна.
	INITCOMMONCONTROLSEX InitCtrls;
	InitCtrls.dwSize = sizeof(InitCtrls);
	// Выберите этот параметр для включения всех общих классов управления, которые необходимо использовать
	// в вашем приложении.
	InitCtrls.dwICC = ICC_WIN95_CLASSES;
	InitCommonControlsEx(&InitCtrls);

	CWinApp::InitInstance();

	// Активация визуального диспетчера "Классический Windows" для включения элементов управления MFC
	CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerWindows));

	// Установка локали для записи протокола
	_tsetlocale(LC_CTYPE, _T("Russian"));

	// Загрузка данных из файла конфигурации
	g_pCFG = new Config();
	if (g_pCFG->CheckExist() == 0)
	{
		MessageBox(NULL, _T("Не найден файл конфигурации"), _T("ОШИБКА"), MB_OK | MB_ICONERROR);
		return FALSE;
	}
	if (g_pCFG->ReadConfig() == 0)
	{
		MessageBox(NULL, _T("Ошибка чтения файла конфигурации"), _T("ОШИБКА"), MB_OK | MB_ICONERROR);
		return FALSE;
	}

	// Проверка версии программы с версией из файла
	TCHAR sCurVersion[128];
	GetAppVersion(sCurVersion);
	if (_tcscmp(sCurVersion, g_sVersFromConfig) != 0)
	{
		MessageBox(NULL, _T("Версия файла конфигурации и версия программы не совпадают!"), _T("ОШИБКА"), MB_OK | MB_ICONERROR);
		return FALSE;
	}

	g_pMainSheet = new MainSheet(_T("Sheet"));
	m_pMainWnd = g_pMainSheet;

	// Поскольку диалоговое окно закрыто, возвратите значение FALSE, чтобы можно было выйти из
	//  приложения вместо запуска генератора сообщений приложения.
	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
int CCheckFutApp::GetAppVersion(TCHAR * pProductVersion)
{
	DWORD   dwBufferSize;
	TCHAR   szVerBuffer[2048];
	TCHAR   szFilePath[MAX_PATH] = { 0 };

	struct LANGANDCODEPAGE {
		WORD wLanguage;
		WORD wCodePage;
	} *lpTranslate;
	unsigned int cbTranslate;
	TCHAR        csLang[32];
	TCHAR        csFormat[256];

	GetModuleFileName(NULL, szFilePath, sizeof(szFilePath));

	//  Get the size of the version info block in the file
	dwBufferSize = GetFileVersionInfoSize(szFilePath, NULL);
	if (dwBufferSize <= 0)
		return 0;

	//  Get the version block from the file
	if (!GetFileVersionInfo(szFilePath, NULL, dwBufferSize, szVerBuffer))
		return 0;

	// Read the list of languages and code pages.
	VerQueryValue(szVerBuffer, _T("\\VarFileInfo\\Translation"), (LPVOID*)&lpTranslate, &cbTranslate);
	if (cbTranslate / sizeof(struct LANGANDCODEPAGE) <= 0)
		return 0;

	_stprintf_s(csLang, _countof(csLang), _T("%04x%04x"), lpTranslate->wLanguage, lpTranslate->wCodePage);
	// Get the name and version strings
	LPVOID pvProductVersion = NULL;
	unsigned int iProductVersionLen = 0;

	_stprintf_s(csFormat, _countof(csFormat), _T("\\StringFileInfo\\%s\\ProductVersion"), csLang);
	VerQueryValue(szVerBuffer, csFormat, &pvProductVersion, &iProductVersionLen);
	if (szVerBuffer != NULL)
		_stprintf_s(pProductVersion, iProductVersionLen, _T("%s"), (TCHAR*)pvProductVersion);

	return 1;
}

/////////////////////////////////////////////////////////////////////////////
int CCheckFutApp::CheckExeExist(TCHAR * sCheckNames, int nBufSize)
{
	int nRet = 0;
	CFileFind Find;

	for (int i = 0; i < g_nNumTests /*NUMCHECKS*/; i++)
	{
		g_zExeInfo[g_nTestIndex[i]].nResult = -88;				// Т.к. функция вызывается перед началом контроля, то обнуляем результаты предыдущего контроля
		if (ListView_GetCheckState(g_pMainSheet->GetPage(4)->GetDlgItem(IDC_LIST)->GetSafeHwnd(), i) != 0 &&			// Проверять только отмеченные тесты
			Find.FindFile(g_zExeInfo[g_nTestIndex[i]].lpszPath) == 0)
		{
			_tcscat_s(sCheckNames, nBufSize, g_zExeInfo[g_nTestIndex[i]].lpszCheckName);
			_tcscat_s(sCheckNames, nBufSize, _T("\n"));
			nRet++;
		}
	}

	return nRet;
}

/////////////////////////////////////////////////////////////////////////////
int CCheckFutApp::SetControlMsg(LPARAM lParam)
{
	TCHAR pszResponse[MAX_PATH] = { 0 };
	float *fstr = (float*)lParam;
	int type = (int)fstr[0]; // Тип устройства
	int num = (int)fstr[1]; // Номер устройства
	int elem = (int)fstr[2]; // Количество элементов
	int mode = (int)fstr[3]; // Режим работы
	int state = (int)fstr[4]; // Состояние

	if (m_pSBM)
	{
		_SB_DEV_STATUS dev;
		int numDev = -1;
		if (m_pSBM->GetDevStatus(&dev, numDev, type, num, mode, state, pszResponse) != 0)
		{
			AfxMessageBox(pszResponse);
			return 1;
		}

		std::vector<_SBM_PARAM> vPar;
		for (int n = 0; n < elem; n++)
		{
			vPar.push_back(_SBM_PARAM(n, fstr[n + 3])); // Заполнение параметров со сдвигом на 3 элемента (тип,номер,кол-во)
		}
		if (m_pSBM->SetMsg_CONTROL(&dev, numDev, vPar, pszResponse) != 0)
		{
			AfxMessageBox(pszResponse);
			vPar.clear();
			return 2;
		}
		vPar.clear();
	}

	return 0;
}

/////////////////////////////////////////////////////////////////////////////
int CCheckFutApp::GetReplyMsg()
{
	TRACE(_T("WM_SB_CYCLE_GET_MSG!\n"));

	TCHAR pszResponse[MAX_PATH] = { 0 };
	int change = -1;
	_SB_DEV_STATUS dev;
	int numDev = -1;
	if (m_pSBM->GetMsg_HDR(change, numDev, &dev, pszResponse) != 0)
	{
		AfxMessageBox(pszResponse);
		return 1;
	}

	//--- MASTER
	if (change == 2) // INFO
	{
		TRACE(_T("GET INFO!\n"));

		// Специальные сообщения - Перечень оборудования
		if (dev.type == 0)
		{
			if (m_pSBM->InitDeviceInfo(pszResponse) != 0)
			{
				AfxMessageBox(pszResponse);
				return 2;
			}
			AfxMessageBox(pszResponse);
			g_iRun = 4;
		}
		else
		{
			// Обновление статуса устройств
			// Получение поля INFO
			if (m_pSBM->GetMsg_INFO(&dev, numDev, pszResponse) != 0)
			{
				AfxMessageBox(pszResponse);
				return 2;
			}
			CString msg;
			msg.Format(_T("Статус устройства: type:%d num:%d mode:%d state:%d\n"), dev.type, dev.num, dev.mode, dev.state);
			AfxMessageBox(msg);
		}
	}

	else if (change == 3) // REPLY
	{
		TRACE(_T("GET REPLY!\n"));
		std::vector<float> vPar;
		if (m_pSBM->GetMsg_REPLY(numDev, vPar, pszResponse) != 0)
		{
			AfxMessageBox(pszResponse);
			vPar.clear();
			return 2;
		}

		// Вывод на интерфейс - TEST!
		// Получение измерений
		if (dev.type == 1) // Источник питания
		{
			if (dev.num <= 3)
			{
				float MV = vPar.at(vPar.size() - 2); // Предпоследний
				float MC = vPar.at(vPar.size() - 1); // Последний

				//if (m_pRLMBDZ[dev.num - 1]->SetRemouteReply(dev.type, dev.num, dev.mode, dev.state, MV, MC, pszResponse) != 0)
				//{
				//	SetOutputMsg(pszResponse);
				//	return 3;
				//}
			}			
		}

		// Вывод в LixtBox
		CString msg, temp;
		msg.Format(_T("Получение ответа:  type:%d num:%d "), dev.type, dev.num);
		for (size_t i = 0; i < vPar.size(); i++)
		{
			temp.Format(_T("|%d=%.2f|"), i, vPar.at(i));
			msg.Append(temp);
		}

		AfxMessageBox(msg);
		vPar.clear();
	}

	else if (change == 4) // ERROR
	{
		TRACE(_T("GET ERROR!\n"));
		int error_cod = 0, error_more = 0;
		TCHAR pszError[512] = { 0 };
		if (m_pSBM->GetMsg_ERROR(error_cod, error_more, pszError, 512, pszResponse) != 0)
		{
			AfxMessageBox(pszResponse);
			g_iRun = 202;
			return 4;
		}
		else
		{
			CString msg;
			msg.Format(_T("Получена ошибка: type:%d num:%d cod:%d more:%d error:%s\n"), dev.type, dev.num, error_cod, error_more, pszError);
			g_iRun = 202;	// Выводить pszError
			AfxMessageBox(msg);
		}
	}
	return 0;
}

/////////////////////////////////////////////////////////////////////////////
DWORD CCheckFutApp::SystemCmd(const TCHAR * strFunct, const TCHAR * strParams)
{
	STARTUPINFO StartupInfo;
	PROCESS_INFORMATION ProcessInfo;
	TCHAR Args[4096] = { 0 };
	TCHAR *pEnvCMD = NULL;
	TCHAR *pDefaultCMD = _T("CMD.EXE");
	ULONG rc;

	memset(&StartupInfo, 0, sizeof(StartupInfo));
	StartupInfo.cb = sizeof(STARTUPINFO);
	StartupInfo.dwFlags = STARTF_USESHOWWINDOW;
	StartupInfo.wShowWindow = SW_HIDE;

	Args[0] = 0;

	size_t buf_sz = 0;
	//if (_wdupenv_s(&pEnvCMD, &buf_sz, _T("COMSPEC")) == 0 && pEnvCMD != nullptr)
	_wdupenv_s(&pEnvCMD, &buf_sz, _T("COMSPEC"));

	if (pEnvCMD) { _tcscpy_s(Args, pEnvCMD); free(pEnvCMD); }
	else _tcscpy_s(Args, pDefaultCMD);

	// "/c" option - Do the command then terminate the command window
	_tcscat_s(Args, _T(" /c "));
	//the application you would like to run from the command window
	_tcscat_s(Args, strFunct);
	_tcscat_s(Args, _T(" "));
	//the parameters passed to the application being run from the command window.
	if (strParams)
		_tcscat_s(Args, strParams);

	if (!CreateProcess(NULL, Args, NULL, NULL, FALSE, CREATE_NEW_CONSOLE, NULL, NULL, &StartupInfo, &ProcessInfo)) return GetLastError();

	WaitForSingleObject(ProcessInfo.hProcess, INFINITE);
	if (!GetExitCodeProcess(ProcessInfo.hProcess, &rc)) rc = 0;

	CloseHandle(ProcessInfo.hThread);
	CloseHandle(ProcessInfo.hProcess);

	return rc;
}

/////////////////////////////////////////////////////////////////////////////
UINT CCheckFutApp::GetCheckSum(char * buf, int buf_sz)
{
	UINT sum_buf = 0;
	for (int i = 0; i < buf_sz; i++)
		sum_buf += (unsigned char)buf[i];

	return sum_buf;
}

/////////////////////////////////////////////////////////////////////////////
int CCheckFutApp::InitFileMapping()
{
	m_hSwapping = ::CreateFileMapping((HANDLE)0xFFFFFFFF, NULL, PAGE_READWRITE, 0, m_nSwapSize,	(LPCTSTR)m_sSwapName);
	if (m_hSwapping == NULL)
		return 0;

	m_pSwap = (float*)::MapViewOfFile(m_hSwapping, FILE_MAP_ALL_ACCESS, 0, 0, m_nSwapSize);
	if (m_pSwap == NULL)
		return 0;

	return 1;
}

/////////////////////////////////////////////////////////////////////////////
void CCheckFutApp::SetModeAndPos()
{
	TCHAR* sCmdPar = _tcsstr(g_zExeInfo[*g_pCurCheck].lpszCmdPar[m_nCountRun - 1], _T("/tag"));
	if (sCmdPar != nullptr)
		sCmdPar[4] = ((g_nMode & 7) >> 1) + '0';		// Обнулить бит контейнера, сместить на 1 вправо и преобразоввать в строку

	// Добавление параметра места проведения. Пока колхоз, потому что нужно срочно. Потом можно добавить бит в g_nMode.
	sCmdPar = _tcsstr(g_zExeInfo[*g_pCurCheck].lpszCmdPar[m_nCountRun - 1], _T("/pos"));
	if (sCmdPar != nullptr)
		sCmdPar[4] = g_zGenInfo.nPosition + '0';
}

/////////////////////////////////////////////////////////////////////////////
int CCheckFutApp::PrevActionHandler()
{
	MesTemplate MessageDlg;
	TCHAR sMes[100];

	switch (g_zExeInfo[*g_pCurCheck].nPrevAction)
	{
		case 1: {
			_stprintf_s(sMes, _T("ФОРМА СИГНАЛА: СИНУСОИДАЛЬНАЯ\r\nЧАСТОТА: %d Гц\r\nАМПЛИТУДА НАПРЯЖЕНИЯ: %.1f В"), BASEFREQ, BASEAMPL);
			MessageDlg.SetStrings(_T("УСТАНОВИТЕ РЕЖИМ РАБОТЫ ГЕНЕРАТОРА:"), sMes);
			MessageDlg.DoModal();
		}; break;
		case 2: {
			// В подобных кейсах извлекать соответствующий параметр ком строки
			// Извлечение параметра /pres
			TCHAR* sCmdPar = _tcsstr(g_zExeInfo[*g_pCurCheck].lpszCmdPar[m_nCountRun - 1], _T("/pres"));
			if (sCmdPar != nullptr)
				sCmdPar = &sCmdPar[5];

			_stprintf_s(sMes, _T("%s кгс/см^2"), sCmdPar);
			// Допуски выводить не нужно
			/*if (_tcscmp(sCmdPar, _T("0")) == 0)
				_stprintf_s(sMes, _T("%s ± 0.18 кгс/см^2"), sCmdPar);
			else if (_tcscmp(sCmdPar, _T("5")) == 0)
				_stprintf_s(sMes, _T("%s ± 1.8 кгс/см^2"), sCmdPar);
			else if (_tcscmp(sCmdPar, _T("55")) == 0)
				_stprintf_s(sMes, _T("%s ± 1.6 кгс/см^2"), sCmdPar);*/

			MessageDlg.SetStrings(_T("УСТАНОВИТЕ ДАВЛЕНИЕ"), sMes);
			MessageDlg.DoModal();
		}; break;

		case 3:
		{
			// Уставку выставляем только при проверке БЭ
			if (g_nMode & 0x2)
			{
				MessageDlg.SetStrings(_T(""), _T("ПОДКЛЮЧИТЕ ЖГУТ ИВП К ПУЛЬТУ КОНТРОЛЯ БЭ"));
				MessageDlg.DoModal();
			}
		}; break;

		default:
			break;
	}

	return 0;
}

//===========================================================================
int CCheckFutApp::PostActionHandler(int ActionInfo)
{
	MesTemplate MessageDlg;

	switch (g_zExeInfo[*g_pCurCheck].nPostAction)
	{
		case 1: {
			MessageDlg.SetStrings(_T(""), _T("ОТКЛЮЧИТЕ ГЕНЕРАТОР"));
			MessageDlg.DoModal();
		}; break;

		case 2: {
			// Извлечение параметра /pres из ком строки
			TCHAR* sCmdPar = _tcsstr(g_zExeInfo[*g_pCurCheck].lpszCmdPar[m_nCountRun - 1], _T("/pres"));
			if (sCmdPar == NULL)	// Если параметра нет, то счетчик уже указывает на след тест, значит выводить сообщение
			{
				MessageDlg.SetStrings(_T("УСТАНОВИТЕ ДАВЛЕНИЕ"), _T("0 кгс/см^2"));
				MessageDlg.DoModal();
			}	
		}; break;

		default:
			break;
	}

	return 0;
}

/////////////////////////////////////////////////////////////////////////////
int CCheckFutApp::GoCheck(int iCheck, int nCmdPar)
{
	BOOL      bRet;
	TCHAR     szNewDir[1024];
	TCHAR*    szExeName;

	GetCurrentDirectory(_countof(m_szOrigDir), m_szOrigDir);
	// Разделение полного пути на имя файла и папки
	_tcscpy_s(szNewDir, g_zExeInfo[iCheck].lpszPath);
	szExeName = _tcsrchr(szNewDir, '\\') + 1;
	szNewDir[_tcsrchr(szNewDir, '\\') - szNewDir] = '\0';

	bRet = SetCurrentDirectory(szNewDir);

	if (bRet == FALSE || RunProgram(szExeName, g_zExeInfo[iCheck].lpszCmdPar[nCmdPar]) == 0)
	{
		g_zExeInfo[iCheck].nResult = 0x5555;			// Не годен
		SetCurrentDirectory(m_szOrigDir);
		return 0;
	}

	return 1;
}

/////////////////////////////////////////////////////////////////////////////
int CCheckFutApp::RunProgram(TCHAR * lpszExePath, TCHAR * lpszCmdLine)
{
	STARTUPINFO		    StartupInfo;
	BOOL                bRet;
	BOOL				bErrorFlag = 0;

	memset(&StartupInfo, NULL, sizeof(STARTUPINFO));
	StartupInfo.cb = sizeof(STARTUPINFO);
	StartupInfo.cbReserved2 = NULL;
	StartupInfo.dwFillAttribute = NULL;
	StartupInfo.dwFlags = STARTF_USESTDHANDLES;

	bRet = CreateProcess(
		lpszExePath,				// pointer to name of executable module
		lpszCmdLine,				// pointer to command line string
		NULL,				        // process security attributes
		NULL,				        // thread security attributes
		FALSE,				        // handle inheritance flag				
		CREATE_DEFAULT_ERROR_MODE,  // creation flags 
		NULL,				        // pointer to new environment block
		NULL, 				        // pointer to current directory name
		&StartupInfo,               // pointer to STARTUPINFO
		&m_ProcessInformation       // pointer to PROCESS_INFORMATION
	);

	if (bRet == FALSE)
	{
		g_pMainSheet->ErrorMessage();
		return 0;
	}

	return 1;
}

/////////////////////////////////////////////////////////////////////////////
int CCheckFutApp::WaitProcessEnd(LONG lCount)
{
	DWORD dwExitCode;
	TCHAR sTime[20];
	Page_List* pPageList = (Page_List*)g_pMainSheet->GetPage(4);
	CProgressCtrl* pProgress = (CProgressCtrl*)g_pMainSheet->GetPage(4)->GetDlgItem(IDC_PROGRESS);
	//CEdit* pTimer = (CEdit*)g_pMainSheet->GetPage(4)->GetDlgItem(IDC_EDIT_TIMER);

	if (g_bProcessEnd == 1)
		TerminateProcess(m_ProcessInformation.hProcess, 0x5555);			// Такой же код как в тестах при их незавершении

	GetExitCodeProcess(m_ProcessInformation.hProcess, &dwExitCode);

	if (dwExitCode == STILL_ACTIVE)
	{
		if (lCount % 10000 == 0)
		{
			GetTimeFrom(sTime);
			g_pMainSheet->GetPage(4)->SetDlgItemText(IDC_EDIT_TIMER, sTime);
			//pPageList->ProgressCorrect(0);

			// Тупая фигня, потому что нет времени придумывать алгоритм отрисовки прогресса для тестов запускаемых несколько раз, удалить
			/*TCHAR sCmd[10];
			if (_tcsstr(g_zExeInfo[*g_pCurCheck].lpszCmdPar[m_nCountRun], _T("/pres")) != NULL)
				_tcscpy_s(sCmd, _tcsstr(g_zExeInfo[*g_pCurCheck].lpszCmdPar[m_nCountRun], _T("/pres")));
			if (_tcscmp(sCmd, _T("/pres0")) == 0)
				pPageList->ProgressCorrect(*m_pSwap, g_iCurProgCheck - 1);
			else if (_tcscmp(sCmd, _T("/pres48")) == 0)
				pPageList->ProgressCorrect(*m_pSwap, g_iCurProgCheck);
			else if (_tcscmp(sCmd, _T("/pres60")) == 0)
			{
				pPageList->ProgressCorrect(*m_pSwap, g_iCurProgCheck + 1);
				m_bDadTest = 1;
			}
			else if (m_bDadTest == 1)
				pPageList->ProgressCorrect(*m_pSwap, g_iCurProgCheck + 2);
			else*/
			
			//if (g_iCurProgCheck - 1 >= 0)!! // какое-то дебильное условие, избавиться от этой переменной
			//pPageList->ProgressCorrect(*m_pSwap, g_iCurProgCheck - 1, g_zExeInfo[*g_pCurCheck].iCountRun - m_nCountRun - 1/*, g_zExeInfo[*g_pCurCheck].iCountRun*/);			// Нормальный вариант
			//if (*m_pSwap != 0)
				pPageList->ProgressCorrect(*m_pSwap);
			//pProgress->StepIt();
		}
		return 0;
	}
	pPageList->IncTestIndex();			// Попытки избавиться от g_iCurProgCheck, отладить
	CloseHandle(m_ProcessInformation.hProcess);
	CloseHandle(m_ProcessInformation.hThread);
	m_ProcessInformation.hProcess = NULL;

	if (m_nOverallRes == 0xffffffff && dwExitCode == 0xffffffff)	// 15.06.2018 тупые колеса, потому что я хочу домой, а если тест запускается 2 раза и идет перемножение 0xffffffff и 0xffffffff происходит переполнение разрядной сетки и получается 1, что означает годен, удалить и написать нормальные коды возвращения в MkioIsaTest
		m_nOverallRes = 0x50;	// Случайное число
	m_nOverallRes *= dwExitCode;
	if (m_nCountRun == 0)		// Если тест уже запускался и больше запускаться не будет, заносить общий результат
		g_zExeInfo[*g_pCurCheck].nResult = m_nOverallRes;

	return 1;
}

/////////////////////////////////////////////////////////////////////////////
int CCheckFutApp::GetTimeFrom(TCHAR * sTime)
{
	//DWORD nCurTime = GetTickCount() - m_nStartTime;
	//int nSec = nCurTime / 1000;
	//div_t res_div = div(nSec, 60);

	//int nCurSec  = nSec - res_div.quot * 60;
	//int nCurMin  = nCurSec / 60;
	//int nCurHour = nCurMin / 60;
	//_stprintf_s(sTime, 20, _T("%02d:%02d:%02d"), nCurHour, nCurMin, nCurSec);

	DWORD nCurTime = GetTickCount() - m_nStartTime;
	int nSec = nCurTime / 1000;
	div_t sec_div = div(nSec, 60);

	int nCurSec = nSec - sec_div.quot * 60;
	div_t min_div = div(sec_div.quot, 60);
	int nCurMin = sec_div.quot - min_div.quot * 60;
	int nCurHour = min_div.quot;
	_stprintf_s(sTime, 20, _T("%02d:%02d:%02d"), nCurHour, nCurMin, nCurSec);

	return 1;
}

void CCheckFutApp::GetTimeStr(TCHAR * sTime)
{
	//SYSTEMTIME time;
	GetLocalTime(&g_zTime);
	_stprintf_s(sTime, 25, _T("%02d.%02d.%d %02d:%02d:%02d"), g_zTime.wDay, g_zTime.wMonth, g_zTime.wYear,
		g_zTime.wHour, g_zTime.wMinute, g_zTime.wSecond);
}

void CCheckFutApp::GetReportName(TCHAR* sReportName)
{
	TCHAR sType[4], sBuf[MAX_PATH];
	CFileFind	finder;
	CString sPath;

	// Чтение рабочей директории
	GetCurrentDirectory(MAX_PATH, sBuf);

	sPath = sBuf;
	sPath += "\\Report";
	// Поиск/создание директории протоколов
	if (finder.FindFile(sPath) == FALSE)
		CreateDirectory(sPath, NULL);

	int nPos = _tcschr(g_zProdTypes[g_zGenInfo.nProduct].szProdTypes, ' ') - g_zProdTypes[g_zGenInfo.nProduct].szProdTypes;
	_tcsncpy_s(sType, g_zProdTypes[g_zGenInfo.nProduct].szProdTypes, nPos);
	_stprintf_s(sReportName, MAX_PATH, _T("%s\\%s - %s - %s.pdf"), _T("Report"), sType, g_zGenInfo.lpszProdNum, g_sTime);
	sReportName[_tcslen(sReportName) - 7] = '.';
	sReportName[_tcslen(sReportName) - 10] = '.';
}

/////////////////////////////////////////////////////////////////////////////
void CCheckFutApp::Result_Dialog_Out()
{
	TCHAR   tcsHead[64];
	int     j = _tcscmp(g_zGenInfo.lpszResult, _T("НЕРАБОТОСПОСОБНОЕ"));
	_stprintf_s(tcsHead, _T("Техническое состояние %s"), g_zProdTypes[g_zGenInfo.nProduct].szProdTypes);

	CResultDlg *pDlg = (CResultDlg*) new class CResultDlg(tcsHead, g_zGenInfo.lpszResult, j);
	pDlg->DoModal();
	delete pDlg;
	return;
}

/////////////////////////////////////////////////////////////////////////////
int CCheckFutApp::ExitInstance()
{
	TCHAR sMes[100];
	BOOL bFlag = TRUE;

	if (m_pSwap != NULL)
		bFlag = ::UnmapViewOfFile(m_pSwap);
	if (m_hSwapping != NULL)
		bFlag = ::CloseHandle(m_hSwapping);

	if (bFlag == 0)
	{
		_stprintf_s(sMes, _T("Ошибка завершения обмена данными с тестами"));
		AfxMessageBox(sMes);
	}

	SystemCmd(_T("taskkill /IM DeviceManagement.exe /T"));

	if (g_pMainSheet != NULL)
		delete g_pMainSheet;
	if (g_pCFG != NULL)
		delete g_pCFG;
	if (m_pSBM != NULL)
		delete m_pSBM;

	return CWinApp::ExitInstance();
}

/////////////////////////////////////////////////////////////////////////////
BOOL CCheckFutApp::OnIdle(LONG lCount)
{
	BOOL bCheck = 0;		// Флаг отмечен ли проверяемый тест

	switch (g_iRun)
	{
		// Не дает грузить ЦП
		case  0: return CWinApp::OnIdle(lCount);
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		case 1:		// Создание объекта SharedBuf
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		{
			if (m_pSBM)
				delete m_pSBM;
			m_pSBM = new CSBManagerMaster(g_pMainSheet);

			TCHAR pszResponse[MAX_PATH] = { 0 };

			if (m_pSBM->SB_Init(pszResponse) != 0)
			{
				AfxMessageBox(pszResponse);
				g_iRun = 0;
				break;
			}

			//--- Запуск потока приема 
			if (m_pSBM->SB_CycleGet(1, pszResponse) != 0)
			{
				AfxMessageBox(pszResponse);
				g_iRun = 0;
				break;
			}

			g_iRun++;
		}; break;
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		case 2:		// Запуск потока ожидания поля CTRL
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		{
			// Запуск ControlPanel
			SystemCmd(_T("taskkill /IM DeviceManagement.exe /T"));
			if (SystemCmd(_T("start /high DeviceManagement.exe -hide")) != 0)
			{
				g_iRun = 201;
				break;
			}

			Sleep(100);
			g_iRun++;
		}; break;
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		case 3:		// Запрос перечня устройств
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		{
			TCHAR pszResponse[MAX_PATH] = { 0 };
			if (m_pSBM->SearchDeviceRequest(pszResponse) != 0)
				AfxMessageBox(pszResponse);
			
			g_iRun = 0;

			//int type_dev = 1; int num_dev = 1; mode_work = 0; int power = 1; float PV = 27; 
			//float pPV_min = 23;	float pPV_max = 30; float PC = 5; float pPC_min = 0; float pPC_max = 7;
			//if (ExcDevices::ExchangeLambda(pSB, type_dev, num_dev, mode_work, power, PV, pPV_min, pPV_max, PC, pPC_min, pPC_max) < 0)
			//{
			//	g_iRun = 202;
			//	break;
			//}

			//// Ждать ответа от программы Саши
			//dwStart = GetTickCount();
			//while (MSB.info.val_stat == 0 && GetTickCount() - dwStart < 2000)
			//	Sleep(100);

			//if (g_iRun == 0)		// Если ошибка приема данных в потоке
			//{
			//	g_iRun = 202;
			//	break;
			//}

		}; break;

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		case 4:			// Анализ подключенных устройств
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		{
			TCHAR sDev[60] = _T("");

			//for (int i = 0; i < _countof(MSB.info.stat); i++)
			//{
			//	if (MSB.info.stat[i].type_dev == 1 && MSB.info.stat[i].num_dev == 1)		// Если устройство - ИП ВИ
			//		g_bDevInfo[i] = MSB.info.stat[i].type_dev;
			//	else
			//		_tcscpy_s(sDev, _T("ИП ВИ\n"));

			//	if (MSB.info.stat[i].type_dev == 1 && MSB.info.stat[i].num_dev == 2)		// Если устройство - ИП АБ
			//		g_bDevInfo[i] = MSB.info.stat[i].type_dev;
			//	else
			//		_tcscpy_s(sDev, _T("ИП АБ\n"));

			//	if (MSB.info.stat[i].type_dev == 1 && MSB.info.stat[i].num_dev == 3)		// Если устройство - ИП Г2
			//		g_bDevInfo[i] = MSB.info.stat[i].type_dev;
			//	else
			//		_tcscpy_s(sDev, _T("ИП Г2\n"));

			//	if (MSB.info.stat[i].type_dev == 2)		// Если устройство - менсор
			//		g_bDevInfo[i] = MSB.info.stat[i].type_dev;
			//	else
			//		_tcscpy_s(sDev, _T("Калибратор давления\n"));

			//	if (MSB.info.stat[i].type_dev == 4)		// Если устройство - генератор
			//		g_bDevInfo[i] = MSB.info.stat[i].type_dev;
			//	else
			//		_tcscpy_s(sDev, _T("Генератор\n"));
			//}

			//if (_tcscmp(sDev, _T("")) != 0)
			//{
			//	TCHAR sMes[MAX_PATH];
			//	_stprintf_s(sMes, _T("Не обнаружены следующие устройства:%s"), sDev);
			//	MessageBox(NULL, sMes, _T("ВНИМАНИЕ!"), MB_OK | MB_ICONINFORMATION);
			//}

			g_iRun++;
		}; break;
		
		case 5:
		{
			// Установка тока на ИП	

			memset(&zPowerSupplyControl, 0, sizeof(_SBM_POWER_SUPPLY));

			zPowerSupplyControl.type = 1;
			zPowerSupplyControl.num = 3;
			zPowerSupplyControl.elem_num = sizeof(_SBM_POWER_SUPPLY) / sizeof(float) - 3; // Количество элементов

			zPowerSupplyControl.mode = 1;
			zPowerSupplyControl.state = 2;
			zPowerSupplyControl.PV = 28;
			zPowerSupplyControl.PC = 15;

			g_pMainSheet->PostMessage(WM_SB_SET_CONTROL, (WPARAM)zPowerSupplyControl.type, (LPARAM)&zPowerSupplyControl);

			Sleep(3000);
			g_iRun = 14;

		}; break;

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		case 14:			// Начальные значения
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		{
			PowerDlg Power;
			if (Power.DoModal() == IDCANCEL)
			{
				Page_List* pPageList = (Page_List*)g_pMainSheet->GetPage(4);
				pPageList->EnableButtons(1, 1);
				//KillThread();
				g_iRun = 0;
				break;
			}
			
			_tcscpy_s(g_zGenInfo.lpszResult, _T("РАБОТОСПОСОБНОЕ"));		// Начальное значение для результата контроля

			if (InitFileMapping() == 0)
			{
				g_iRun = 203;
				break;
			}

			m_nCountRun = g_zExeInfo[m_nTestCount].iCountRun;		// Для первого теста, для остальных задается в case 18 при инкременте m_nTestCount
			m_nOverallRes = 1;
			g_pCurCheck = &g_nTestIndex[0];
		
			m_bDadTest = 0;

			m_nStartTime = GetTickCount();
			g_pMainSheet->ClearResults();
			GetTimeStr(g_sTime);
			g_iRun = 15;
		} break;
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		case 15:			// Запуск теста
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		{
			g_nSetChecks[*g_pCurCheck] = -1;							// Начальное значение для массива отмеченных тестов
			bCheck = ListView_GetCheckState(g_pMainSheet->GetPage(4)->GetDlgItem(IDC_LIST)->GetSafeHwnd(), m_nTestCount);
			if (bCheck > 0 && g_bProcessEnd == 0)
			{
				g_pMainSheet->SetCurTest(m_nTestCount);
				SetModeAndPos();
				if (g_zExeInfo[*g_pCurCheck].nPrevAction > 0)
					PrevActionHandler();
				g_nSetChecks[*g_pCurCheck] = *g_pCurCheck;
				//g_iCurProgCheck++;
				// Для фейка
				GoCheck(*g_pCurCheck, m_nCountRun - 1);			//nCmdPar = g_iCheckCfg == 2 + m_nCountRun - 1
				m_nLastCheck = *g_pCurCheck;
			}
			m_nCountRun--;					// Уменьшение количества запусков текущего теста
			g_iRun++;
		} break;
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		case 16:			// Ожидание окончания процесса с тестом
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		{
			if (m_ProcessInformation.hProcess != NULL && WaitProcessEnd(lCount) == 0)
				break;
			SetCurrentDirectory(m_szOrigDir);
			g_iRun++;
		} break;
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		case 17:			// Запуск события после теста
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		{
			bCheck = ListView_GetCheckState(g_pMainSheet->GetPage(4)->GetDlgItem(IDC_LIST)->GetSafeHwnd(), m_nTestCount);
			if ((bCheck > 0) && (g_zExeInfo[*g_pCurCheck].nPostAction > 0) && g_bProcessEnd == 0)		// Если выбран тест и контроль продолжается
				PostActionHandler();	// то необходимо событие после теста
			g_iRun++;
		}; break;
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		case 18:			// Проверка прошли ли все тесты
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		{
			bCheck = ListView_GetCheckState(g_pMainSheet->GetPage(4)->GetDlgItem(IDC_LIST)->GetSafeHwnd(), m_nTestCount);
			Page_List* pPageList = (Page_List*)g_pMainSheet->GetPage(4);
// !!!!!!!!!!!!!!			
			//pPageList->IncTestIndex();			// Попытки избавиться от g_iCurProgCheck, отладить
			
			// Анализ результата контроля
 			if (g_zExeInfo[*g_pCurCheck].nResult == 0x5555)		// Если аварийная остановка или контроль не проводился
				g_nSetChecks[*g_pCurCheck] = -1;				// Для того, чтобы протокол не включался в общий, иначе подсасывается старый
		
			g_iRun = 15;

			if (m_nCountRun > 0)			// Если текущий тест надо запустить еще раз, то вернуться на g_iRun = 5
			{
				//g_pCurCheck--;			// Тупизм для тестов запускаемых несколько раз, удалить по возможности
				break;
			}

			// Раньше было до g_iRun, перенесено потому что выдавалось неработоспособен при тесте ДАД
			if (/*g_nSetChecks[*g_pCurCheck] > -1 &&*/ bCheck > 0 && g_zExeInfo[*g_pCurCheck].nResult != 1)			// Если результат выбранного теста - не годен
				_tcscpy_s(g_zGenInfo.lpszResult, _T("НЕРАБОТОСПОСОБНОЕ"));

			g_pMainSheet->PrintCheckResult(m_nTestCount);


			if (m_nTestCount < g_nNumTests - 1/*NUMCHECKS - 1*/)			// Если прошли не все тесты, то вернуться на g_iRun = 5 с инкременацией счетчика
			{
				m_nTestCount++;
				g_pCurCheck++;
				m_nCountRun = g_zExeInfo[*g_pCurCheck].iCountRun;
				m_nOverallRes = 1;
			}
			else
				g_iRun = 19;
		}; break;

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		case 19:			// Окно результата проверки
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		{
			Result_Dialog_Out();
			g_iRun++;
		}; break;
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		case 20:		// Создание протокола
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		{
			TCHAR sReportName[MAX_PATH];
			GetReportName(sReportName);
			CreateReport Report;
			Report.ReadPaths();
			Report.ReadFiles();
			if (Report.CreatePdf(sReportName) == 0)
				MessageBox(NULL, _T("Ошибка сохранения файла протокола"), _T("ОШИБКА"), MB_OK | MB_ICONERROR);

			Page_List* pPageList = (Page_List*)g_pMainSheet->GetPage(4);
			pPageList->EnableButtons(1, 0);
			
			//pPageList->ProgressCorrect(*m_pSwap, g_iCurProgCheck - 1);
			//if (g_zExeInfo[m_nLastCheck].iResult == 1)				// Если последний тест ГОДЕН
			//	pPageList->ProgressCorrect(1);						// Установить прогресс на конец

			m_nTestCount = 0;
			g_iRun = 0;

		} break;

		case 201:
			MessageBox(NULL, _T("Ошибка открытия ControlPanel.exe"), _T("ОШИБКА"), MB_OK | MB_ICONERROR);
			g_iRun = 250;
			break;
		case 202:
			MessageBox(NULL, _T("Ошибка обмена данными с приборами"), _T("ОШИБКА"), MB_OK | MB_ICONERROR);
			g_iRun = 250;
			break;
		case 203:
			MessageBox(NULL, _T("Ошибка обмена данными с тестами"), _T("ОШИБКА"), MB_OK | MB_ICONERROR);
			g_iRun = 250;
			break;

		case 250:
		{
			g_iRun = 0;
			Page_List* pPageList = (Page_List*)g_pMainSheet->GetPage(4);
			pPageList->EnableButtons(1, 1);
			break;
		} break;

		default:
			break;
	}

	/*return */CWinApp::OnIdle(lCount);
	return TRUE;
}

//--- CHECK MSG
//int CCheckFutApp::SB_Check_MSG_INFO(_MSG_SHARED_BUF * msg, TCHAR * OUT_pszResponse)
//{
//	// Проверка идентификатора начала и конца 
//	if (msg->info.id_start != 0x4F464E49 || msg->info.id_end != 0x494E464F)
//	{
//		if (OUT_pszResponse) _stprintf_s(OUT_pszResponse, MAX_PATH, _T("SharedBuf-Error! Invalid INFO ID! id_start:%8X id_end:%8X\n"), msg->info.id_start, msg->info.id_end);
//		return 1;
//	}
//	// Проверка версии программы
//	if (msg->info.ver_num_prog < VERSION_PROGRAMM)
//	{
//		if (OUT_pszResponse) _stprintf_s(OUT_pszResponse, MAX_PATH, _T("SharedBuf-Error! Invalid programm version! Control:%d Remote:%d\n"), msg->info.ver_num_prog, VERSION_PROGRAMM);
//		return 2;
//	}
//
//	// Проверка версии протокола
//	if (msg->info.ver_num_prot != VERSION_PROTOCOL)
//	{
//		if (OUT_pszResponse) _stprintf_s(OUT_pszResponse, MAX_PATH, _T("SharedBuf-Error! Invalid protocol version! Control:%d Remote:%d\n"), msg->info.ver_num_prot, VERSION_PROTOCOL);
//		return 3;
//	}
//
//	// Проверка контрольной суммы
//	if (msg->info.check_sum != pSB->GetCheckSum((BYTE*)&msg->info, sizeof(_REG_INFO) - sizeof(uint32) * 2))
//	{
//		if (OUT_pszResponse) _stprintf_s(OUT_pszResponse, MAX_PATH, _T("SharedBuf-Error! Invalid INFO CheckSum %d!\n"), msg->info.check_sum);
//		return 4;
//	}
//
//	if (msg->info.total_buf_sz != sizeof(_MSG_SHARED_BUF))
//	{
//		if (OUT_pszResponse) _stprintf_s(OUT_pszResponse, MAX_PATH, _T("SharedBuf-Error! Invalid SharedBuf size! my_buf:%d msg_buf:%d\n"), sizeof(_MSG_SHARED_BUF), msg->info.total_buf_sz);
//		return 5;
//	}
//
//	CString temp, cs;
//	temp.Format(_T("num_prog:%d num_prot:%d buf_sz:%d val_stat:%d | "), msg->info.ver_num_prog, msg->info.ver_num_prot, msg->info.total_buf_sz, msg->info.val_stat);
//
//	for (int i = 0; i < msg->info.val_stat; i++)
//	{
//		cs.Format(_T("[%d%d%d%d]|"), msg->info.stat[i].type_dev, msg->info.stat[i].num_dev, msg->info.stat[i].power, msg->info.stat[i].reserve);
//		temp.Append(cs);
//	}
//
//	if (OUT_pszResponse) _stprintf_s(OUT_pszResponse, MAX_PATH, _T("%s"), temp.GetString());
//
//	if (msg->info.val_stat == 0) return -1;
//
//	return 0;
//}
//
//int CCheckFutApp::SB_Check_MSG_CTRL(_MSG_SHARED_BUF * msg, TCHAR * OUT_pszResponse)
//{
//	if (msg->ctrl.id_start != 0x4C525443 || msg->ctrl.id_end != 0x4354524C)
//	{
//		if (OUT_pszResponse) _stprintf_s(OUT_pszResponse, MAX_PATH, _T("SharedBuf-Error! Invalid CTRL ID! id_start:%8X id_end:%8X\n"), msg->ctrl.id_start, msg->ctrl.id_end);
//		return 1;
//	}
//
//	// CheckSum
//	if (msg->ctrl.check_sum != pSB->GetCheckSum((BYTE*)&msg->ctrl, sizeof(_REG_CONTROL) - sizeof(uint32) * 2))
//	{
//		if (OUT_pszResponse) _stprintf_s(OUT_pszResponse, MAX_PATH, _T("SharedBuf-Error! Invalid CTRL CheckSum %d!\n"), msg->ctrl.check_sum);
//		return 2;
//	}
//
//	unsigned char *mb = (unsigned char*)&msg->ctrl.control_cmd[0];
//	int TYPE = mb[0];
//
//	_CTRL_CMD_LAMBDAZ *msg_lmbd = (_CTRL_CMD_LAMBDAZ*)msg->ctrl.control_cmd;
//	_CTRL_CMD_MENSOR  *msg_mnsr = (_CTRL_CMD_MENSOR*)msg->ctrl.control_cmd;
//
//	switch (TYPE)
//	{
//		case 0: // Разбор общих сообщений
//			break;
//		case 1: // Разбор сообщения от LambdaZ
//			temp_str.Format(_T("w:MSB:CTRL:dev:%d num:%d work:%d power:%d | PV:%.2f pPV_min:%.2f pPV_max:%.2f | PC:%.2f pPC_min:%.2f pPC_max:%.2f\n"),
//				msg_lmbd->type_dev, msg_lmbd->num_dev, msg_lmbd->mode_work, msg_lmbd->power, msg_lmbd->PV, msg_lmbd->pPV_min, msg_lmbd->pPV_max, msg_lmbd->PC, msg_lmbd->pPC_min, msg_lmbd->pPC_max);
//			//m_textOut.AddString(temp_str);
//			//m_textOut.SetCurSel(m_textOut.GetCount() - 1); // Прокрутка до конца списка
//			AfxMessageBox(temp_str);
//			break;
//
//		case 2: // Разбор сообщения от Mensor
//			temp_str.Format(_T("w:MSB:CTRL:dev:%d num:%d work:%d reserve:%d | PP:%.2f pP_min:%.2f pP_max:%.2f\n"),
//				msg_mnsr->type_dev, msg_mnsr->num_dev, msg_mnsr->mode_work, msg_mnsr->reserve, msg_mnsr->PPressure, msg_mnsr->pPressure_min, msg_mnsr->pPressure_max);
//			//m_textOut.AddString(temp_str);
//			//m_textOut.SetCurSel(m_textOut.GetCount() - 1); // Прокрутка до конца списка
//			AfxMessageBox(temp_str);
//			break;
//
//		default:
//			if (OUT_pszResponse) _stprintf_s(OUT_pszResponse, MAX_PATH, _T("SharedBuf-Error! Invalid DEV TYPE %d!\n"), TYPE); return 3;
//	}
//
//	return 0;
//}
//
//int CCheckFutApp::SB_Check_MSG_ERROR(_MSG_SHARED_BUF * msg, TCHAR * OUT_pszResponse)
//{
//	if (msg->error.id_start != 0x21525245 || msg->error.id_end != 0x45525221)
//	{
//		if (OUT_pszResponse) _stprintf_s(OUT_pszResponse, MAX_PATH, _T("SharedBuf-Error! Invalid ERROR ID! id_start:%8X id_end:%8X\n"), msg->error.id_start, msg->error.id_end);
//		return 1;
//	}
//	// CheckSum
//	if (msg->error.check_sum != pSB->GetCheckSum((BYTE*)&msg->error, sizeof(_REG_ERROR) - sizeof(uint32) * 2))
//	{
//		if (OUT_pszResponse) _stprintf_s(OUT_pszResponse, MAX_PATH, _T("SharedBuf-Error! Invalid ERROR CheckSum %d!\n"), msg->error.check_sum);
//		return 2;
//	}
//
//	if (msg->error.cod_error != 0)
//	{
//		if (OUT_pszResponse) _stprintf_s(OUT_pszResponse, MAX_PATH, _T("error_cod:%d more_cod:%d msg:%s\n"), msg->error.cod_error, msg->error.more_cod_error, msg->error.info_str);
//		return -1;
//	}
//
//	if (OUT_pszResponse)_stprintf_s(OUT_pszResponse, MAX_PATH, _T("error clear\n"));
//
//	return 0;
//}
//
//int CCheckFutApp::SB_Check_MSG_REPLY(_MSG_SHARED_BUF * msg, TCHAR * OUT_pszResponse)
//{
//	if (msg->reply.id_start != 0x4C504552 || msg->reply.id_end != 0x5245504C)
//	{
//		if (OUT_pszResponse) _stprintf_s(OUT_pszResponse, MAX_PATH, _T("SharedBuf-Error! Invalid REPLY ID! id_start:%8X id_end:%8X\n"), msg->ctrl.id_start, msg->ctrl.id_end);
//		return 1;
//	}
//	// CheckSum
//	if (msg->reply.check_sum != pSB->GetCheckSum((BYTE*)&msg->reply, sizeof(_REG_REPLY) - sizeof(uint32) * 2))
//	{
//		if (OUT_pszResponse) _stprintf_s(OUT_pszResponse, MAX_PATH, _T("SharedBuf-Error! Invalid REPLY CheckSum %d!\n"), msg->reply.check_sum);
//		return 2;
//	}
//
//	if (OUT_pszResponse) _stprintf_s(OUT_pszResponse, MAX_PATH, _T("reply clear\n"));
//
//	_REPL_INFO_LAMBDAZ *msg_lmbd;
//	_REPL_INFO_MENSOR  *msg_mnsr;
//	_REPL_INFO_FLUKE *msg_fluke;
//	_REPL_INFO_AKIP *msg_akip;
//
//	unsigned char *mb = (unsigned char*)&msg->reply.reply_info[0];
//	int TYPE = mb[0];
//
//	switch (TYPE)
//	{
//		case 0: // Общие
//			break;
//		case 1: // Разбор сообщения для источника питания
//			msg_lmbd = (_REPL_INFO_LAMBDAZ*)&msg->reply.reply_info[0];
//			_stprintf_s(OUT_pszResponse, MAX_PATH, _T("LAMBDAZ:dev:%d num:%d work:%d power:%d | MV:%.2f pPV_min:%.2f pPV_max:%.2f | MC:%.2f pPC_min:%.2f pPC_max:%.2f\n"),
//				msg_lmbd->type_dev, msg_lmbd->num_dev, msg_lmbd->mode_work, msg_lmbd->power, msg_lmbd->MV, msg_lmbd->pPV_min, msg_lmbd->pPV_max, msg_lmbd->MC, msg_lmbd->pPC_min, msg_lmbd->pPC_max);
//			return -1;
//			break;
//
//		case 2:
//			msg_mnsr = (_REPL_INFO_MENSOR*)&msg->reply.reply_info[0];
//			_stprintf_s(OUT_pszResponse, MAX_PATH, _T("MENSOR:dev:%d num:%d work:%d stable:%d | MV:%.2f pP_min:%.2f pP_max:%.2f\n"),
//				msg_mnsr->type_dev, msg_mnsr->num_dev, msg_mnsr->mode_work, msg_mnsr->stable, msg_mnsr->MPressure, msg_mnsr->pPressure_min, msg_mnsr->pPressure_max);
//			return -1;
//			break;
//
//		case 3:
//			/*msg_fluke = (_REPL_INFO_FLUKE*)&msg->reply.reply_info[0];
//			swprintf(OUT_pszResponse, _T("FLUKE:dev:%d num:%d work:%d stable:%d | MVal:%.2f\n"),
//				msg_fluke->type_dev, msg_fluke->num_dev, msg_fluke->mode_work, msg_fluke->stable, msg_fluke->MVal);
//			if (msg_fluke->mode_work == 1) pPageFluke->SetMeasRes(msg_fluke->stable, msg_fluke->MVal);
//			if (msg_fluke->mode_work == 2) pPageFluke->SetMeasCont(msg_fluke->stable, msg_fluke->MVal);*/
//			return -1;
//			break;
//
//		case 4:
//			/*msg_akip = (_REPL_INFO_AKIP*)&msg->reply.reply_info[0];
//			swprintf(OUT_pszResponse, _T("AKIP:dev:%d num:%d work:%d power:%d | freq:%.3f amp:%.3f\n"),
//				msg_akip->type_dev, msg_akip->num_dev, msg_akip->mode_work, msg_akip->power, msg_akip->freq, msg_akip->amp);
//			pPageAkip->SetGetFreq(msg_akip->freq);
//			pPageAkip->SetGetAmp(msg_akip->amp);*/
//			return -1;
//			break;
//
//		default:
//			if (OUT_pszResponse) _stprintf_s(OUT_pszResponse, MAX_PATH, _T("SharedBuf-Error! Invalid DEV TYPE %d!\n"), TYPE); return 3;
//	}
//
//	return 0;
//}