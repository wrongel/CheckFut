
#include "Const.h"

class MainSheet* g_pMainSheet;		// Указатель на объект главного окна
class Config*       g_pCFG;			// Указатель на объект конфигурации

int g_nMode = 0;					// Режим работы программы
/*           
		0x1 - Контроль МА
		0x2 - Контроль БЭ
		0x4 - Контроль МКУ 
		0x8 - В контейнере, доступто только для МА
*/
int g_iRun = 0;
int g_nNumTests;				// Количество тестов для текущего режима, когда все будет готов сделать жесткую привязку в конфиге, режим - колво тестов, сейчас постоянно считается в Page_List::PrintCheckNames()
int g_nTestIndex[NUMCHECKS];	// Массив индексов тестов, доступный в текущем режиме
int* g_pCurCheck;				// Общий номер теста (из всех возможных во всех режимах)

//int g_iCurProgCheck = 0;			// Индекс выбранной программы для ProgressBar, Рассмотреть возможность избавления от этой переменной или переноса ее в app.cpp
int g_nSetChecks[NUMCHECKS];		// Массив индексов отмеченных тестов
TCHAR g_sVersFromConfig[15];		// Версия программы из файла конфигурации
BOOL g_bProcessEnd;					// Флаг остановки процесса с текущим тестом
TCHAR g_sReportPath[MAX_PATH];		// Путь к общему файлу протокола
SYSTEMTIME g_zTime;					// Структура с временем начала контроля
TCHAR g_sTime[25];					// Тектовая строка с датой и временем начала контроля

double g_dfU;				// Напрядение питания
double g_dfI;				// Ток потребления
BOOL   g_bDevInfo[5] = {	// Массив флагов подключены ли устройства, 3 ИП, генератор и менсор
	0, 0, 0, 0, 0 };
// Типы изделий
struct tagProdTypes
{
	TCHAR szProdTypes[20];			// Сокращенное название
	TCHAR szProdExt[128];			// Полное название
};
struct tagProdTypes g_zProdTypes[3] = {
	{ _T("МА 2534М"),			 _T("модуля аппаратурного изделия 2534М") },
	{ _T("БЭ ШФИГ.778215.011"),  _T("блока электронного ШФИГ.778215.011") },
	{ _T("МКУ 2534М"),			 _T("модуля контроллера управления изделия 2534М") }
};

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
struct tagEXEINFO g_zExeInfo[NUMCHECKS];

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
struct tagGenInfo g_zGenInfo;