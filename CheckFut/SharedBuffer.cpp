#include "stdafx.h"
#include "SharedBuffer.h"

#define  NAME_MAP    _T("Local\\MySharedBuf-memory")
#define  NAME_MUTEX  _T("Local\\MySharedBuf-mutex")
#define  NAME_EVENT  _T("Local\\MySharedBuf-event")

static _SB_GLOBAL SBG; // ���������� ��������� SharedBuffer

CSharedBuffer::CSharedBuffer(CWnd *hostWindow)
{
	m_hostWindow = hostWindow;
	hMapFile = NULL;			// ����� ����� ��������
	hMutex = NULL;				// ����� ��������
	m_StatInit = FALSE;			// ���� ������� ������������� (����� Init(...), ����� Destroy())
	pBuf = 0;					// ��������� �� ����� SharedMemory
	m_Buf_size = 0;				// ������ ������ SharedMemory

								//--- �������� ������� ��� ������ ������ ���������
	m_EventExit = new CEvent(FALSE, FALSE, NULL); // �������� ������� ������
	m_EventMySending = new CEvent(FALSE, TRUE, NULL); // �������� ������� ��� ��������� ���������� ����
	m_EventGetMsg = new CEvent(FALSE, FALSE, NAME_EVENT); // �������� ����������� ������� ������ ���������

	m_pThread = NULL;
}

CSharedBuffer::~CSharedBuffer()
{
	// �������� ������ ������ ���������
	ClearThread();
	if (m_EventExit) delete m_EventExit;
	if (m_EventGetMsg) delete m_EventGetMsg;
	if (m_EventMySending) delete m_EventMySending;

	Destroy(); // �������� SharedBuffer
}

int CSharedBuffer::Init(DWORD byte_size, TCHAR * OUT_pszResponse)
{
	Destroy(); // �������, ���� ��� �� ���� ������� ��� SharedBuffer

	m_Buf_size = byte_size;

	// ������� �����
	hMapFile = CreateFileMapping(
		INVALID_HANDLE_VALUE,   // ���������� �����
		NULL,					// ������ 
		PAGE_READWRITE,         // �������� ������
		0,						// ������� ����� �������
		m_Buf_size,				// ������� ����� �������
		NAME_MAP);				// ��� �������

								// ��������� �����
	if (hMapFile == NULL)
	{
		if (OUT_pszResponse) swprintf_s(OUT_pszResponse, MAX_PATH, _T("Could not create file mapping object (%d)!\n"), GetLastError());
		return 1;
	}

	// ������� ��������� �� Shared Memory
	pBuf = (BYTE*)MapViewOfFile(
		hMapFile,				// �����. ������� ������������ ����
		FILE_MAP_ALL_ACCESS,    // ����� �������
		0,						// ������� DWORD ��������
		0,						// ������� DWORD ��������
		0);						// ����� ������������ ������ - ���� 0 �� ���� ����

								// ��������� ���������
	if (pBuf == NULL)
	{
		if (OUT_pszResponse) swprintf_s(OUT_pszResponse, MAX_PATH, _T("Could not map view of file (%d)!\n"), GetLastError());
		if (hMapFile) CloseHandle(hMapFile);
		hMapFile = NULL;
		return 2;
	}

	hMutex = CreateMutex(
		NULL,			// ��������� ������������ ��������
		FALSE,			// ��������� ���������
		NAME_MUTEX);	// ��� �������

	if (hMutex == NULL)
	{
		if (OUT_pszResponse) swprintf_s(OUT_pszResponse, MAX_PATH, _T("Could not create Mutex! Error: %d!\n"), GetLastError());
		if (pBuf) UnmapViewOfFile(pBuf);
		if (hMapFile) CloseHandle(hMapFile);
		hMapFile = NULL;
		pBuf = 0;
		return 3;
	}

	m_StatInit = TRUE;

	return 0;
}

void CSharedBuffer::Destroy()
{
	if (hMutex) CloseHandle(hMutex);
	if (pBuf) UnmapViewOfFile(pBuf);
	if (hMapFile) CloseHandle(hMapFile);

	hMapFile = NULL;
	hMutex = NULL;
	pBuf = 0;

	m_StatInit = FALSE;
}

int CSharedBuffer::ResetBuf(TCHAR * OUT_pszResponse)
{
	if (!GetStatusInit()) // �����, ���� SharedBuffer �� ���������������
	{
		if (OUT_pszResponse) swprintf_s(OUT_pszResponse, MAX_PATH, _T("ResetBuf - ERROR! ���������� ������������� ������ ������!\n"));
		return 1;
	}

	CString temp;

	_SB_GLOBAL SBG;
	memset(&SBG, 0, sizeof(_SB_GLOBAL)); // ����� ���� ����� SharedBuf

										 //--- ����������
										 // HEADER
	SBG.hdr.id_header = 0x52444548;
	SBG.hdr.version_prot = VERSION_SHARED_BUFFER;
	SBG.hdr.global_size = sizeof(_SB_GLOBAL);
	SBG.hdr.check_sum = GetCheckSum((BYTE*)&SBG.hdr, sizeof(_SB_HEADER));

	// CONTROL
	SBG.control.id_control = 0x4C525443;
	SBG.control.check_sum = GetCheckSum((BYTE*)&SBG.control, sizeof(_SB_CONTROL));

	// INFO
	SBG.info.id_info = 0x4F464E49;
	SBG.info.check_sum = GetCheckSum((BYTE*)&SBG.info, sizeof(_SB_INFO));

	// REPLY
	SBG.reply.id_reply = 0x4C504552;
	SBG.reply.check_sum = GetCheckSum((BYTE*)&SBG.reply, sizeof(_SB_REPLY));

	// ERROR
	SBG.error.id_error = 0x21525245;
	SBG.error.check_sum = GetCheckSum((BYTE*)&SBG.error, sizeof(_SB_ERROR));

	if (SetBuf((BYTE*)&SBG, 0, sizeof(_SB_GLOBAL), OUT_pszResponse) != 0) return 2;

	if (OUT_pszResponse) swprintf_s(OUT_pszResponse, MAX_PATH, _T("���������� ����� ����� ����� ������!\n"));
	return 0;
}

int CSharedBuffer::SetBuf(BYTE * IN_buf, DWORD byte_offset, DWORD byte_size, TCHAR * OUT_pszResponse)
{
	if (IN_buf == 0)
	{
		if (OUT_pszResponse) swprintf_s(OUT_pszResponse, MAX_PATH, _T("SetBuf - ERROR! ������ IN_BUF = 0!\n"));
		return 1;
	}

	if (hMapFile == NULL || hMutex == NULL || pBuf == 0 || m_StatInit == FALSE)
	{
		if (OUT_pszResponse) swprintf_s(OUT_pszResponse, MAX_PATH, _T("SetBuf - ERROR! ���������� ���������������� ����� ������!\n"));
		return 2;
	}

	if (byte_size == 0)
	{
		if (OUT_pszResponse) swprintf_s(OUT_pszResponse, MAX_PATH, _T("SetBuf - ERROR! ������������ ������ ������������� ������! portion = %d!\n"), byte_size);
		return 3;
	}

	DWORD buf_sum = byte_offset + byte_size;
	if (buf_sum > m_Buf_size)
	{
		if (OUT_pszResponse) swprintf_s(OUT_pszResponse, MAX_PATH, _T("SetBuf - ERROR! ������������ ���������! offset:%d portion:%d | buf_sum:%d > max_size:%d!\n"), byte_offset, byte_size, buf_sum, m_Buf_size);
		return 4;
	}

	// ������� ������� � ���������
	if (WaitForSingleObject(hMutex, INFINITE) != WAIT_OBJECT_0)
	{
		if (OUT_pszResponse) swprintf_s(OUT_pszResponse, MAX_PATH, _T("SetBuf - ERROR! Invalid hMutex!\n"));
		return 5;
	}

	memcpy(&pBuf[byte_offset], IN_buf, byte_size);

	ReleaseMutex(hMutex); // ����������� mutex

	return 0;
}

void CSharedBuffer::UpdateBuf()
{
	// ���� PulseEvent ���������� ��� ������� �� ������� � ������, �� ��� ������, ��������� ���� ������, �������� ����������
	m_EventMySending->SetEvent(); // �� �� ��������� ��������� - �� ��������� ���
	m_EventGetMsg->PulseEvent(); // �� ��������� ��������� � ����� �����
}

int CSharedBuffer::GetBuf(BYTE * OUT_buf, DWORD byte_offset, DWORD byte_size, TCHAR * OUT_pszResponse)
{
	if (OUT_buf == 0)
	{
		if (OUT_pszResponse) swprintf_s(OUT_pszResponse, MAX_PATH, _T("GetBuf - ERROR! ������ OUT_buf = 0!\n"));
		return 1;
	}

	if (hMapFile == NULL || hMutex == NULL || pBuf == 0 || m_StatInit == FALSE)
	{
		if (OUT_pszResponse) swprintf_s(OUT_pszResponse, MAX_PATH, _T("GetBuf - ERROR! ���������� ������������� ����� ������!\n"));
		return 2;
	}

	if (byte_size == 0)
	{
		if (OUT_pszResponse) swprintf_s(OUT_pszResponse, MAX_PATH, _T("GetBuf - ERROR! ������������ ������ ������������� ������! portion:%d == 0!\n"), byte_size);
		return 3;
	}

	DWORD buf_sum = byte_offset + byte_size;
	if (buf_sum > m_Buf_size)
	{
		if (OUT_pszResponse) swprintf_s(OUT_pszResponse, MAX_PATH, _T("GetBuf - ERROR! ������������ ���������! offset:%d portion:%d | buf_sum:%d > max_size:%d!\n"), byte_offset, byte_size, buf_sum, m_Buf_size);
		return 4;
	}

	// ������� ������� � ���������
	if (WaitForSingleObject(hMutex, INFINITE) != WAIT_OBJECT_0)
	{
		if (OUT_pszResponse) swprintf_s(OUT_pszResponse, MAX_PATH, _T("GetBuf - ERROR! Invalid hMutex!\n"));
		return 5;
	}

	memcpy(OUT_buf, &pBuf[byte_offset], byte_size);

	ReleaseMutex(hMutex); // �����������

	return 0;
}

UINT CSharedBuffer::GetCheckSum(BYTE * buf, size_t buf_sz)
{
	UINT sum_buf = 0;
	for (size_t i = 0; i<buf_sz - sizeof(uint32_t); i++)
	{
		sum_buf += buf[i];
	}
	return sum_buf;
}

void CSharedBuffer::ClearThread()
{
	// ��������� ��� �� ��������������� ����� � �� ���� �� ��� ���� ������
	if (m_pThread != NULL && m_pThread != INVALID_HANDLE_VALUE)
	{
		m_EventExit->SetEvent(); // ����� ������� ������
		WaitForSingleObject(m_pThread->m_hThread, INFINITE); // ������� ���������� ������
		delete m_pThread; // ������� ������ � ��������� �� �����
		m_pThread = NULL; // �������� ���������
	}
}

int CSharedBuffer::StartCycleGet()
{
	if (!m_StatInit) return 1;
	ClearThread(); // ���� ����� ��� ������� - ���������� ���

	// ������ ������
	m_EventExit->ResetEvent(); // ����� ������� ������
	m_pThread = AfxBeginThread(Thread, this, 0, 0, CREATE_SUSPENDED, NULL);
	m_pThread->m_bAutoDelete = false; // ��������� ������������
	m_pThread->ResumeThread(); // ����� ������
	return 0;
}

UINT CSharedBuffer::Thread(LPVOID pParam)
{
	((CSharedBuffer*)pParam)->Run(); // ��������� � ����� ������� Run, ������� ���������� ��������������
	AfxEndThread(0, FALSE); // �� ������� ����� ��� ������ �� ����
	return 0;
}

void CSharedBuffer::Run()
{
	// ������ ������� �� ������� ������������ ��������
	DWORD res = 0;
	CSyncObject *pWaitObjects[] = { m_EventExit, m_EventGetMsg, m_EventMySending };
	CMultiLock MultiLock(pWaitObjects, 3L); // ��������� �� 3 �������
	while (1)
	{
		// ���������� � �������� ������ �� �������
		res = MultiLock.Lock(INFINITE, FALSE);

		if (res == WAIT_OBJECT_0 + 1) // �� �������� ���������
		{
			m_hostWindow->PostMessageW(WM_SB_CYCLE_GET_MSG, 0, 0);
			TRACE(_T("RUN: GET_MSG!\n"));
			continue;
		}
		else if (res == WAIT_OBJECT_0 + 2) // �� ���� ��������� ��������� - ����������
		{
			m_EventMySending->ResetEvent();
			TRACE(_T("RUN: MY_SEND!\n"));
			continue;
		}
		else // �����
		{
			//if (OUT_pszResponse) swprintf_s(OUT_pszResponse, MAX_PATH, _T("Exit WaitEvent! Stop Thread!\n"));
			m_EventMySending->ResetEvent();
			m_EventExit->ResetEvent();
			TRACE(_T("RUN: EXIT!\n"));
			break;
		}
	}
}

//--- MASTER
CSBManagerMaster::CSBManagerMaster(CWnd * host)
{
	m_pSB = new CSharedBuffer(host);
}

CSBManagerMaster::~CSBManagerMaster()
{
	m_vDevice.clear();

	if (m_pSB) delete m_pSB;
}

int CSBManagerMaster::SB_Init(TCHAR * OUT_pszResponse)
{
	TCHAR pszResponse[MAX_PATH] = { 0 };
	// ������������� ����� ����� ������
	if (m_pSB->Init(sizeof(_SB_GLOBAL), pszResponse) != 0)
	{
		if (OUT_pszResponse) swprintf_s(OUT_pszResponse, MAX_PATH, _T("CSBManagerMaster - ������ ������������� %s\n"), pszResponse);
		return 1;
	}
	else
	{
		if (OUT_pszResponse) swprintf_s(OUT_pszResponse, MAX_PATH, _T("CSBManagerMaster - �������������!\n"));
	}
	return 0;
}

int CSBManagerMaster::SB_Reset(TCHAR * OUT_pszResponse)
{
	TCHAR pszResponse[MAX_PATH] = { 0 };
	// ����� ����� ����� ������
	if (m_pSB->ResetBuf(pszResponse) != 0)
	{
		if (OUT_pszResponse) swprintf_s(OUT_pszResponse, MAX_PATH, _T("CSBManagerMaster - ������ ������ �����:%s\n"), pszResponse);
		return 1;
	}
	else
	{
		if (OUT_pszResponse) swprintf_s(OUT_pszResponse, MAX_PATH, _T("CSBManagerMaster - ����� �����!\n"));
	}
	return 0;
}

int CSBManagerMaster::SB_CycleGet(bool start, TCHAR * OUT_pszResponse)
{
	if (start)
	{
		if (m_pSB->StartCycleGet() != 0)
		{
			if (OUT_pszResponse) swprintf_s(OUT_pszResponse, MAX_PATH, _T("CSBManagerMaster - ��� SB_CycleGet ���������� ������������� SharedBuffer!\n"));
			return 1;
		}
		else
		{
			if (OUT_pszResponse) swprintf_s(OUT_pszResponse, MAX_PATH, _T("CSBManagerMaster - ������ ������ ������ ���������!\n"));
		}
	}
	else
	{
		m_pSB->ClearThread();
		if (OUT_pszResponse) swprintf_s(OUT_pszResponse, MAX_PATH, _T("CSBManagerMaster - ��������� ������ ������ ���������!\n"));
	}

	return 0;
}

int CSBManagerMaster::SearchDeviceRequest(TCHAR * OUT_pszResponse)
{
	//--- �������� ���� ������� (1-CONTROL,0-type,255-num) ��� ����������� ���������
	// ������� ���� �� ����� ������
	if (m_pSB->GetBuf((BYTE*)&m_SBG.hdr, _SB_OFFSET().hdr, sizeof(_SB_HEADER), OUT_pszResponse) != 0) return 1;
	// ���������� �����
	m_SBG.hdr.change_msg = 1; // ��� ��������� ����������� ����
	m_SBG.hdr.dev.type = 0; // ��� ���������� ����������� ����
	m_SBG.hdr.dev.num = 255; // ����� ���������� ����������� ����
	m_SBG.hdr.check_sum = m_pSB->GetCheckSum((BYTE*)&m_SBG.hdr, sizeof(_SB_HEADER));
	// �������� ���� � ����� ������
	if (m_pSB->SetBuf((BYTE*)&m_SBG.hdr, _SB_OFFSET().hdr, sizeof(_SB_HEADER), OUT_pszResponse) != 0) return 2;
	// �������� ������� �� ���������� ������ ������
	m_pSB->UpdateBuf(); // ��������� ������� ���������� ������
	return 0;
}

int CSBManagerMaster::InitDeviceInfo(TCHAR * OUT_pszResponse)
{
	_SB_INFO *info = (_SB_INFO*)&m_SBG.info;
	// ������� ���� �� ����� ������
	if (m_pSB->GetBuf((BYTE*)info, _SB_OFFSET().info, sizeof(_SB_INFO), OUT_pszResponse) != 0) return 1;

	_SBM_DEVICE m_SBMDT; memset(&m_SBMDT, 0, sizeof(_SBM_DEVICE));
	m_vDevice.clear();

	int offset_control = 0;
	int offset_reply = 0;
	int size_control = 0;
	int size_reply = 0;

	CString msg, temp;
	msg = _T("����������: ");
	for (int i = 0; i < info->number_dev; i++)
	{
		m_SBMDT.type = info->DEV_STAT[i].type;
		m_SBMDT.num = info->DEV_STAT[i].num;
		// CONTROL
		size_control = info->DEV_STAT[i].control_size;
		m_SBMDT.control_offset = offset_control;
		m_SBMDT.control_size = size_control;
		offset_control += size_control;
		// REPLY
		size_reply = info->DEV_STAT[i].reply_size;
		m_SBMDT.reply_offset = offset_reply;
		m_SBMDT.reply_size = size_reply;
		offset_reply += size_reply;

		m_vDevice.push_back(m_SBMDT);

		//---
		temp.Format(_T("|%d%d%d%d|"), info->DEV_STAT[i].type, info->DEV_STAT[i].num, info->DEV_STAT[i].mode, info->DEV_STAT[i].state);
		msg.Append(temp);
	}
	//TRACE(_T("%s\n"), msg);

	if (OUT_pszResponse) swprintf_s(OUT_pszResponse, MAX_PATH, msg);

	return 0;
}

int CSBManagerMaster::GetDevStatus(_SB_DEV_STATUS * dev, int & numDev, int type, int num, int mode, int state, TCHAR * OUT_pszResponse)
{
	if (m_vDevice.size() == 0)
	{
		if (OUT_pszResponse) swprintf_s(OUT_pszResponse, MAX_PATH, _T("CSBManagerSlave - ERROR! �������������� ����� ��������� ����!\n"));
		return 1;
	}

	// ����� ���������� �� ���� � ������
	int n = -1;
	for (size_t i = 0; i < m_vDevice.size(); i++)
	{
		if (type == m_vDevice.at(i).type && num == m_vDevice.at(i).num)
		{
			n = i;
			break;
		}
	}
	if (n == -1) // ���� �� ����� ������ ��� � �����
	{
		if (OUT_pszResponse) swprintf_s(OUT_pszResponse, MAX_PATH, _T("CSBManagerSlave - ERROR! GetDevStat �� ����� ������������� ���������� type:%d num:%d\n"), type, num);
		return 2;
	}

	// ���������� ��������� ����������
	if (dev)
	{
		dev->type = type;
		dev->num = num;
		dev->mode = mode;
		dev->state = state;
		dev->control_size = m_vDevice.at(n).control_size;
		dev->reply_size = m_vDevice.at(n).reply_size;
	}

	numDev = n; // �������� ����� ���������� ����������

	return 0;
}

//---
int CSBManagerMaster::SetMsg_HDR(int change_msg, _SB_DEV_STATUS * dev, TCHAR * OUT_pszResponse)
{
	// ������� ���� �� ����� ������
	if (m_pSB->GetBuf((BYTE*)&m_SBG.hdr, _SB_OFFSET().hdr, sizeof(_SB_HEADER), OUT_pszResponse) != 0) return 1;
	// ���������� �����
	m_SBG.hdr.change_msg = change_msg; // ��� ��������� ����������� ����
	if (dev) memcpy(&m_SBG.hdr.dev, dev, sizeof(_SB_DEV_STATUS));
	else memset(&m_SBG.hdr.dev, 0, sizeof(_SB_DEV_STATUS));
	// ������ ����������� �����
	m_SBG.hdr.check_sum = m_pSB->GetCheckSum((BYTE*)&m_SBG.hdr, sizeof(_SB_HEADER));
	// �������� ���� � ����� ������
	if (m_pSB->SetBuf((BYTE*)&m_SBG.hdr, _SB_OFFSET().hdr, sizeof(_SB_HEADER), OUT_pszResponse) != 0) return 2;
	// �������� ������� �� ���������� ������ ������
	m_pSB->UpdateBuf();
	return 0;
}

int CSBManagerMaster::GetMsg_HDR(int & change_msg, int & numDev, _SB_DEV_STATUS *dev, TCHAR * OUT_pszResponse)
{
	_SB_HEADER *hdr = (_SB_HEADER*)&m_SBG.hdr;
	// ������� ���� �� ����� ������
	if (m_pSB->GetBuf((BYTE*)hdr, _SB_OFFSET().hdr, sizeof(_SB_HEADER), OUT_pszResponse) != 0) return 1;

	//TRACE(_T("change:%d | type:%d num:%d mode:%d state:%d | sz_c:%d sz_r:%d\n"), hdr->change_msg, hdr->dev.type, hdr->dev.num, hdr->dev.mode, hdr->dev.state, hdr->dev.control_size, hdr->dev.reply_size);

	if (hdr->change_msg == 0)
	{
		if (OUT_pszResponse) swprintf_s(OUT_pszResponse, MAX_PATH, _T("CSBManagerSlave - ERROR! GetMsg_HDR - ������������ ��� ���� %d!\n"), hdr->change_msg);
		return 1;
	}

	change_msg = hdr->change_msg;

	if (hdr->dev.type != 0) // ����������� ������
	{
		if (GetDevStatus(dev, numDev, hdr->dev.type, hdr->dev.num, hdr->dev.mode, hdr->dev.state, OUT_pszResponse) != 0)
		{
			if (OUT_pszResponse) swprintf_s(OUT_pszResponse, MAX_PATH, _T("CSBManagerSlave - ERROR! GetDevStatus - ������������ type:%d num:%d!\n"), hdr->dev.type, hdr->dev.num);
			return 2;
		}
	}

	if (dev)
	{
		memcpy(dev, &hdr->dev, sizeof(_SB_DEV_STATUS));
	}

	return 0;
}

int CSBManagerMaster::SetMsg_CONTROL(_SB_DEV_STATUS *dev, int numDev, std::vector<_SBM_PARAM>& vParam, TCHAR * OUT_pszResponse)
{
	if (m_vDevice.size() == 0)
	{
		if (OUT_pszResponse) swprintf_s(OUT_pszResponse, MAX_PATH, _T("CSBManagerSlave - ERROR! �������������� ����� ��������� ����!\n"));
		return 1;
	}

	_SB_CONTROL *ctrl = (_SB_CONTROL*)&m_SBG.control;
	// ������� ���� �� ����� ������
	if (m_pSB->GetBuf((BYTE*)ctrl, _SB_OFFSET().control, sizeof(_SB_CONTROL), OUT_pszResponse) != 0) return 3;

	int offset = m_vDevice.at(numDev).control_offset;
	int size = m_vDevice.at(numDev).control_size;
	float *pf = (float*)&ctrl->BUF[offset];
	int nt = 0;
	int elem = size / sizeof(float);
	for (size_t i = 0; i < vParam.size(); i++)
	{
		nt = vParam.at(i).num;
		if (nt >= 0 && nt < elem)
		{
			pf[nt] = vParam.at(i).value;
		}
		else
		{
			if (OUT_pszResponse) swprintf_s(OUT_pszResponse, MAX_PATH, _T("CSBManagerMaster - ERROR! SetMsg_CONTROL ����� ��������� [%d] ��� ������ ���������� ��������� [%d]!\n"), nt, elem);
			return 4;
		}
	}

	// ������������� ����������� �����
	ctrl->check_sum = m_pSB->GetCheckSum((BYTE*)ctrl, sizeof(_SB_CONTROL));

	// �������� ���� � ����� ������
	if (m_pSB->SetBuf((BYTE*)ctrl, _SB_OFFSET().control, sizeof(_SB_CONTROL), OUT_pszResponse) != 0) return 5;

	// ���������� ���� HDR + ��������
	if (SetMsg_HDR(1, dev, OUT_pszResponse) != 0) return 6;

	return 0;
}

int CSBManagerMaster::GetMsg_INFO(_SB_DEV_STATUS * dev, int numDev, TCHAR * OUT_pszResponse)
{
	if (m_vDevice.size() == 0)
	{
		if (OUT_pszResponse) swprintf_s(OUT_pszResponse, MAX_PATH, _T("CSBManagerSlave - ERROR! �������������� ����� ��������� ����!\n"));
		return 1;
	}

	_SB_INFO *info = (_SB_INFO*)&m_SBG.info;

	// ������� ���� �� ����� ������
	if (m_pSB->GetBuf((BYTE*)info, _SB_OFFSET().info, sizeof(_SB_INFO), OUT_pszResponse) != 0) return 2;

	if (dev)
	{
		int type = m_vDevice.at(numDev).type;
		int num = m_vDevice.at(numDev).num;
		int index = -1;
		for (int i = 0; i < info->number_dev; i++)
		{
			if (type == info->DEV_STAT[i].type && num == info->DEV_STAT[i].num)
			{
				index = i;
				break;
			}
		}
		if (index == -1)
		{
			if (OUT_pszResponse) swprintf_s(OUT_pszResponse, MAX_PATH, _T("CSBManagerMaster - ERROR! GetMsg_INFO �� ����� ������������� ���������� type:%d num:%d\n"), type, num);
			return 3;
		}

		memcpy(dev, &info->DEV_STAT[index], sizeof(_SB_DEV_STATUS));
	}

	return 0;
}

int CSBManagerMaster::GetMsg_REPLY(int numDev, std::vector<float>& vParam, TCHAR * OUT_pszResponse)
{
	if (m_vDevice.size() == 0)
	{
		if (OUT_pszResponse) swprintf_s(OUT_pszResponse, MAX_PATH, _T("CSBManagerMaster - ERROR! �������������� ����� ��������� ����!\n"));
		return 1;
	}

	_SB_REPLY *repl = (_SB_REPLY*)&m_SBG.reply;

	// ������� ���� �� ����� ������
	if (m_pSB->GetBuf((BYTE*)repl, _SB_OFFSET().reply, sizeof(_SB_REPLY), OUT_pszResponse) != 0) return 2;

	int elem = m_vDevice.at(numDev).reply_size / sizeof(float); // ���-�� ����������� ���������
	float *pf = (float*)&repl->BUF[m_vDevice.at(numDev).reply_offset]; // ��������� �� ������ ������

	vParam.clear();
	for (int n = 0; n < elem; n++)
	{
		vParam.push_back(pf[n]);
	}

	return 0;
}

int CSBManagerMaster::GetMsg_ERROR(int error_cod, int error_more, TCHAR * str_error, int str_sz, TCHAR * OUT_pszResponse)
{
	_SB_ERROR *err = (_SB_ERROR*)&m_SBG.error;

	// ������� ���� �� ����� ������
	if (m_pSB->GetBuf((BYTE*)err, _SB_OFFSET().error, sizeof(_SB_ERROR), OUT_pszResponse) != 0) return 1;

	error_cod = err->error_cod;
	error_more = err->error_more;

	int max = sizeof(_SB_ERROR::error_str) / 2;
	if (str_sz > max) str_sz = max;
	swprintf_s(str_error, str_sz, _T("%s"), err->error_str);

	return 0;
}

//--- SLAVE
CSBManagerSlave::CSBManagerSlave(CWnd * host)
{
	m_pSB = new CSharedBuffer(host);
}

CSBManagerSlave::~CSBManagerSlave()
{
	m_vDevice.clear();

	if (m_pSB) delete m_pSB;
}

int CSBManagerSlave::SB_Init(TCHAR * OUT_pszResponse)
{
	TCHAR pszResponse[MAX_PATH] = { 0 };
	// ������������� ����� ����� ������
	if (m_pSB->Init(sizeof(_SB_GLOBAL), pszResponse) != 0)
	{
		if (OUT_pszResponse) swprintf_s(OUT_pszResponse, MAX_PATH, _T("CSBManagerSlave - ������ ������������� %s\n"), pszResponse);
		return 1;
	}
	else
	{
		if (OUT_pszResponse) swprintf_s(OUT_pszResponse, MAX_PATH, _T("CSBManagerSlave - �������������!\n"));
	}
	return 0;
}

int CSBManagerSlave::SB_Reset(TCHAR * OUT_pszResponse)
{
	TCHAR pszResponse[MAX_PATH] = { 0 };
	// ����� ����� ����� ������
	if (m_pSB->ResetBuf(pszResponse) != 0)
	{
		if (OUT_pszResponse) swprintf_s(OUT_pszResponse, MAX_PATH, _T("CSBManagerSlave - ������ ������ �����:%s\n"), pszResponse);
		return 1;
	}
	else
	{
		if (OUT_pszResponse) swprintf_s(OUT_pszResponse, MAX_PATH, _T("CSBManagerSlave - ����� �����!\n"));
	}
	return 0;
}

int CSBManagerSlave::SB_CycleGet(bool start, TCHAR * OUT_pszResponse)
{
	if (start)
	{
		if (m_pSB->StartCycleGet() != 0)
		{
			if (OUT_pszResponse) swprintf_s(OUT_pszResponse, MAX_PATH, _T("CSBManagerSlave - ��� SB_CycleGet ���������� ������������� SharedBuffer!\n"));
			return 1;
		}
		else
		{
			if (OUT_pszResponse) swprintf_s(OUT_pszResponse, MAX_PATH, _T("CSBManagerSlave - ������ ������ ������ ���������!\n"));
		}
	}
	else
	{
		m_pSB->ClearThread();
		if (OUT_pszResponse) swprintf_s(OUT_pszResponse, MAX_PATH, _T("CSBManagerSlave - ��������� ������ ������ ���������!\n"));
	}

	return 0;
}

//---
int CSBManagerSlave::SearchDeviceAnswer(_SB_DEV_STATUS *dev, int dev_num, TCHAR * OUT_pszResponse)
{
	_SB_INFO *info = (_SB_INFO*)&m_SBG.info;

	// ������� ���� �� ����� ������
	if (m_pSB->GetBuf((BYTE*)info, _SB_OFFSET().info, sizeof(_SB_INFO), OUT_pszResponse) != 0) return 1;

	info->number_dev = dev_num;
	memset(info->DEV_STAT, 0, sizeof(_SB_INFO::DEV_STAT));
	memcpy(info->DEV_STAT, dev, dev_num * sizeof(_SB_DEV_STATUS));

	//--- ���������� ������� ��������� ����������� ��������+������
	_SBM_DEVICE m_SBMDT; memset(&m_SBMDT, 0, sizeof(_SBM_DEVICE));
	m_vDevice.clear();

	int offset_control = 0;
	int offset_reply = 0;
	int size_control = 0;
	int size_reply = 0;

	// ���������� �������� �������� � ������� ��� CONTROL & REPLY
	for (int i = 0; i < dev_num; i++)
	{
		m_SBMDT.type = info->DEV_STAT[i].type;
		m_SBMDT.num = info->DEV_STAT[i].num;
		// CONTROL
		size_control = info->DEV_STAT[i].control_size;
		m_SBMDT.control_offset = offset_control;
		m_SBMDT.control_size = size_control;
		offset_control += size_control;
		// REPLY
		size_reply = info->DEV_STAT[i].reply_size;
		m_SBMDT.reply_offset = offset_reply;
		m_SBMDT.reply_size = size_reply;
		offset_reply += size_reply;

		m_vDevice.push_back(m_SBMDT);
	}
	// ������ ����������� �����
	info->check_sum = m_pSB->GetCheckSum((BYTE*)info, sizeof(_SB_INFO));

	// �������� ���� � ����� ������
	if (m_pSB->SetBuf((BYTE*)info, _SB_OFFSET().info, sizeof(_SB_INFO), OUT_pszResponse) != 0) return 2;
	
	// ���������� ���� HDR
	if (SetMsg_HDR(2, 0, OUT_pszResponse) != 0) return 3;

	return 0;
}

int CSBManagerSlave::GetDevStatus(_SB_DEV_STATUS *dev, int &numDev, int type, int num, int mode, int state, TCHAR *OUT_pszResponse)
{
	if (m_vDevice.size() == 0)
	{
		if (OUT_pszResponse) swprintf_s(OUT_pszResponse, MAX_PATH, _T("CSBManagerSlave - ERROR! �������������� ����� ��������� ����!\n"));
		return 1;
	}

	// ����� ���������� �� ���� � ������
	int n = -1;
	for (size_t i = 0; i < m_vDevice.size(); i++)
	{
		if (type == m_vDevice.at(i).type && num == m_vDevice.at(i).num)
		{
			n = i;
			break;
		}
	}
	if (n == -1) // ���� �� ����� ������ ��� � �����
	{
		if (OUT_pszResponse) swprintf_s(OUT_pszResponse, MAX_PATH, _T("CSBManagerSlave - ERROR! GetDevStat �� ����� ������������� ���������� type:%d num:%d\n"), type, num);
		return 2;
	}

	// ���������� ��������� ����������
	if (dev)
	{
		dev->type = type;
		dev->num = num;
		dev->mode = mode;
		dev->state = state;
		dev->control_size = m_vDevice.at(n).control_size;
		dev->reply_size = m_vDevice.at(n).reply_size;
	}

	numDev = n; // �������� ����� ���������� ����������

	return 0;
}

//---
int CSBManagerSlave::SetMsg_HDR(int change_msg, _SB_DEV_STATUS * dev, TCHAR * OUT_pszResponse)
{
	// ������� ���� �� ����� ������
	if (m_pSB->GetBuf((BYTE*)&m_SBG.hdr, _SB_OFFSET().hdr, sizeof(_SB_HEADER), OUT_pszResponse) != 0) return 1;
	// ���������� �����
	m_SBG.hdr.change_msg = change_msg; // ��� ��������� ����������� ����
	if (dev) memcpy(&m_SBG.hdr.dev, dev, sizeof(_SB_DEV_STATUS));
	else memset(&m_SBG.hdr.dev, 0, sizeof(_SB_DEV_STATUS));
	// ������ ����������� �����
	m_SBG.hdr.check_sum = m_pSB->GetCheckSum((BYTE*)&m_SBG.hdr, sizeof(_SB_HEADER));
	// �������� ���� � ����� ������
	if (m_pSB->SetBuf((BYTE*)&m_SBG.hdr, _SB_OFFSET().hdr, sizeof(_SB_HEADER), OUT_pszResponse) != 0) return 2;
	// �������� ������� �� ���������� ������ ������
	m_pSB->UpdateBuf();
	return 0;
}

int CSBManagerSlave::GetMsg_HDR(int & change_msg, int & numDev, _SB_DEV_STATUS * dev, TCHAR * OUT_pszResponse)
{
	_SB_HEADER *hdr = (_SB_HEADER*)&m_SBG.hdr;
	// ������� ���� �� ����� ������
	if (m_pSB->GetBuf((BYTE*)hdr, _SB_OFFSET().hdr, sizeof(_SB_HEADER), OUT_pszResponse) != 0) return 1;

	//TRACE(_T("change:%d | type:%d num:%d mode:%d state:%d | sz_c:%d sz_r:%d\n"), hdr->change_msg, hdr->dev.type, hdr->dev.num, hdr->dev.mode, hdr->dev.state, hdr->dev.control_size, hdr->dev.reply_size);

	if (hdr->change_msg == 0)
	{
		if (OUT_pszResponse) swprintf_s(OUT_pszResponse, MAX_PATH, _T("CSBManagerSlave - ERROR! GetMsg_HDR - ������������ ��� ���� %d!\n"), hdr->change_msg);
		return 1;
	}

	change_msg = hdr->change_msg;

	if (hdr->dev.type != 0) // ����������� ������
	{
		if (GetDevStatus(dev, numDev, hdr->dev.type, hdr->dev.num, hdr->dev.mode, hdr->dev.state, OUT_pszResponse) != 0)
		{
			if (OUT_pszResponse) swprintf_s(OUT_pszResponse, MAX_PATH, _T("CSBManagerSlave - ERROR! GetDevStatus - ������������ type:%d num:%d!\n"), hdr->dev.type, hdr->dev.num);
			return 2;
		}
	}

	if (dev)
	{
		memcpy(dev, &hdr->dev, sizeof(_SB_DEV_STATUS));
	}

	return 0;
}

int CSBManagerSlave::GetMsg_CONTROL(int numDev, std::vector<float> &vParam, TCHAR * OUT_pszResponse)
{
	if (m_vDevice.size() == 0)
	{
		if (OUT_pszResponse) swprintf_s(OUT_pszResponse, MAX_PATH, _T("CSBManagerSlave - ERROR! �������������� ����� ��������� ����!\n"));
		return 1;
	}

	_SB_CONTROL *ctrl = (_SB_CONTROL*)&m_SBG.control;
	
	// ������� ���� �� ����� ������
	if (m_pSB->GetBuf((BYTE*)ctrl, _SB_OFFSET().control, sizeof(_SB_CONTROL), OUT_pszResponse) != 0) return 2;

	int elem = m_vDevice.at(numDev).control_size / sizeof(float); // ���-�� ����������� ���������
	float *pf = (float*)&ctrl->BUF[m_vDevice.at(numDev).control_offset]; // ��������� �� ������ ������

	vParam.clear();
	for (int n = 0; n < elem; n++)
	{
		vParam.push_back(pf[n]);
	}

	return 0;
}

int CSBManagerSlave::SetMsg_INFO(_SB_DEV_STATUS *dev, int numDev, bool update, TCHAR * OUT_pszResponse)
{
	if (m_vDevice.size() == 0)
	{
		if (OUT_pszResponse) swprintf_s(OUT_pszResponse, MAX_PATH, _T("CSBManagerSlave - ERROR! �������������� ����� ��������� ����!\n"));
		return 1;
	}

	_SB_INFO *info = (_SB_INFO*)&m_SBG.info;

	// ������� ���� �� ����� ������
	if (m_pSB->GetBuf((BYTE*)info, _SB_OFFSET().info, sizeof(_SB_INFO), OUT_pszResponse) != 0) return 2;

	info->DEV_STAT[numDev].mode = dev->mode;
	info->DEV_STAT[numDev].state = dev->state;

	// ������ ����������� �����
	info->check_sum = m_pSB->GetCheckSum((BYTE*)info, sizeof(_SB_INFO));

	// �������� ���� � ����� ������
	if (m_pSB->SetBuf((BYTE*)info, _SB_OFFSET().info, sizeof(_SB_INFO), OUT_pszResponse) != 0) return 3;

	if (update)
	{
		if (SetMsg_HDR(2, dev, OUT_pszResponse) != 0) return 4;
	}

	return 0;
}

int CSBManagerSlave::SetMsg_REPLY(_SB_DEV_STATUS * dev, int numDev, float *pParam, int param_num, TCHAR * OUT_pszResponse)
{
	if (m_vDevice.size() == 0)
	{
		if (OUT_pszResponse) swprintf_s(OUT_pszResponse, MAX_PATH, _T("CSBManagerSlave - ERROR! �������������� ����� ��������� ����!\n"));
		return 1;
	}

	_SB_REPLY *repl = (_SB_REPLY*)&m_SBG.reply;

	// ������� ���� �� ����� ������
	if (m_pSB->GetBuf((BYTE*)repl, _SB_OFFSET().reply, sizeof(_SB_REPLY), OUT_pszResponse) != 0) return 2;

	int elem = m_vDevice.at(numDev).reply_size / sizeof(float); // ���-�� ����������� ���������
	float *pf = (float*)&repl->BUF[m_vDevice.at(numDev).reply_offset]; // ��������� �� ������ ������

	if (param_num != elem)
	{
		if (OUT_pszResponse) swprintf_s(OUT_pszResponse, MAX_PATH, _T("CSBManagerSlave - ERROR! SetMsg_REPLY - ���������� ���������� [%d] �� ����� ����� ���� ������ [%d]! \n"), param_num,elem);
		return 3;
	}

	memcpy(pf, pParam, elem*sizeof(float)); // ����������� ����������

	// ������ ����������� �����
	repl->check_sum = m_pSB->GetCheckSum((BYTE*)repl, sizeof(_SB_REPLY));

	// �������� ���� � ����� ������
	if (m_pSB->SetBuf((BYTE*)repl, _SB_OFFSET().reply, sizeof(_SB_REPLY), OUT_pszResponse) != 0) return 3;

	// ��������� HDR + ��������
	if (SetMsg_HDR(3, dev, OUT_pszResponse) != 0) return 3;

	return 0;
}

int CSBManagerSlave::SetMsg_ERROR(_SB_DEV_STATUS *dev, int numDev, int cod_error, int more_error, TCHAR * str_error, int str_sz, TCHAR * OUT_pszResponse)
{
	_SB_ERROR *err = (_SB_ERROR*)&m_SBG.error;

	// ������� ���� �� ����� ������
	if (m_pSB->GetBuf((BYTE*)err, _SB_OFFSET().error, sizeof(_SB_ERROR), OUT_pszResponse) != 0) return 1;

	err->type = (uint8_t)dev->type;
	err->num = (uint8_t)dev->num;
	err->error_cod = (int8_t)cod_error;
	err->error_more = (int8_t)more_error;
	int max = sizeof(_SB_ERROR::error_str)/2;
	if (str_sz > max) str_sz = max;
	swprintf_s(err->error_str, str_sz, _T("%s"), str_error);

	// ������ ����������� �����
	err->check_sum = m_pSB->GetCheckSum((BYTE*)err, sizeof(_SB_ERROR));

	// �������� ���� � ����� ������
	if (m_pSB->SetBuf((BYTE*)err, _SB_OFFSET().error, sizeof(_SB_ERROR), OUT_pszResponse) != 0) return 2;

	// ��������� HDR + ��������
	if (SetMsg_HDR(4, dev, OUT_pszResponse) != 0) return 3;

	return 0;
}