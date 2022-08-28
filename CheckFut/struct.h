#pragma once

const UINT WM_SB_SET_CONTROL = WM_APP + 101; // wparam = номер устройства, lparam = указатель на структуру

#pragma pack(1)

typedef struct
{
	float type;		// Тип устройства
	float num;		// Номер устройства
	float elem_num; // Количество элементов
	float mode;		// Режим работы устройства
	float state;	// Состояние устройства (connect, power и тд)
	float PV;		// Установка значения напряжения
	float PC;		// Установка значения тока
	float PVpMin;	// Установка минимального порога напряжения
	float PVpMax;	// Установка максимального порога напряжения
	float PCpMin;	// Установка минимального порога тока
	float PCpMax;	// Установка максимального порога тока
} _SBM_POWER_SUPPLY;

#pragma pack(8)