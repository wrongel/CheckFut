
/*		Версия 1.3.1.0. от 31.10.2019

	1. Добавлен новый режим, в контейнере. 3 бит g_nMode, в связи с чем изменен класс Page_ProdInfo и соответствующее диалоговое окно.
	Режим доступен только для МА.
	2. В связи с добавлением нового режима со своим набором тестов изменен config.dat.
	3. Удалено поле szProdName[15] из структуры tagProdTypes в CheckFut.hpp и CheckFut.hxx за ненадобностью. Устарело, когда внес тип в название тестов
	4. Добавлено поле тип хранения lpszStorage в g_zGenInfo и соответствующая строка в шапке протокола
	5. Исправлена ошибка при исправлении файла конфигурации из окна Settings. Раньше удалялись префиксы названий тестов.
	6. Добавлена иконка IDI_ICON_HEAD и соответствующее поле HICON в MainSheet
	7. Добавлено обнуление 3 бита g_nMode в PrevActionHandler case 3 для корректной передачи режима в тесты команд и признаков и напряжения
	8. Исправленный ReportCtrl и соответствующие правки в SetColumnHeader во всех таблицах

		Версия 1.3.2.0. от 11.11.2019
	1. В PrevActionHandler добавлена передача места проведения работ. Связано с разными окнами в тесте напряжений.
	   Пока принято решение не трогать config, подумать надо ли это. Подумать как передавать во все тесты.

	    Версия 1.3.3.0. от 18.11.2019
	1. Добавлен case 0 в OnIdle для снижения нагрузки на процессор при простое.
	2. Добавлен тест высоких напряжений для МА и БЭ.
	3. Добавлена функция SetModeAndPos() для задания параметров /pos и /tag. Раньше для этого использовалось case 3 в PrevActionHandler().
	4. Case 3 в PrevActionHandler() теперь используется для выдачи окна перед тестом высокого напряжения.

		Версия 1.3.3.1. от 10.01.2019
	1. Расширен буфер для проверки отсутствующих тестов до 1024 в OnBnClickedButtonStart() из-за увеличения количества тестов.
*/


// CheckFut.h : главный файл заголовка для приложения PROJECT_NAME
//

#pragma once

#ifndef __AFXWIN_H__
	#error "включить stdafx.h до включения этого файла в PCH"
#endif

#include "resource.h"		// основные символы
#include "SharedBuffer.h"
#include "struct.h"

// CCheckFutApp:
// О реализации данного класса см. CheckFut.cpp
//

class CCheckFutApp : public CWinApp
{
public:
	CCheckFutApp();

// Переопределение
public:
	virtual BOOL InitInstance();
	int GetAppVersion(TCHAR* pProductVersion);					// Получить номер версии программы
	int CheckExeExist(TCHAR* sCheckNames, int nBufSize);		// Проверка exe-файлов на существование

	int SetControlMsg(LPARAM lParam);			// Задача управляющего поля
	int GetReplyMsg();							// Прием ответных полей


private:
	int   m_nLastCheck;							// Индекст последнего проведенного теста
	int   m_nCountRun;							// Количество оставшихся запусков текущего теста
	int   m_nOverallRes;						// Результат теста, запущенного несколько раз
	PROCESS_INFORMATION m_ProcessInformation;	// Информация для процесса запуска теста
	TCHAR m_szOrigDir[1024];					// Директория для основного процесса
	DWORD m_nStartTime;							// Начальная точка для таймера

	// Информация для обмена данными между тестами и верхней программой
	HANDLE	m_hSwapping;						// Handle на объект FileMapping
	CString m_sSwapName = L"DataSwapZone";		// Имя объекта, по которому программы получают доступ к виртуальной памяти
	int     m_nSwapSize = 4;					// Размер передаваемых данных (float)
	float*  m_pSwap;							// Указатель на начальный адрес виртуального адресного пространства
	// После раскомментирования раскомментировать конструктор, деструктор, case 1 и поиск m_pSwap

	int  m_nTestCount;	// Счетчик тестов отрисованных в окне
	BOOL m_bDadTest;	// Флаг прохождения теста датчиков давления, удалить как только придумаю нормальный алгоритм отрисовки прогресса для тестов запускаемых несколько раз

	// Параметры для обмена с Сашей
	CSBManagerMaster *m_pSBM;
	_SBM_POWER_SUPPLY zPowerSupplyControl;		// Собственная структура управляющих параметров 

	DWORD SystemCmd(const TCHAR * strFunct, const TCHAR * strParams = NULL);	// Запуск программы командной строкой

	// Контрольная сумма
	UINT GetCheckSum(char *buf, int buf_sz);

	int InitFileMapping();
	void SetModeAndPos();					// Передать в командную строку теста параметр /tag - режим, /pos - место проведения, если они есть
	int PrevActionHandler();
	int PostActionHandler(int ActionInfo = 0);
	int GoCheck(int iCheck, int nCmdPar);				// Запуск теста
	int RunProgram(TCHAR* lpszExePath, TCHAR* lpszCmdLine);		// Создание процесса с тестом
	int WaitProcessEnd(LONG lCount);		// Ждать окончания процесса и обновлять прогрессбар с таймером
	int GetTimeFrom(TCHAR* sTime);			// Получение текущего времени для таймера
	void GetTimeStr(TCHAR* sTime);			// Получить строковое предствление времени
	void GetReportName(TCHAR* sReportName);	// Получить имя протокола на основе времени начала контроля

	void    Result_Dialog_Out();			// Вывод окна с общим результатом контроля
// Реализация

	DECLARE_MESSAGE_MAP()
	virtual int ExitInstance();
	virtual BOOL OnIdle(LONG lCount);

	//int SB_Check_MSG_INFO(_MSG_SHARED_BUF *msg, TCHAR *OUT_pszResponse = NULL);
	//int SB_Check_MSG_CTRL(_MSG_SHARED_BUF *msg, TCHAR *OUT_pszResponse = NULL);
	//int SB_Check_MSG_ERROR(_MSG_SHARED_BUF *msg, TCHAR *OUT_pszResponse = NULL);
	//int SB_Check_MSG_REPLY(_MSG_SHARED_BUF *msg, TCHAR *OUT_pszResponse = NULL);
};

extern CCheckFutApp theApp;