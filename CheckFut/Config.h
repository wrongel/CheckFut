#pragma once
class Config
{
private:
	TCHAR m_sConfigPath[MAX_PATH];			// Путь к файлу конфигурации
public:
	Config();
	~Config();

	int CheckExist();				// Проверка существования файла
	int ReadConfig();				// Считать конфигурацию в структуру
	int SaveConfig();				// Записать конфигурацию в файл
};

