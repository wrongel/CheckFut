#pragma once

#include <stdint.h>
#include <vector>

#define VERSION_SHARED_BUFFER 101

// Сигналы для WindowProc(UINT message, WPARAM wParam, LPARAM lParam)
const UINT WM_SB_CYCLE_GET_MSG = WM_APP + 1000; // Сигнал получения сообщения в цикле

//--- Объявление структур полей для общей памяти
#pragma pack(1)

//--- Параметр устройства | size:6
typedef struct _SBM_PARAM
{
	int   num;					// Номер
	float value;				// Значение
	_SBM_PARAM(int number=0, float val=0) : num(number), value(val) {} // Конструктор
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

//--- Статус устройства | size:8
typedef struct
{
	uint8_t	 type;				// Тип устройства
	uint8_t	 num;				// Номер устройства
	uint8_t	 mode;				// Режим работы
	uint8_t	 state;				// Состояние (connect, power и тд.)
	uint16_t control_size;		// Размер поля управления в байтах
	uint16_t reply_size;		// Размер поля ответа в байтах
} _SB_DEV_STATUS;

//--- Заголовок полей | size:24
typedef struct
{
	uint32_t id_header;			// Идентификатор заголовка / 0x52444548 / "HEDR"
	uint16_t version_prot;		// Версия протокола
	uint16_t global_size;		// Общий размер
	uint8_t	 change_msg;		// Тип сообщения изменившего поле	"1-CONTROL 2-INFO 3-REPLY 4-ERROR"
	_SB_DEV_STATUS dev;			// Устройство обновившее память
	int8_t   reserve[3];		// Резерв
	uint32_t check_sum;			// Контрольная сумма
} _SB_HEADER;

//--- Управляющие поле | size:8200
typedef struct
{
	uint32_t id_control;		// Идентификатор управления	/ 0x4C525443 / “CTRL”
	uint8_t  BUF[8192];			// Поле байт для управляющих команд
	uint32_t check_sum;			// Контрольная сумма
} _SB_CONTROL;

//--- Информационное поле | size:500
typedef struct
{
	uint32_t id_info;			// Идентификатор информации / 0x4F464E49 / “INFO”
	uint16_t number_dev;		// Количество устройств
	_SB_DEV_STATUS DEV_STAT[60];// Массив структур статусов устройств
	int8_t   reserve[10];		// Резерв
	uint32_t check_sum;			// Контрольная сумма
} _SB_INFO;

//--- Ответное поле | size:8200
typedef struct
{
	uint32_t id_reply;			// Идентификатор ответа / 0x4C504552 / “REPL”
	uint8_t  BUF[8192];			// Поле байт для ответных команд
	uint32_t check_sum;			// Контрольная сумма
} _SB_REPLY;

//--- Поле ошибок | size:1040
typedef struct
{
	uint32_t id_error;			// Идентификатор ошибки / 0x21525245 / “ERR!”
	uint8_t type;				// Тип устройства
	uint8_t	num;				// Номер устройства
	int8_t error_cod;			// Код ошибки
	int8_t error_more;			// Доп.код ошибки
	TCHAR error_str[512];		// Строка ошибки
	int8_t reserve[4];			// Резерв
	uint32_t check_sum;			// Контрольная сумма
} _SB_ERROR;

//--- Общая структура полей | size:17964 
typedef struct
{
	_SB_HEADER  hdr;			// Поле заголовка
	_SB_CONTROL control;		// Поле управляющих команд
	_SB_INFO    info;			// Поле информации об устройствах
	_SB_REPLY   reply;			// Поле ответных команд
	_SB_ERROR   error;			// Поле ошибки
} _SB_GLOBAL;

//--- Смещения
typedef struct
{
	int hdr = 0;
	int control = sizeof(_SB_HEADER);
	int info = sizeof(_SB_HEADER) + sizeof(_SB_CONTROL);
	int reply = sizeof(_SB_HEADER) + sizeof(_SB_CONTROL) + sizeof(_SB_INFO);
	int error = sizeof(_SB_HEADER) + sizeof(_SB_CONTROL) + sizeof(_SB_INFO) + sizeof(_SB_REPLY);
} _SB_OFFSET;

#pragma pack(8)

//--- Абстрактный класс CSharedBuffer для работы с общей памятью
class CSharedBuffer
{
public:
	CSharedBuffer(CWnd *hostWindow); // Конструктор класса
	~CSharedBuffer(); // Деструктор класса

	// >INITIALIZATION - Инициализация
	// >Входные значения 
	// byte_size - размер инициализируемого буфера в байтах
	// pszUnicName - уникальная добавка к глобальным именам SharedMemory для создания своего отдельного буфера
	// >Выходные значения
	// OUT_pszResponse - ответ устройства (может быть NULL)
	// >return
	// 0 - Инициализация прошла успешно
	// 1 - Не удалось инициализировать хендл для FileMap
	// 2 - Не удалось инициализировать буфер на SharedMemory
	// 3 - Не удалось создать mutex для блокировки обращения к данным
	int Init(DWORD byte_size, TCHAR *OUT_pszResponse = NULL); // Инициализация

	// >DESTROY - Уничтожение
	void Destroy(); // Уничтожение

	// >RESET BUF - Сброс полей общего буфера
	// >Выходные значения
	// OUT_pszResponse - ответ (может быть NULL)
	// >return
	// 0 - Сброс произведен
	// 1 - Общая память не инициализирована
	// 2 - Невозможно передать общее поле в общую память
	int ResetBuf(TCHAR *OUT_pszResponse = NULL); // Сброс параметров

	// >SET BUF - Копирования значения буфера в SharedMemory
	// >Входные значения 
	// IN_buf - указатель буфера, который будет скопирован в SharedMemory
	// byte_offset - смещение в байтах от начала SharedMemory
	// byte_size - размер инициализируемого буфера в байтах
	// >Выходные значения
	// OUT_pszResponse - ответ устройства (может быть NULL)
	// >return
	// 0 - Копирование прошло успешно
	// 1 - Копируемый буфер не инициализирован
	// 2 - Не была вызвана инициализация SharedMemory функцией Init(...)
	// 3 - Размер копируемых данных равен нулю
	// 4 - Выход за пределы выделенной памяти SharedMemory. byte_size+byte_offset > m_Buf_size
	// 5 - Ожидание mutex завершилось с ошибкой
	int SetBuf(BYTE *IN_buf, DWORD byte_offset, DWORD byte_size, TCHAR *OUT_pszResponse = NULL);

	// >UPDATE BUF - Отправка флага обновления буфера
	void UpdateBuf();

	// >GET BUF - Копирования значения из SharedMemory в буфер
	// >Входные значения 
	// byte_offset - смещение в байтах от начала SharedMemory
	// byte_size - размер инициализируемого буфера в байтах
	// >Выходные значения
	// OUT_buf - указатель буфера, в который будет скопирован SharedMemory
	// OUT_pszResponse - ответ устройства (может быть NULL)
	// >return
	// 0 - Копирование прошло успешно
	// 1 - Буфер назанчения не инициализирован
	// 2 - Не была вызвана инициализация SharedMemory функцией Init(...)
	// 3 - Размер копируемых данных равен нулю
	// 4 - Выход за пределы выделенной памяти SharedMemory. byte_size+byte_offset > m_Buf_size
	// 5 - Ожидание mutex завершилось с ошибкой
	int GetBuf(BYTE *OUT_buf, DWORD byte_offset, DWORD byte_size, TCHAR *OUT_pszResponse = NULL);

	// >GET STATUS INIT - Возвращает статус инициализация SharedBuf
	// >return
	// 0 - Инициализация не проводилась
	// 1 - Инициализация произведена
	BOOL GetStatusInit() { return m_StatInit; }

	// >GET CHECK SUM - Производит расчет контрольной суммы (сумма байт)
	// >Входные значения 
	// buf - указатель на буфер
	// buf_sz - размер байтов для расчета суммы
	// >return
	// UINT - результат контрольной суммы ввиде числа
	UINT GetCheckSum(BYTE *buf, size_t buf_sz);

	// THREAD
	int StartCycleGet(); // Запуск потока
	void ClearThread(); // Завершение потока

private:
	static UINT Thread(LPVOID pParam); // Методок потока
	void Run(); // Функция вызываемемая внутри потока
	
	CEvent *m_EventExit; // Событие выхода их потока ожидания
	CEvent *m_EventGetMsg; // События приема сообщения
	CEvent *m_EventMySending; // Событие что мы же и произвели отправку сообщения
	CWinThread *m_pThread; // Указатель на поток
	CWnd *m_hostWindow; // Указатель на host окно
	
	HANDLE hMapFile;			// Хэндл файла маппинга
	HANDLE hMutex;				// Хэндл мьютекса
	BOOL m_StatInit;			// Флаг статуса инициализации (Взвод Init(...), сброс Destroy())
	BYTE *pBuf;					// Указатель на буфер SharedMemory
	DWORD m_Buf_size;			// Размер буфера SharedMemory
};

//--- MASTER
//--- Класс управления со стороны Master
class CSBManagerMaster
{
public:
	CSBManagerMaster(CWnd * host); // Конструктор
	~CSBManagerMaster(); // Деструктор

	//--- SharedBuffer
	int SB_Init(TCHAR *OUT_pszResponse);	// Инициализация SharedBuffer
	int SB_Reset(TCHAR *OUT_pszResponse);   // Сброс SharedBuffer
	int SB_CycleGet(bool start, TCHAR *OUT_pszResponse); // Запуск потока приема сообщений от SharedBuffer

	//--- Устройства
	int SearchDeviceRequest(TCHAR *OUT_pszResponse = NULL); // Отправить команду на поиск подключенных устройств
	int InitDeviceInfo(TCHAR *OUT_pszResponse); // Инициализировать список устройств
	int GetDevStatus(_SB_DEV_STATUS *dev, int &numDev, int type, int num, int mode, int state, TCHAR *OUT_pszResponse); // Получить структуру и номер устройства

	//--- Взаимодействие с полями SharedBuffer
	int SetMsg_HDR(int change_msg, _SB_DEV_STATUS *dev, TCHAR *OUT_pszResponse); // Задать поле SB HEADER
	int GetMsg_HDR(int & change_msg, int & numDev, _SB_DEV_STATUS *dev=0, TCHAR * OUT_pszResponse = NULL);
	int SetMsg_CONTROL(_SB_DEV_STATUS *dev, int numDev, std::vector<_SBM_PARAM> &vParam, TCHAR *OUT_pszResponse);
	int GetMsg_INFO(_SB_DEV_STATUS *dev, int numDev, TCHAR *OUT_pszResponse);
	int GetMsg_REPLY(int numDev, std::vector<float> &vParam, TCHAR * OUT_pszResponse); // Считать поле SB REPLY
	int GetMsg_ERROR(int error_cod, int error_more, TCHAR *str_error, int str_sz, TCHAR *OUT_pszResponse); // Считать поле SB ERROR

private:
	CSharedBuffer *m_pSB;		// Указатель на класс CSharedBuffer
	_SB_GLOBAL m_SBG;			// Глобальный буфер для считывания сообщения

	std::vector<_SBM_DEVICE> m_vDevice; // Вектор инициализированных устройств
};

//--- SLAVE
class CSBManagerSlave
{
public:
	CSBManagerSlave(CWnd * host); // Конструктор
	~CSBManagerSlave(); // Деструктор

	//--- SharedBuffer
	int SB_Init(TCHAR *OUT_pszResponse);	// Инициализация SharedBuffer
	int SB_Reset(TCHAR *OUT_pszResponse);   // Сброс SharedBuffer
	int SB_CycleGet(bool start, TCHAR *OUT_pszResponse); // Запуск потока приема сообщений от SharedBuffer

	//--- Устройства
	int SearchDeviceAnswer(_SB_DEV_STATUS *dev, int dev_num, TCHAR *OUT_pszResponse = NULL); // Отправить ответ на поиск подключенных устройств
	int GetDevStatus(_SB_DEV_STATUS *dev, int &numDev, int type, int num, int mode, int state, TCHAR *OUT_pszResponse); // Получить структуру и номер устройства
	
	// Добавить проверку ID & CHECKSUM
	//--- Взаимодействие с полями SharedBuffer
	int SetMsg_HDR(int change_msg,  _SB_DEV_STATUS *dev, TCHAR *OUT_pszResponse); // Задать поле SB HEADER
	int GetMsg_HDR(int & change_msg, int & numDev, _SB_DEV_STATUS *dev = 0, TCHAR * OUT_pszResponse = NULL);
	int GetMsg_CONTROL(int numDev, std::vector<float> &vParam, TCHAR * OUT_pszResponse); // Задать поле SB CONTROL
	int SetMsg_INFO(_SB_DEV_STATUS *dev, int numDev, bool update, TCHAR *OUT_pszResponse); // Задать поле SB INFO
	int SetMsg_REPLY(_SB_DEV_STATUS *dev, int numDev, float *pParam, int param_num, TCHAR *OUT_pszResponse); // Задать поле SB REPLY
	int SetMsg_ERROR(_SB_DEV_STATUS *dev, int numDev, int cod_error, int more_error, TCHAR *str_error, int str_num, TCHAR *OUT_pszResponse); // Задать поле SB ERROR
	
private:
	CSharedBuffer *m_pSB;		// Указатель на класс CSharedBuffer
	_SB_GLOBAL m_SBG;			// Глобальный буфер для считывания сообщения

	std::vector<_SBM_DEVICE> m_vDevice; // Вектор инициализированных устройств
};