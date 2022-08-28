
#include "Const.h"

class MainSheet* g_pMainSheet;		// ��������� �� ������ �������� ����
class Config*       g_pCFG;			// ��������� �� ������ ������������

int g_nMode = 0;					// ����� ������ ���������
/*           
		0x1 - �������� ��
		0x2 - �������� ��
		0x4 - �������� ��� 
		0x8 - � ����������, �������� ������ ��� ��
*/
int g_iRun = 0;
int g_nNumTests;				// ���������� ������ ��� �������� ������, ����� ��� ����� ����� ������� ������� �������� � �������, ����� - ����� ������, ������ ��������� ��������� � Page_List::PrintCheckNames()
int g_nTestIndex[NUMCHECKS];	// ������ �������� ������, ��������� � ������� ������
int* g_pCurCheck;				// ����� ����� ����� (�� ���� ��������� �� ���� �������)

//int g_iCurProgCheck = 0;			// ������ ��������� ��������� ��� ProgressBar, ����������� ����������� ���������� �� ���� ���������� ��� �������� �� � app.cpp
int g_nSetChecks[NUMCHECKS];		// ������ �������� ���������� ������
TCHAR g_sVersFromConfig[15];		// ������ ��������� �� ����� ������������
BOOL g_bProcessEnd;					// ���� ��������� �������� � ������� ������
TCHAR g_sReportPath[MAX_PATH];		// ���� � ������ ����� ���������
SYSTEMTIME g_zTime;					// ��������� � �������� ������ ��������
TCHAR g_sTime[25];					// �������� ������ � ����� � �������� ������ ��������

double g_dfU;				// ���������� �������
double g_dfI;				// ��� �����������
BOOL   g_bDevInfo[5] = {	// ������ ������ ���������� �� ����������, 3 ��, ��������� � ������
	0, 0, 0, 0, 0 };
// ���� �������
struct tagProdTypes
{
	TCHAR szProdTypes[20];			// ����������� ��������
	TCHAR szProdExt[128];			// ������ ��������
};
struct tagProdTypes g_zProdTypes[3] = {
	{ _T("�� 2534�"),			 _T("������ ������������� ������� 2534�") },
	{ _T("�� ����.778215.011"),  _T("����� ������������ ����.778215.011") },
	{ _T("��� 2534�"),			 _T("������ ����������� ���������� ������� 2534�") }
};

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
struct tagEXEINFO g_zExeInfo[NUMCHECKS];

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
struct tagGenInfo g_zGenInfo;