
#include "Const.h"

extern class MainSheet* g_pMainSheet;		// ��������� �� ������ �������� ����
extern class Config*       g_pCFG;			// ��������� �� ������ ������������

extern int g_nMode;							// ����� ������ ���������
extern int g_iRun;
extern int g_nTestIndex[NUMCHECKS];
extern int g_nNumTests;						// ���������� ������ ��� �������� ������
extern int* g_pCurCheck;					// ����� ����� ����� (�� ���� ��������� �� ���� �������)

//extern int g_iCurProgCheck;					// ������ ��������� ��������� ��� ProgressBar
extern int g_nSetChecks[NUMCHECKS];			// ������ �������� ���������� ������
extern TCHAR g_sVersFromConfig[15];			// ������ ��������� �� ����� ������������
extern BOOL g_bProcessEnd;					// ���� ��������� �������� � ������� ������
extern TCHAR g_sReportPath[MAX_PATH];		// ���� � ������ ����� ���������
extern SYSTEMTIME g_zTime;					// ��������� � �������� ������ ��������
extern TCHAR g_sTime[25];					// �������� ������ � ����� � �������� ������ ��������

extern double g_dfU;			// ���������� �������
extern double g_dfI;			// ��� �����������
extern BOOL   g_bDevInfo[5];	// ������ ������ ���������� �� ����������, 3 ��, ��������� � ������

// ���� �������
struct tagProdTypes
{
	TCHAR szProdTypes[20];			// ����������� ��������
	TCHAR szProdExt[128];			// ������ ��������
};
extern struct tagProdTypes g_zProdTypes[3];

// ���������� � �����
struct tagEXEINFO
{
	TCHAR  lpszCheckName[256];							// �������� �����
	TCHAR  lpszPath[1024];								// ����
	TCHAR  lpszCmdPar[5][1024];							// ��������� ��������� ������
	TCHAR  lpszRepName[1024];							// ���� � ��������� �����
	int    nResult;										// ��������� �������� 1 - �����, ��������� - �� �����
	int    iCountRun;									// ���������� �������� �����
	int    nMode;										// �����, � ������� ������������ ����
	int    nPrevAction;									// �������� ����� �������� �����
	int    nPostAction;									// �������� ����� ������� �����
};
extern struct tagEXEINFO g_zExeInfo[NUMCHECKS];

// ���������� � ��������
struct tagGenInfo
{
	int   nPosition;									// ������ ����� ����������
	TCHAR lpszPosition[256];							// �������� ����� ����������
	TCHAR lpszStorage[256];								// ��� ��������
	TCHAR lpszOperator[256];							// ��� ���������
	TCHAR lpszOtkMan[256];								// ��� ������������� ���
	TCHAR lpszVpMan[256];								// ��� ������������� ��
	DWORD nProduct;										// ������ ���� �������
	TCHAR lpszProdNum[256];								// ����� �������
	TCHAR lpszResult[25];								// ����� ��������� ��������
};
extern struct tagGenInfo g_zGenInfo;