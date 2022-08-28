#pragma once

const UINT WM_SB_SET_CONTROL = WM_APP + 101; // wparam = ����� ����������, lparam = ��������� �� ���������

#pragma pack(1)

typedef struct
{
	float type;		// ��� ����������
	float num;		// ����� ����������
	float elem_num; // ���������� ���������
	float mode;		// ����� ������ ����������
	float state;	// ��������� ���������� (connect, power � ��)
	float PV;		// ��������� �������� ����������
	float PC;		// ��������� �������� ����
	float PVpMin;	// ��������� ������������ ������ ����������
	float PVpMax;	// ��������� ������������� ������ ����������
	float PCpMin;	// ��������� ������������ ������ ����
	float PCpMax;	// ��������� ������������� ������ ����
} _SBM_POWER_SUPPLY;

#pragma pack(8)