#pragma once

#include <stdint.h>
#include <vector>

#define VERSION_SHARED_BUFFER 101

// ������� ��� WindowProc(UINT message, WPARAM wParam, LPARAM lParam)
const UINT WM_SB_CYCLE_GET_MSG = WM_APP + 1000; // ������ ��������� ��������� � �����

//--- ���������� �������� ����� ��� ����� ������
#pragma pack(1)

//--- �������� ���������� | size:6
typedef struct _SBM_PARAM
{
	int   num;					// �����
	float value;				// ��������
	_SBM_PARAM(int number=0, float val=0) : num(number), value(val) {} // �����������
} _SBM_PARAM;

typedef struct
{
	uint8_t type;
	uint8_t num;
	uint8_t mode;
	uint8_t state;
	uint16_t control_offset;
	uint16_t control_size;
	uint16_t reply_offset;
	uint16_t reply_size;
} _SBM_DEVICE;

//--- ������ ���������� | size:8
typedef struct
{
	uint8_t	 type;				// ��� ����������
	uint8_t	 num;				// ����� ����������
	uint8_t	 mode;				// ����� ������
	uint8_t	 state;				// ��������� (connect, power � ��.)
	uint16_t control_size;		// ������ ���� ���������� � ������
	uint16_t reply_size;		// ������ ���� ������ � ������
} _SB_DEV_STATUS;

//--- ��������� ����� | size:24
typedef struct
{
	uint32_t id_header;			// ������������� ��������� / 0x52444548 / "HEDR"
	uint16_t version_prot;		// ������ ���������
	uint16_t global_size;		// ����� ������
	uint8_t	 change_msg;		// ��� ��������� ����������� ����	"1-CONTROL 2-INFO 3-REPLY 4-ERROR"
	_SB_DEV_STATUS dev;			// ���������� ���������� ������
	int8_t   reserve[3];		// ������
	uint32_t check_sum;			// ����������� �����
} _SB_HEADER;

//--- ����������� ���� | size:8200
typedef struct
{
	uint32_t id_control;		// ������������� ����������	/ 0x4C525443 / �CTRL�
	uint8_t  BUF[8192];			// ���� ���� ��� ����������� ������
	uint32_t check_sum;			// ����������� �����
} _SB_CONTROL;

//--- �������������� ���� | size:500
typedef struct
{
	uint32_t id_info;			// ������������� ���������� / 0x4F464E49 / �INFO�
	uint16_t number_dev;		// ���������� ���������
	_SB_DEV_STATUS DEV_STAT[60];// ������ �������� �������� ���������
	int8_t   reserve[10];		// ������
	uint32_t check_sum;			// ����������� �����
} _SB_INFO;

//--- �������� ���� | size:8200
typedef struct
{
	uint32_t id_reply;			// ������������� ������ / 0x4C504552 / �REPL�
	uint8_t  BUF[8192];			// ���� ���� ��� �������� ������
	uint32_t check_sum;			// ����������� �����
} _SB_REPLY;

//--- ���� ������ | size:1040
typedef struct
{
	uint32_t id_error;			// ������������� ������ / 0x21525245 / �ERR!�
	uint8_t type;				// ��� ����������
	uint8_t	num;				// ����� ����������
	int8_t error_cod;			// ��� ������
	int8_t error_more;			// ���.��� ������
	TCHAR error_str[512];		// ������ ������
	int8_t reserve[4];			// ������
	uint32_t check_sum;			// ����������� �����
} _SB_ERROR;

//--- ����� ��������� ����� | size:17964 
typedef struct
{
	_SB_HEADER  hdr;			// ���� ���������
	_SB_CONTROL control;		// ���� ����������� ������
	_SB_INFO    info;			// ���� ���������� �� �����������
	_SB_REPLY   reply;			// ���� �������� ������
	_SB_ERROR   error;			// ���� ������
} _SB_GLOBAL;

//--- ��������
typedef struct
{
	int hdr = 0;
	int control = sizeof(_SB_HEADER);
	int info = sizeof(_SB_HEADER) + sizeof(_SB_CONTROL);
	int reply = sizeof(_SB_HEADER) + sizeof(_SB_CONTROL) + sizeof(_SB_INFO);
	int error = sizeof(_SB_HEADER) + sizeof(_SB_CONTROL) + sizeof(_SB_INFO) + sizeof(_SB_REPLY);
} _SB_OFFSET;

#pragma pack(8)

//--- ����������� ����� CSharedBuffer ��� ������ � ����� �������
class CSharedBuffer
{
public:
	CSharedBuffer(CWnd *hostWindow); // ����������� ������
	~CSharedBuffer(); // ���������� ������

	// >INITIALIZATION - �������������
	// >������� �������� 
	// byte_size - ������ ����������������� ������ � ������
	// pszUnicName - ���������� ������� � ���������� ������ SharedMemory ��� �������� ������ ���������� ������
	// >�������� ��������
	// OUT_pszResponse - ����� ���������� (����� ���� NULL)
	// >return
	// 0 - ������������� ������ �������
	// 1 - �� ������� ���������������� ����� ��� FileMap
	// 2 - �� ������� ���������������� ����� �� SharedMemory
	// 3 - �� ������� ������� mutex ��� ���������� ��������� � ������
	int Init(DWORD byte_size, TCHAR *OUT_pszResponse = NULL); // �������������

	// >DESTROY - �����������
	void Destroy(); // �����������

	// >RESET BUF - ����� ����� ������ ������
	// >�������� ��������
	// OUT_pszResponse - ����� (����� ���� NULL)
	// >return
	// 0 - ����� ����������
	// 1 - ����� ������ �� ����������������
	// 2 - ���������� �������� ����� ���� � ����� ������
	int ResetBuf(TCHAR *OUT_pszResponse = NULL); // ����� ����������

	// >SET BUF - ����������� �������� ������ � SharedMemory
	// >������� �������� 
	// IN_buf - ��������� ������, ������� ����� ���������� � SharedMemory
	// byte_offset - �������� � ������ �� ������ SharedMemory
	// byte_size - ������ ����������������� ������ � ������
	// >�������� ��������
	// OUT_pszResponse - ����� ���������� (����� ���� NULL)
	// >return
	// 0 - ����������� ������ �������
	// 1 - ���������� ����� �� ���������������
	// 2 - �� ���� ������� ������������� SharedMemory �������� Init(...)
	// 3 - ������ ���������� ������ ����� ����
	// 4 - ����� �� ������� ���������� ������ SharedMemory. byte_size+byte_offset > m_Buf_size
	// 5 - �������� mutex ����������� � �������
	int SetBuf(BYTE *IN_buf, DWORD byte_offset, DWORD byte_size, TCHAR *OUT_pszResponse = NULL);

	// >UPDATE BUF - �������� ����� ���������� ������
	void UpdateBuf();

	// >GET BUF - ����������� �������� �� SharedMemory � �����
	// >������� �������� 
	// byte_offset - �������� � ������ �� ������ SharedMemory
	// byte_size - ������ ����������������� ������ � ������
	// >�������� ��������
	// OUT_buf - ��������� ������, � ������� ����� ���������� SharedMemory
	// OUT_pszResponse - ����� ���������� (����� ���� NULL)
	// >return
	// 0 - ����������� ������ �������
	// 1 - ����� ���������� �� ���������������
	// 2 - �� ���� ������� ������������� SharedMemory �������� Init(...)
	// 3 - ������ ���������� ������ ����� ����
	// 4 - ����� �� ������� ���������� ������ SharedMemory. byte_size+byte_offset > m_Buf_size
	// 5 - �������� mutex ����������� � �������
	int GetBuf(BYTE *OUT_buf, DWORD byte_offset, DWORD byte_size, TCHAR *OUT_pszResponse = NULL);

	// >GET STATUS INIT - ���������� ������ ������������� SharedBuf
	// >return
	// 0 - ������������� �� �����������
	// 1 - ������������� �����������
	BOOL GetStatusInit() { return m_StatInit; }

	// >GET CHECK SUM - ���������� ������ ����������� ����� (����� ����)
	// >������� �������� 
	// buf - ��������� �� �����
	// buf_sz - ������ ������ ��� ������� �����
	// >return
	// UINT - ��������� ����������� ����� ����� �����
	UINT GetCheckSum(BYTE *buf, size_t buf_sz);

	// THREAD
	int StartCycleGet(); // ������ ������
	void ClearThread(); // ���������� ������

private:
	static UINT Thread(LPVOID pParam); // ������� ������
	void Run(); // ������� ������������ ������ ������
	
	CEvent *m_EventExit; // ������� ������ �� ������ ��������
	CEvent *m_EventGetMsg; // ������� ������ ���������
	CEvent *m_EventMySending; // ������� ��� �� �� � ��������� �������� ���������
	CWinThread *m_pThread; // ��������� �� �����
	CWnd *m_hostWindow; // ��������� �� host ����
	
	HANDLE hMapFile;			// ����� ����� ��������
	HANDLE hMutex;				// ����� ��������
	BOOL m_StatInit;			// ���� ������� ������������� (����� Init(...), ����� Destroy())
	BYTE *pBuf;					// ��������� �� ����� SharedMemory
	DWORD m_Buf_size;			// ������ ������ SharedMemory
};

//--- MASTER
//--- ����� ���������� �� ������� Master
class CSBManagerMaster
{
public:
	CSBManagerMaster(CWnd * host); // �����������
	~CSBManagerMaster(); // ����������

	//--- SharedBuffer
	int SB_Init(TCHAR *OUT_pszResponse);	// ������������� SharedBuffer
	int SB_Reset(TCHAR *OUT_pszResponse);   // ����� SharedBuffer
	int SB_CycleGet(bool start, TCHAR *OUT_pszResponse); // ������ ������ ������ ��������� �� SharedBuffer

	//--- ����������
	int SearchDeviceRequest(TCHAR *OUT_pszResponse = NULL); // ��������� ������� �� ����� ������������ ���������
	int InitDeviceInfo(TCHAR *OUT_pszResponse); // ���������������� ������ ���������
	int GetDevStatus(_SB_DEV_STATUS *dev, int &numDev, int type, int num, int mode, int state, TCHAR *OUT_pszResponse); // �������� ��������� � ����� ����������

	//--- �������������� � ������ SharedBuffer
	int SetMsg_HDR(int change_msg, _SB_DEV_STATUS *dev, TCHAR *OUT_pszResponse); // ������ ���� SB HEADER
	int GetMsg_HDR(int & change_msg, int & numDev, _SB_DEV_STATUS *dev=0, TCHAR * OUT_pszResponse = NULL);
	int SetMsg_CONTROL(_SB_DEV_STATUS *dev, int numDev, std::vector<_SBM_PARAM> &vParam, TCHAR *OUT_pszResponse);
	int GetMsg_INFO(_SB_DEV_STATUS *dev, int numDev, TCHAR *OUT_pszResponse);
	int GetMsg_REPLY(int numDev, std::vector<float> &vParam, TCHAR * OUT_pszResponse); // ������� ���� SB REPLY
	int GetMsg_ERROR(int error_cod, int error_more, TCHAR *str_error, int str_sz, TCHAR *OUT_pszResponse); // ������� ���� SB ERROR

private:
	CSharedBuffer *m_pSB;		// ��������� �� ����� CSharedBuffer
	_SB_GLOBAL m_SBG;			// ���������� ����� ��� ���������� ���������

	std::vector<_SBM_DEVICE> m_vDevice; // ������ ������������������ ���������
};

//--- SLAVE
class CSBManagerSlave
{
public:
	CSBManagerSlave(CWnd * host); // �����������
	~CSBManagerSlave(); // ����������

	//--- SharedBuffer
	int SB_Init(TCHAR *OUT_pszResponse);	// ������������� SharedBuffer
	int SB_Reset(TCHAR *OUT_pszResponse);   // ����� SharedBuffer
	int SB_CycleGet(bool start, TCHAR *OUT_pszResponse); // ������ ������ ������ ��������� �� SharedBuffer

	//--- ����������
	int SearchDeviceAnswer(_SB_DEV_STATUS *dev, int dev_num, TCHAR *OUT_pszResponse = NULL); // ��������� ����� �� ����� ������������ ���������
	int GetDevStatus(_SB_DEV_STATUS *dev, int &numDev, int type, int num, int mode, int state, TCHAR *OUT_pszResponse); // �������� ��������� � ����� ����������
	
	// �������� �������� ID & CHECKSUM
	//--- �������������� � ������ SharedBuffer
	int SetMsg_HDR(int change_msg,  _SB_DEV_STATUS *dev, TCHAR *OUT_pszResponse); // ������ ���� SB HEADER
	int GetMsg_HDR(int & change_msg, int & numDev, _SB_DEV_STATUS *dev = 0, TCHAR * OUT_pszResponse = NULL);
	int GetMsg_CONTROL(int numDev, std::vector<float> &vParam, TCHAR * OUT_pszResponse); // ������ ���� SB CONTROL
	int SetMsg_INFO(_SB_DEV_STATUS *dev, int numDev, bool update, TCHAR *OUT_pszResponse); // ������ ���� SB INFO
	int SetMsg_REPLY(_SB_DEV_STATUS *dev, int numDev, float *pParam, int param_num, TCHAR *OUT_pszResponse); // ������ ���� SB REPLY
	int SetMsg_ERROR(_SB_DEV_STATUS *dev, int numDev, int cod_error, int more_error, TCHAR *str_error, int str_num, TCHAR *OUT_pszResponse); // ������ ���� SB ERROR
	
private:
	CSharedBuffer *m_pSB;		// ��������� �� ����� CSharedBuffer
	_SB_GLOBAL m_SBG;			// ���������� ����� ��� ���������� ���������

	std::vector<_SBM_DEVICE> m_vDevice; // ������ ������������������ ���������
};