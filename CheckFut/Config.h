#pragma once
class Config
{
private:
	TCHAR m_sConfigPath[MAX_PATH];			// ���� � ����� ������������
public:
	Config();
	~Config();

	int CheckExist();				// �������� ������������� �����
	int ReadConfig();				// ������� ������������ � ���������
	int SaveConfig();				// �������� ������������ � ����
};

