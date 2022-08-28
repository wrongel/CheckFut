
#include "Const.h"

extern class MainSheet* g_pMainSheet;		// Указатель на объект главного окна
extern class Config*       g_pCFG;			// Указатель на объект конфигурации

extern int g_nMode;							// Режим работы программы
extern int g_iRun;
extern int g_nTestIndex[NUMCHECKS];
extern int g_nNumTests;						// Количество тестов для текущего режима
extern int* g_pCurCheck;					// Общий номер теста (из всех возможных во всех режимах)

//extern int g_iCurProgCheck;					// Индекс выбранной программы для ProgressBar
extern int g_nSetChecks[NUMCHECKS];			// Массив индексов отмеченных тестов
extern TCHAR g_sVersFromConfig[15];			// Версия программы из файла конфигурации
extern BOOL g_bProcessEnd;					// Флаг остановки процесса с текущим тестом
extern TCHAR g_sReportPath[MAX_PATH];		// Путь к общему файлу протокола
extern SYSTEMTIME g_zTime;					// Структура с временем начала контроля
extern TCHAR g_sTime[25];					// Тектовая строка с датой и временем начала контроля

extern double g_dfU;			// Напрядение питания
extern double g_dfI;			// Ток потребления
extern BOOL   g_bDevInfo[5];	// Массив флагов подключены ли устройства, 3 ИП, генератор и менсор

// Типы изделий
struct tagProdTypes
{
	TCHAR szProdTypes[20];			// Сокращенное название
	TCHAR szProdExt[128];			// Полное название
};
extern struct tagProdTypes g_zProdTypes[3];

// Информация о тесте
struct tagEXEINFO
{
	TCHAR  lpszCheckName[256];							// Название теста
	TCHAR  lpszPath[1024];								// Путь
	TCHAR  lpszCmdPar[5][1024];							// Параметры командной строки
	TCHAR  lpszRepName[1024];							// Путь к протоколу теста
	int    nResult;										// Результат контроля 1 - годен, остальное - не годен
	int    iCountRun;									// Количество запусков теста
	int    nMode;										// Режим, в котором используется тест
	int    nPrevAction;									// Действие перед запуском теста
	int    nPostAction;									// Действие после запуска теста
};
extern struct tagEXEINFO g_zExeInfo[NUMCHECKS];

// Информация о контроле
struct tagGenInfo
{
	int   nPosition;									// Индекс места проведения
	TCHAR lpszPosition[256];							// Название места проведения
	TCHAR lpszStorage[256];								// Тип хранения
	TCHAR lpszOperator[256];							// ФИО оператора
	TCHAR lpszOtkMan[256];								// ФИО представителя ОТК
	TCHAR lpszVpMan[256];								// ФИО представителя ВП
	DWORD nProduct;										// Индекс типа изделия
	TCHAR lpszProdNum[256];								// Номер изделия
	TCHAR lpszResult[25];								// Общий результат контроля
};
extern struct tagGenInfo g_zGenInfo;