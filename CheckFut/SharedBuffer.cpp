#include "stdafx.h"
#include "SharedBuffer.h"

#define  NAME_MAP    _T("Local\\MySharedBuf-memory")
#define  NAME_MUTEX  _T("Local\\MySharedBuf-mutex")
#define  NAME_EVENT  _T("Local\\MySharedBuf-event")

static _SB_GLOBAL SBG; // Глобальная структура SharedBuffer

CSharedBuffer::CSharedBuffer(CWnd *hostWindow)
{
	m_hostWindow = hostWindow;
	hMapFile = NULL;			// Хэндл файла маппинга
	hMutex = NULL;				// Хэндл мьютекса
	m_StatInit = FALSE;			// Флаг статуса инициализации (Взвод Init(...), сброс Destroy())
	pBuf = 0;					// Указатель на буфер SharedMemory
	m_Buf_size = 0;				// Размер буфера SharedMemory

								//--- Создание событий для потока приема сообщений
	m_EventExit = new CEvent(FALSE, FALSE, NULL); // Создание события выхода
	m_EventMySending = new CEvent(FALSE, TRUE, NULL); // Создание события что сообщение отправлено мною
	m_EventGetMsg = new CEvent(FALSE, FALSE, NAME_EVENT); // Создание глобального события приема сообщения

	m_pThread = NULL;
}

CSharedBuffer::~CSharedBuffer()
{
	// Удаление потока приема сообщений
	ClearThread();
	if (m_EventExit) delete m_EventExit;
	if (m_EventGetMsg) delete m_EventGetMsg;
	if (m_EventMySending) delete m_EventMySending;

	Destroy(); // Удаление SharedBuffer
}

int CSharedBuffer::Init(DWORD byte_size, TCHAR * OUT_pszResponse)
{
	Destroy(); // Удаляем, если что то было создано для SharedBuffer

	m_Buf_size = byte_size;

	// Создаем хэндл
	hMapFile = CreateFileMapping(
		INVALID_HANDLE_VALUE,   // дескриптор файла
		NULL,					// защита 
		PAGE_READWRITE,         // атрибуты защиты
		0,						// старшее слово размера
		m_Buf_size,				// младшее слово размера
		NAME_MAP);				// имя объекта

								// Проверяем хэндл
	if (hMapFile == NULL)
	{
		if (OUT_pszResponse) swprintf_s(OUT_pszResponse, MAX_PATH, _T("Could not create file mapping object (%d)!\n"), GetLastError());
		return 1;
	}

	// Создаем указатель на Shared Memory
	pBuf = (BYTE*)MapViewOfFile(
		hMapFile,				// дескр. объекта проецируемый файл
		FILE_MAP_ALL_ACCESS,    // режим доступа
		0,						// старшее DWORD смещения
		0,						// младшее DWORD смещения
		0);						// число отображаемых байтов - если 0 то весь файл

								// Проверяем указатель
	if (pBuf == NULL)
	{
		if (OUT_pszResponse) swprintf_s(OUT_pszResponse, MAX_PATH, _T("Could not map view of file (%d)!\n"), GetLastError());
		if (hMapFile) CloseHandle(hMapFile);
		hMapFile = NULL;
		return 2;
	}

	hMutex = CreateMutex(
		NULL,			// параметры безопасности атрибута
		FALSE,			// начальное состояние
		NAME_MUTEX);	// имя объекта

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
	if (!GetStatusInit()) // Выход, если SharedBuffer не инициализирован
	{
		if (OUT_pszResponse) swprintf_s(OUT_pszResponse, MAX_PATH, _T("ResetBuf - ERROR! Необходима инициализация общего буфера!\n"));
		return 1;
	}

	CString temp;

	_SB_GLOBAL SBG;
	memset(&SBG, 0, sizeof(_SB_GLOBAL)); // Сброс всех полей SharedBuf

										 //--- Заполнение
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

	if (OUT_pszResponse) swprintf_s(OUT_pszResponse, MAX_PATH, _T("Произведен сброс полей общей памяти!\n"));
	return 0;
}

int CSharedBuffer::SetBuf(BYTE * IN_buf, DWORD byte_offset, DWORD byte_size, TCHAR * OUT_pszResponse)
{
	if (IN_buf == 0)
	{
		if (OUT_pszResponse) swprintf_s(OUT_pszResponse, MAX_PATH, _T("SetBuf - ERROR! Размер IN_BUF = 0!\n"));
		return 1;
	}

	if (hMapFile == NULL || hMutex == NULL || pBuf == 0 || m_StatInit == FALSE)
	{
		if (OUT_pszResponse) swprintf_s(OUT_pszResponse, MAX_PATH, _T("SetBuf - ERROR! Необходимо инициализировать общую память!\n"));
		return 2;
	}

	if (byte_size == 0)
	{
		if (OUT_pszResponse) swprintf_s(OUT_pszResponse, MAX_PATH, _T("SetBuf - ERROR! Некорректный размер передаваемого буфера! portion = %d!\n"), byte_size);
		return 3;
	}

	DWORD buf_sum = byte_offset + byte_size;
	if (buf_sum > m_Buf_size)
	{
		if (OUT_pszResponse) swprintf_s(OUT_pszResponse, MAX_PATH, _T("SetBuf - ERROR! Некорректные параметры! offset:%d portion:%d | buf_sum:%d > max_size:%d!\n"), byte_offset, byte_size, buf_sum, m_Buf_size);
		return 4;
	}

	// Ожидаем мьютекс и закрываем
	if (WaitForSingleObject(hMutex, INFINITE) != WAIT_OBJECT_0)
	{
		if (OUT_pszResponse) swprintf_s(OUT_pszResponse, MAX_PATH, _T("SetBuf - ERROR! Invalid hMutex!\n"));
		return 5;
	}

	memcpy(&pBuf[byte_offset], IN_buf, byte_size);

	ReleaseMutex(hMutex); // Освобождаем mutex

	return 0;
}

void CSharedBuffer::UpdateBuf()
{
	// Если PulseEvent вызывается для события со сбросом в ручную, то все потоки, ожидающие этот объект, получают управление
	m_EventMySending->SetEvent(); // Мы же отправили сообщение - не разбирать его
	m_EventGetMsg->PulseEvent(); // Мы отправили сообщение в общий буфер
}

int CSharedBuffer::GetBuf(BYTE * OUT_buf, DWORD byte_offset, DWORD byte_size, TCHAR * OUT_pszResponse)
{
	if (OUT_buf == 0)
	{
		if (OUT_pszResponse) swprintf_s(OUT_pszResponse, MAX_PATH, _T("GetBuf - ERROR! Размер OUT_buf = 0!\n"));
		return 1;
	}

	if (hMapFile == NULL || hMutex == NULL || pBuf == 0 || m_StatInit == FALSE)
	{
		if (OUT_pszResponse) swprintf_s(OUT_pszResponse, MAX_PATH, _T("GetBuf - ERROR! Необходима инициализация общей памяти!\n"));
		return 2;
	}

	if (byte_size == 0)
	{
		if (OUT_pszResponse) swprintf_s(OUT_pszResponse, MAX_PATH, _T("GetBuf - ERROR! Некорректный размер передаваемого буфера! portion:%d == 0!\n"), byte_size);
		return 3;
	}

	DWORD buf_sum = byte_offset + byte_size;
	if (buf_sum > m_Buf_size)
	{
		if (OUT_pszResponse) swprintf_s(OUT_pszResponse, MAX_PATH, _T("GetBuf - ERROR! Некорректные параметры! offset:%d portion:%d | buf_sum:%d > max_size:%d!\n"), byte_offset, byte_size, buf_sum, m_Buf_size);
		return 4;
	}

	// Ожидаем мьютекс и закрываем
	if (WaitForSingleObject(hMutex, INFINITE) != WAIT_OBJECT_0)
	{
		if (OUT_pszResponse) swprintf_s(OUT_pszResponse, MAX_PATH, _T("GetBuf - ERROR! Invalid hMutex!\n"));
		return 5;
	}

	memcpy(OUT_buf, &pBuf[byte_offset], byte_size);

	ReleaseMutex(hMutex); // Освобождаем

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
	// Проверяем был ли инициализирован поток и не было ли при этом ошибки
	if (m_pThread != NULL && m_pThread != INVALID_HANDLE_VALUE)
	{
		m_EventExit->SetEvent(); // Взвод события выхода
		WaitForSingleObject(m_pThread->m_hThread, INFINITE); // Ожидаем завершение потока
		delete m_pThread; // Удаляем память с указателя на поток
		m_pThread = NULL; // Зануляем указатель
	}
}

int CSharedBuffer::StartCycleGet()
{
	if (!m_StatInit) return 1;
	ClearThread(); // Если поток был запущен - остановить его

	// Запуск потока
	m_EventExit->ResetEvent(); // Сброс события выхода
	m_pThread = AfxBeginThread(Thread, this, 0, 0, CREATE_SUSPENDED, NULL);
	m_pThread->m_bAutoDelete = false; // Отключить автоудаление
	m_pThread->ResumeThread(); // Старт потока
	return 0;
}

UINT CSharedBuffer::Thread(LPVOID pParam)
{
	((CSharedBuffer*)pParam)->Run(); // Запускаем в поток функцию Run, которую необходимо переопределить
	AfxEndThread(0, FALSE); // Не удаляем поток при выходе из него
	return 0;
}

void CSharedBuffer::Run()
{
	// Массив событий по которым производится проверка
	DWORD res = 0;
	CSyncObject *pWaitObjects[] = { m_EventExit, m_EventGetMsg, m_EventMySending };
	CMultiLock MultiLock(pWaitObjects, 3L); // Мультилок на 3 события
	while (1)
	{
		// Блокировка с ожидание одного из событий
		res = MultiLock.Lock(INFINITE, FALSE);

		if (res == WAIT_OBJECT_0 + 1) // Мы получили сообщение
		{
			m_hostWindow->PostMessageW(WM_SB_CYCLE_GET_MSG, 0, 0);
			TRACE(_T("RUN: GET_MSG!\n"));
			continue;
		}
		else if (res == WAIT_OBJECT_0 + 2) // Мы сами отправили сообщение - игнорируем
		{
			m_EventMySending->ResetEvent();
			TRACE(_T("RUN: MY_SEND!\n"));
			continue;
		}
		else // Выход
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
	// Инициализация полей общей памяти
	if (m_pSB->Init(sizeof(_SB_GLOBAL), pszResponse) != 0)
	{
		if (OUT_pszResponse) swprintf_s(OUT_pszResponse, MAX_PATH, _T("CSBManagerMaster - Ошибка инициализации %s\n"), pszResponse);
		return 1;
	}
	else
	{
		if (OUT_pszResponse) swprintf_s(OUT_pszResponse, MAX_PATH, _T("CSBManagerMaster - Инициализация!\n"));
	}
	return 0;
}

int CSBManagerMaster::SB_Reset(TCHAR * OUT_pszResponse)
{
	TCHAR pszResponse[MAX_PATH] = { 0 };
	// Сброс полей общей памяти
	if (m_pSB->ResetBuf(pszResponse) != 0)
	{
		if (OUT_pszResponse) swprintf_s(OUT_pszResponse, MAX_PATH, _T("CSBManagerMaster - Ошибка сброса полей:%s\n"), pszResponse);
		return 1;
	}
	else
	{
		if (OUT_pszResponse) swprintf_s(OUT_pszResponse, MAX_PATH, _T("CSBManagerMaster - Сброс полей!\n"));
	}
	return 0;
}

int CSBManagerMaster::SB_CycleGet(bool start, TCHAR * OUT_pszResponse)
{
	if (start)
	{
		if (m_pSB->StartCycleGet() != 0)
		{
			if (OUT_pszResponse) swprintf_s(OUT_pszResponse, MAX_PATH, _T("CSBManagerMaster - Для SB_CycleGet необходима инициализация SharedBuffer!\n"));
			return 1;
		}
		else
		{
			if (OUT_pszResponse) swprintf_s(OUT_pszResponse, MAX_PATH, _T("CSBManagerMaster - Запуск потока приема сообщений!\n"));
		}
	}
	else
	{
		m_pSB->ClearThread();
		if (OUT_pszResponse) swprintf_s(OUT_pszResponse, MAX_PATH, _T("CSBManagerMaster - Остановка потока приема сообщений!\n"));
	}

	return 0;
}

int CSBManagerMaster::SearchDeviceRequest(TCHAR * OUT_pszResponse)
{
	//--- Отправка спец запроса (1-CONTROL,0-type,255-num) для обнаружения устройств
	// Считать поле из общей памяти
	if (m_pSB->GetBuf((BYTE*)&m_SBG.hdr, _SB_OFFSET().hdr, sizeof(_SB_HEADER), OUT_pszResponse) != 0) return 1;
	// Заполнение полей
	m_SBG.hdr.change_msg = 1; // Тип сообщения изменившего поле
	m_SBG.hdr.dev.type = 0; // Тип устройства изменившего поле
	m_SBG.hdr.dev.num = 255; // Номер устройства изменившего поле
	m_SBG.hdr.check_sum = m_pSB->GetCheckSum((BYTE*)&m_SBG.hdr, sizeof(_SB_HEADER));
	// Записать поле в общую память
	if (m_pSB->SetBuf((BYTE*)&m_SBG.hdr, _SB_OFFSET().hdr, sizeof(_SB_HEADER), OUT_pszResponse) != 0) return 2;
	// Отправка команды на считывания общего буфера
	m_pSB->UpdateBuf(); // Отправить событие обновление буфера
	return 0;
}

int CSBManagerMaster::InitDeviceInfo(TCHAR * OUT_pszResponse)
{
	_SB_INFO *info = (_SB_INFO*)&m_SBG.info;
	// Считать поле из общей памяти
	if (m_pSB->GetBuf((BYTE*)info, _SB_OFFSET().info, sizeof(_SB_INFO), OUT_pszResponse) != 0) return 1;

	_SBM_DEVICE m_SBMDT; memset(&m_SBMDT, 0, sizeof(_SBM_DEVICE));
	m_vDevice.clear();

	int offset_control = 0;
	int offset_reply = 0;
	int size_control = 0;
	int size_reply = 0;

	CString msg, temp;
	msg = _T("Устройства: ");
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
		if (OUT_pszResponse) swprintf_s(OUT_pszResponse, MAX_PATH, _T("CSBManagerSlave - ERROR! Информационный буфер устройств пуст!\n"));
		return 1;
	}

	// Поиск устройства по типу и номеру
	int n = -1;
	for (size_t i = 0; i < m_vDevice.size(); i++)
	{
		if (type == m_vDevice.at(i).type && num == m_vDevice.at(i).num)
		{
			n = i;
			break;
		}
	}
	if (n == -1) // Если не нашли нужный тип и номер
	{
		if (OUT_pszResponse) swprintf_s(OUT_pszResponse, MAX_PATH, _T("CSBManagerSlave - ERROR! GetDevStat не нашел подключенного устройства type:%d num:%d\n"), type, num);
		return 2;
	}

	// Заполнение структуры устройства
	if (dev)
	{
		dev->type = type;
		dev->num = num;
		dev->mode = mode;
		dev->state = state;
		dev->control_size = m_vDevice.at(n).control_size;
		dev->reply_size = m_vDevice.at(n).reply_size;
	}

	numDev = n; // Передаем номер найденного устройства

	return 0;
}

//---
int CSBManagerMaster::SetMsg_HDR(int change_msg, _SB_DEV_STATUS * dev, TCHAR * OUT_pszResponse)
{
	// Считать поле из общей памяти
	if (m_pSB->GetBuf((BYTE*)&m_SBG.hdr, _SB_OFFSET().hdr, sizeof(_SB_HEADER), OUT_pszResponse) != 0) return 1;
	// Заполнение полей
	m_SBG.hdr.change_msg = change_msg; // Тип сообщения изменившего поле
	if (dev) memcpy(&m_SBG.hdr.dev, dev, sizeof(_SB_DEV_STATUS));
	else memset(&m_SBG.hdr.dev, 0, sizeof(_SB_DEV_STATUS));
	// Расчет контрольной суммы
	m_SBG.hdr.check_sum = m_pSB->GetCheckSum((BYTE*)&m_SBG.hdr, sizeof(_SB_HEADER));
	// Записать поле в общую память
	if (m_pSB->SetBuf((BYTE*)&m_SBG.hdr, _SB_OFFSET().hdr, sizeof(_SB_HEADER), OUT_pszResponse) != 0) return 2;
	// Отправка команды на считывания общего буфера
	m_pSB->UpdateBuf();
	return 0;
}

int CSBManagerMaster::GetMsg_HDR(int & change_msg, int & numDev, _SB_DEV_STATUS *dev, TCHAR * OUT_pszResponse)
{
	_SB_HEADER *hdr = (_SB_HEADER*)&m_SBG.hdr;
	// Считать поле из общей памяти
	if (m_pSB->GetBuf((BYTE*)hdr, _SB_OFFSET().hdr, sizeof(_SB_HEADER), OUT_pszResponse) != 0) return 1;

	//TRACE(_T("change:%d | type:%d num:%d mode:%d state:%d | sz_c:%d sz_r:%d\n"), hdr->change_msg, hdr->dev.type, hdr->dev.num, hdr->dev.mode, hdr->dev.state, hdr->dev.control_size, hdr->dev.reply_size);

	if (hdr->change_msg == 0)
	{
		if (OUT_pszResponse) swprintf_s(OUT_pszResponse, MAX_PATH, _T("CSBManagerSlave - ERROR! GetMsg_HDR - Некорректный тип поля %d!\n"), hdr->change_msg);
		return 1;
	}

	change_msg = hdr->change_msg;

	if (hdr->dev.type != 0) // Специальный запрос
	{
		if (GetDevStatus(dev, numDev, hdr->dev.type, hdr->dev.num, hdr->dev.mode, hdr->dev.state, OUT_pszResponse) != 0)
		{
			if (OUT_pszResponse) swprintf_s(OUT_pszResponse, MAX_PATH, _T("CSBManagerSlave - ERROR! GetDevStatus - Некорректный type:%d num:%d!\n"), hdr->dev.type, hdr->dev.num);
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
		if (OUT_pszResponse) swprintf_s(OUT_pszResponse, MAX_PATH, _T("CSBManagerSlave - ERROR! Информационный буфер устройств пуст!\n"));
		return 1;
	}

	_SB_CONTROL *ctrl = (_SB_CONTROL*)&m_SBG.control;
	// Считать поле из общей памяти
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
			if (OUT_pszResponse) swprintf_s(OUT_pszResponse, MAX_PATH, _T("CSBManagerMaster - ERROR! SetMsg_CONTROL номер параметра [%d] вне границ выделенных элементов [%d]!\n"), nt, elem);
			return 4;
		}
	}

	// Пересчитываем контрольную сумму
	ctrl->check_sum = m_pSB->GetCheckSum((BYTE*)ctrl, sizeof(_SB_CONTROL));

	// Записать поле в общую память
	if (m_pSB->SetBuf((BYTE*)ctrl, _SB_OFFSET().control, sizeof(_SB_CONTROL), OUT_pszResponse) != 0) return 5;

	// Обновление поля HDR + отправка
	if (SetMsg_HDR(1, dev, OUT_pszResponse) != 0) return 6;

	return 0;
}

int CSBManagerMaster::GetMsg_INFO(_SB_DEV_STATUS * dev, int numDev, TCHAR * OUT_pszResponse)
{
	if (m_vDevice.size() == 0)
	{
		if (OUT_pszResponse) swprintf_s(OUT_pszResponse, MAX_PATH, _T("CSBManagerSlave - ERROR! Информационный буфер устройств пуст!\n"));
		return 1;
	}

	_SB_INFO *info = (_SB_INFO*)&m_SBG.info;

	// Считать поле из общей памяти
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
			if (OUT_pszResponse) swprintf_s(OUT_pszResponse, MAX_PATH, _T("CSBManagerMaster - ERROR! GetMsg_INFO не нашел подключенного устройства type:%d num:%d\n"), type, num);
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
		if (OUT_pszResponse) swprintf_s(OUT_pszResponse, MAX_PATH, _T("CSBManagerMaster - ERROR! Информационный буфер устройств пуст!\n"));
		return 1;
	}

	_SB_REPLY *repl = (_SB_REPLY*)&m_SBG.reply;

	// Считать поле из общей памяти
	if (m_pSB->GetBuf((BYTE*)repl, _SB_OFFSET().reply, sizeof(_SB_REPLY), OUT_pszResponse) != 0) return 2;

	int elem = m_vDevice.at(numDev).reply_size / sizeof(float); // Кол-во считываемых элементов
	float *pf = (float*)&repl->BUF[m_vDevice.at(numDev).reply_offset]; // Указатель на начало данных

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

	// Считать поле из общей памяти
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
	// Инициализация полей общей памяти
	if (m_pSB->Init(sizeof(_SB_GLOBAL), pszResponse) != 0)
	{
		if (OUT_pszResponse) swprintf_s(OUT_pszResponse, MAX_PATH, _T("CSBManagerSlave - Ошибка инициализации %s\n"), pszResponse);
		return 1;
	}
	else
	{
		if (OUT_pszResponse) swprintf_s(OUT_pszResponse, MAX_PATH, _T("CSBManagerSlave - Инициализация!\n"));
	}
	return 0;
}

int CSBManagerSlave::SB_Reset(TCHAR * OUT_pszResponse)
{
	TCHAR pszResponse[MAX_PATH] = { 0 };
	// Сброс полей общей памяти
	if (m_pSB->ResetBuf(pszResponse) != 0)
	{
		if (OUT_pszResponse) swprintf_s(OUT_pszResponse, MAX_PATH, _T("CSBManagerSlave - Ошибка сброса полей:%s\n"), pszResponse);
		return 1;
	}
	else
	{
		if (OUT_pszResponse) swprintf_s(OUT_pszResponse, MAX_PATH, _T("CSBManagerSlave - Сброс полей!\n"));
	}
	return 0;
}

int CSBManagerSlave::SB_CycleGet(bool start, TCHAR * OUT_pszResponse)
{
	if (start)
	{
		if (m_pSB->StartCycleGet() != 0)
		{
			if (OUT_pszResponse) swprintf_s(OUT_pszResponse, MAX_PATH, _T("CSBManagerSlave - Для SB_CycleGet необходима инициализация SharedBuffer!\n"));
			return 1;
		}
		else
		{
			if (OUT_pszResponse) swprintf_s(OUT_pszResponse, MAX_PATH, _T("CSBManagerSlave - Запуск потока приема сообщений!\n"));
		}
	}
	else
	{
		m_pSB->ClearThread();
		if (OUT_pszResponse) swprintf_s(OUT_pszResponse, MAX_PATH, _T("CSBManagerSlave - Остановка потока приема сообщений!\n"));
	}

	return 0;
}

//---
int CSBManagerSlave::SearchDeviceAnswer(_SB_DEV_STATUS *dev, int dev_num, TCHAR * OUT_pszResponse)
{
	_SB_INFO *info = (_SB_INFO*)&m_SBG.info;

	// Считать поле из общей памяти
	if (m_pSB->GetBuf((BYTE*)info, _SB_OFFSET().info, sizeof(_SB_INFO), OUT_pszResponse) != 0) return 1;

	info->number_dev = dev_num;
	memset(info->DEV_STAT, 0, sizeof(_SB_INFO::DEV_STAT));
	memcpy(info->DEV_STAT, dev, dev_num * sizeof(_SB_DEV_STATUS));

	//--- Заполнение вектора устройств информацией смещения+размер
	_SBM_DEVICE m_SBMDT; memset(&m_SBMDT, 0, sizeof(_SBM_DEVICE));
	m_vDevice.clear();

	int offset_control = 0;
	int offset_reply = 0;
	int size_control = 0;
	int size_reply = 0;

	// Заполнение значений смещения и размера для CONTROL & REPLY
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
	// Расчет контрольной суммы
	info->check_sum = m_pSB->GetCheckSum((BYTE*)info, sizeof(_SB_INFO));

	// Записать поле в общую память
	if (m_pSB->SetBuf((BYTE*)info, _SB_OFFSET().info, sizeof(_SB_INFO), OUT_pszResponse) != 0) return 2;
	
	// Обновление поля HDR
	if (SetMsg_HDR(2, 0, OUT_pszResponse) != 0) return 3;

	return 0;
}

int CSBManagerSlave::GetDevStatus(_SB_DEV_STATUS *dev, int &numDev, int type, int num, int mode, int state, TCHAR *OUT_pszResponse)
{
	if (m_vDevice.size() == 0)
	{
		if (OUT_pszResponse) swprintf_s(OUT_pszResponse, MAX_PATH, _T("CSBManagerSlave - ERROR! Информационный буфер устройств пуст!\n"));
		return 1;
	}

	// Поиск устройства по типу и номеру
	int n = -1;
	for (size_t i = 0; i < m_vDevice.size(); i++)
	{
		if (type == m_vDevice.at(i).type && num == m_vDevice.at(i).num)
		{
			n = i;
			break;
		}
	}
	if (n == -1) // Если не нашли нужный тип и номер
	{
		if (OUT_pszResponse) swprintf_s(OUT_pszResponse, MAX_PATH, _T("CSBManagerSlave - ERROR! GetDevStat не нашел подключенного устройства type:%d num:%d\n"), type, num);
		return 2;
	}

	// Заполнение структуры устройства
	if (dev)
	{
		dev->type = type;
		dev->num = num;
		dev->mode = mode;
		dev->state = state;
		dev->control_size = m_vDevice.at(n).control_size;
		dev->reply_size = m_vDevice.at(n).reply_size;
	}

	numDev = n; // Передаем номер найденного устройства

	return 0;
}

//---
int CSBManagerSlave::SetMsg_HDR(int change_msg, _SB_DEV_STATUS * dev, TCHAR * OUT_pszResponse)
{
	// Считать поле из общей памяти
	if (m_pSB->GetBuf((BYTE*)&m_SBG.hdr, _SB_OFFSET().hdr, sizeof(_SB_HEADER), OUT_pszResponse) != 0) return 1;
	// Заполнение полей
	m_SBG.hdr.change_msg = change_msg; // Тип сообщения изменившего поле
	if (dev) memcpy(&m_SBG.hdr.dev, dev, sizeof(_SB_DEV_STATUS));
	else memset(&m_SBG.hdr.dev, 0, sizeof(_SB_DEV_STATUS));
	// Расчет контрольной суммы
	m_SBG.hdr.check_sum = m_pSB->GetCheckSum((BYTE*)&m_SBG.hdr, sizeof(_SB_HEADER));
	// Записать поле в общую память
	if (m_pSB->SetBuf((BYTE*)&m_SBG.hdr, _SB_OFFSET().hdr, sizeof(_SB_HEADER), OUT_pszResponse) != 0) return 2;
	// Отправка команды на считывания общего буфера
	m_pSB->UpdateBuf();
	return 0;
}

int CSBManagerSlave::GetMsg_HDR(int & change_msg, int & numDev, _SB_DEV_STATUS * dev, TCHAR * OUT_pszResponse)
{
	_SB_HEADER *hdr = (_SB_HEADER*)&m_SBG.hdr;
	// Считать поле из общей памяти
	if (m_pSB->GetBuf((BYTE*)hdr, _SB_OFFSET().hdr, sizeof(_SB_HEADER), OUT_pszResponse) != 0) return 1;

	//TRACE(_T("change:%d | type:%d num:%d mode:%d state:%d | sz_c:%d sz_r:%d\n"), hdr->change_msg, hdr->dev.type, hdr->dev.num, hdr->dev.mode, hdr->dev.state, hdr->dev.control_size, hdr->dev.reply_size);

	if (hdr->change_msg == 0)
	{
		if (OUT_pszResponse) swprintf_s(OUT_pszResponse, MAX_PATH, _T("CSBManagerSlave - ERROR! GetMsg_HDR - Некорректный тип поля %d!\n"), hdr->change_msg);
		return 1;
	}

	change_msg = hdr->change_msg;

	if (hdr->dev.type != 0) // Специальный запрос
	{
		if (GetDevStatus(dev, numDev, hdr->dev.type, hdr->dev.num, hdr->dev.mode, hdr->dev.state, OUT_pszResponse) != 0)
		{
			if (OUT_pszResponse) swprintf_s(OUT_pszResponse, MAX_PATH, _T("CSBManagerSlave - ERROR! GetDevStatus - Некорректный type:%d num:%d!\n"), hdr->dev.type, hdr->dev.num);
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
		if (OUT_pszResponse) swprintf_s(OUT_pszResponse, MAX_PATH, _T("CSBManagerSlave - ERROR! Информационный буфер устройств пуст!\n"));
		return 1;
	}

	_SB_CONTROL *ctrl = (_SB_CONTROL*)&m_SBG.control;
	
	// Считать поле из общей памяти
	if (m_pSB->GetBuf((BYTE*)ctrl, _SB_OFFSET().control, sizeof(_SB_CONTROL), OUT_pszResponse) != 0) return 2;

	int elem = m_vDevice.at(numDev).control_size / sizeof(float); // Кол-во считываемых элементов
	float *pf = (float*)&ctrl->BUF[m_vDevice.at(numDev).control_offset]; // Указатель на начало данных

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
		if (OUT_pszResponse) swprintf_s(OUT_pszResponse, MAX_PATH, _T("CSBManagerSlave - ERROR! Информационный буфер устройств пуст!\n"));
		return 1;
	}

	_SB_INFO *info = (_SB_INFO*)&m_SBG.info;

	// Считать поле из общей памяти
	if (m_pSB->GetBuf((BYTE*)info, _SB_OFFSET().info, sizeof(_SB_INFO), OUT_pszResponse) != 0) return 2;

	info->DEV_STAT[numDev].mode = dev->mode;
	info->DEV_STAT[numDev].state = dev->state;

	// Расчет контрольной суммы
	info->check_sum = m_pSB->GetCheckSum((BYTE*)info, sizeof(_SB_INFO));

	// Записать поле в общую память
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
		if (OUT_pszResponse) swprintf_s(OUT_pszResponse, MAX_PATH, _T("CSBManagerSlave - ERROR! Информационный буфер устройств пуст!\n"));
		return 1;
	}

	_SB_REPLY *repl = (_SB_REPLY*)&m_SBG.reply;

	// Считать поле из общей памяти
	if (m_pSB->GetBuf((BYTE*)repl, _SB_OFFSET().reply, sizeof(_SB_REPLY), OUT_pszResponse) != 0) return 2;

	int elem = m_vDevice.at(numDev).reply_size / sizeof(float); // Кол-во считываемых элементов
	float *pf = (float*)&repl->BUF[m_vDevice.at(numDev).reply_offset]; // Указатель на начало данных

	if (param_num != elem)
	{
		if (OUT_pszResponse) swprintf_s(OUT_pszResponse, MAX_PATH, _T("CSBManagerSlave - ERROR! SetMsg_REPLY - Количество параметров [%d] не равно длине поля ответа [%d]! \n"), param_num,elem);
		return 3;
	}

	memcpy(pf, pParam, elem*sizeof(float)); // Копирование параметров

	// Расчет контрольной суммы
	repl->check_sum = m_pSB->GetCheckSum((BYTE*)repl, sizeof(_SB_REPLY));

	// Записать поле в общую память
	if (m_pSB->SetBuf((BYTE*)repl, _SB_OFFSET().reply, sizeof(_SB_REPLY), OUT_pszResponse) != 0) return 3;

	// Заполняем HDR + отправка
	if (SetMsg_HDR(3, dev, OUT_pszResponse) != 0) return 3;

	return 0;
}

int CSBManagerSlave::SetMsg_ERROR(_SB_DEV_STATUS *dev, int numDev, int cod_error, int more_error, TCHAR * str_error, int str_sz, TCHAR * OUT_pszResponse)
{
	_SB_ERROR *err = (_SB_ERROR*)&m_SBG.error;

	// Считать поле из общей памяти
	if (m_pSB->GetBuf((BYTE*)err, _SB_OFFSET().error, sizeof(_SB_ERROR), OUT_pszResponse) != 0) return 1;

	err->type = (uint8_t)dev->type;
	err->num = (uint8_t)dev->num;
	err->error_cod = (int8_t)cod_error;
	err->error_more = (int8_t)more_error;
	int max = sizeof(_SB_ERROR::error_str)/2;
	if (str_sz > max) str_sz = max;
	swprintf_s(err->error_str, str_sz, _T("%s"), str_error);

	// Расчет контрольной суммы
	err->check_sum = m_pSB->GetCheckSum((BYTE*)err, sizeof(_SB_ERROR));

	// Записать поле в общую память
	if (m_pSB->SetBuf((BYTE*)err, _SB_OFFSET().error, sizeof(_SB_ERROR), OUT_pszResponse) != 0) return 2;

	// Заполняем HDR + отправка
	if (SetMsg_HDR(4, dev, OUT_pszResponse) != 0) return 3;

	return 0;
}