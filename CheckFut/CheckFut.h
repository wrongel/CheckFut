
/*		������ 1.3.1.0. �� 31.10.2019

	1. �������� ����� �����, � ����������. 3 ��� g_nMode, � ����� � ��� ������� ����� Page_ProdInfo � ��������������� ���������� ����.
	����� �������� ������ ��� ��.
	2. � ����� � ����������� ������ ������ �� ����� ������� ������ ������� config.dat.
	3. ������� ���� szProdName[15] �� ��������� tagProdTypes � CheckFut.hpp � CheckFut.hxx �� �������������. ��������, ����� ���� ��� � �������� ������
	4. ��������� ���� ��� �������� lpszStorage � g_zGenInfo � ��������������� ������ � ����� ���������
	5. ���������� ������ ��� ����������� ����� ������������ �� ���� Settings. ������ ��������� �������� �������� ������.
	6. ��������� ������ IDI_ICON_HEAD � ��������������� ���� HICON � MainSheet
	7. ��������� ��������� 3 ���� g_nMode � PrevActionHandler case 3 ��� ���������� �������� ������ � ����� ������ � ��������� � ����������
	8. ������������ ReportCtrl � ��������������� ������ � SetColumnHeader �� ���� ��������

		������ 1.3.2.0. �� 11.11.2019
	1. � PrevActionHandler ��������� �������� ����� ���������� �����. ������� � ������� ������ � ����� ����������.
	   ���� ������� ������� �� ������� config, �������� ���� �� ���. �������� ��� ���������� �� ��� �����.

	    ������ 1.3.3.0. �� 18.11.2019
	1. �������� case 0 � OnIdle ��� �������� �������� �� ��������� ��� �������.
	2. �������� ���� ������� ���������� ��� �� � ��.
	3. ��������� ������� SetModeAndPos() ��� ������� ���������� /pos � /tag. ������ ��� ����� �������������� case 3 � PrevActionHandler().
	4. Case 3 � PrevActionHandler() ������ ������������ ��� ������ ���� ����� ������ �������� ����������.

		������ 1.3.3.1. �� 10.01.2019
	1. �������� ����� ��� �������� ������������� ������ �� 1024 � OnBnClickedButtonStart() ��-�� ���������� ���������� ������.
*/


// CheckFut.h : ������� ���� ��������� ��� ���������� PROJECT_NAME
//

#pragma once

#ifndef __AFXWIN_H__
	#error "�������� stdafx.h �� ��������� ����� ����� � PCH"
#endif

#include "resource.h"		// �������� �������
#include "SharedBuffer.h"
#include "struct.h"

// CCheckFutApp:
// � ���������� ������� ������ ��. CheckFut.cpp
//

class CCheckFutApp : public CWinApp
{
public:
	CCheckFutApp();

// ���������������
public:
	virtual BOOL InitInstance();
	int GetAppVersion(TCHAR* pProductVersion);					// �������� ����� ������ ���������
	int CheckExeExist(TCHAR* sCheckNames, int nBufSize);		// �������� exe-������ �� �������������

	int SetControlMsg(LPARAM lParam);			// ������ ������������ ����
	int GetReplyMsg();							// ����� �������� �����


private:
	int   m_nLastCheck;							// ������� ���������� ������������ �����
	int   m_nCountRun;							// ���������� ���������� �������� �������� �����
	int   m_nOverallRes;						// ��������� �����, ����������� ��������� ���
	PROCESS_INFORMATION m_ProcessInformation;	// ���������� ��� �������� ������� �����
	TCHAR m_szOrigDir[1024];					// ���������� ��� ��������� ��������
	DWORD m_nStartTime;							// ��������� ����� ��� �������

	// ���������� ��� ������ ������� ����� ������� � ������� ����������
	HANDLE	m_hSwapping;						// Handle �� ������ FileMapping
	CString m_sSwapName = L"DataSwapZone";		// ��� �������, �� �������� ��������� �������� ������ � ����������� ������
	int     m_nSwapSize = 4;					// ������ ������������ ������ (float)
	float*  m_pSwap;							// ��������� �� ��������� ����� ������������ ��������� ������������
	// ����� ������������������ ����������������� �����������, ����������, case 1 � ����� m_pSwap

	int  m_nTestCount;	// ������� ������ ������������ � ����
	BOOL m_bDadTest;	// ���� ����������� ����� �������� ��������, ������� ��� ������ �������� ���������� �������� ��������� ��������� ��� ������ ����������� ��������� ���

	// ��������� ��� ������ � �����
	CSBManagerMaster *m_pSBM;
	_SBM_POWER_SUPPLY zPowerSupplyControl;		// ����������� ��������� ����������� ���������� 

	DWORD SystemCmd(const TCHAR * strFunct, const TCHAR * strParams = NULL);	// ������ ��������� ��������� �������

	// ����������� �����
	UINT GetCheckSum(char *buf, int buf_sz);

	int InitFileMapping();
	void SetModeAndPos();					// �������� � ��������� ������ ����� �������� /tag - �����, /pos - ����� ����������, ���� ��� ����
	int PrevActionHandler();
	int PostActionHandler(int ActionInfo = 0);
	int GoCheck(int iCheck, int nCmdPar);				// ������ �����
	int RunProgram(TCHAR* lpszExePath, TCHAR* lpszCmdLine);		// �������� �������� � ������
	int WaitProcessEnd(LONG lCount);		// ����� ��������� �������� � ��������� ����������� � ��������
	int GetTimeFrom(TCHAR* sTime);			// ��������� �������� ������� ��� �������
	void GetTimeStr(TCHAR* sTime);			// �������� ��������� ������������ �������
	void GetReportName(TCHAR* sReportName);	// �������� ��� ��������� �� ������ ������� ������ ��������

	void    Result_Dialog_Out();			// ����� ���� � ����� ����������� ��������
// ����������

	DECLARE_MESSAGE_MAP()
	virtual int ExitInstance();
	virtual BOOL OnIdle(LONG lCount);

	//int SB_Check_MSG_INFO(_MSG_SHARED_BUF *msg, TCHAR *OUT_pszResponse = NULL);
	//int SB_Check_MSG_CTRL(_MSG_SHARED_BUF *msg, TCHAR *OUT_pszResponse = NULL);
	//int SB_Check_MSG_ERROR(_MSG_SHARED_BUF *msg, TCHAR *OUT_pszResponse = NULL);
	//int SB_Check_MSG_REPLY(_MSG_SHARED_BUF *msg, TCHAR *OUT_pszResponse = NULL);
};

extern CCheckFutApp theApp;