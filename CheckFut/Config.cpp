#include "stdafx.h"
#include "Config.h"
#include "CheckFut.hxx"

struct tagVers
{
	int Major;
	int	Minor;
	int	Build;
	int	Revision;
};
struct tagVers zVersion;

/////////////////////////////////////////////////////////////////////////////
Config::Config()
{
	for (int i = 0; i < NUMCHECKS; i++)
	{
		_tcscpy_s(g_zExeInfo[i].lpszCheckName, _T("\0"));
		_tcscpy_s(g_zExeInfo[i].lpszRepName,   _T("\0"));
		g_zExeInfo[i].nResult = -88;
		g_zExeInfo[i].nMode = -1;
		for (int j = 0; j < NUMPARAM; j++)
		{
			_tcscpy_s(g_zExeInfo[i].lpszPath,   _T("\0"));
			_tcscpy_s(g_zExeInfo[i].lpszCmdPar[j], _T("\0"));
		}
	}
}

/////////////////////////////////////////////////////////////////////////////
Config::~Config()
{

}

/////////////////////////////////////////////////////////////////////////////
int Config::CheckExist()
{
	CFileFind find;

	GetModuleFileName(NULL, m_sConfigPath, _countof(m_sConfigPath));
	m_sConfigPath[_tcsrchr(m_sConfigPath, '\\') - m_sConfigPath + 1] = '\0';
	_tcscat_s(m_sConfigPath, _T("config.dat"));

	return find.FindFile(m_sConfigPath, 0);
}

/////////////////////////////////////////////////////////////////////////////
int Config::ReadConfig()
{
	CStdioFile pFile;

	if (!pFile.Open(m_sConfigPath, CFile::modeRead | CFile::typeBinary))
		return 0;
	pFile.SeekToBegin();
	pFile.Read(&zVersion, sizeof(zVersion));
	pFile.Read(&g_zExeInfo, sizeof(g_zExeInfo));
	pFile.Close();

	_stprintf_s(g_sVersFromConfig, _T("%d.%d.%d.%d"), zVersion.Major, zVersion.Minor, zVersion.Build, zVersion.Revision);

	return 1;
}

/////////////////////////////////////////////////////////////////////////////
int Config::SaveConfig()
{
	CStdioFile pFile;
	if (!pFile.Open(m_sConfigPath, CFile::modeWrite | CFile::typeBinary))
		return 0;

	pFile.Seek(sizeof(zVersion), CFile::begin);
	pFile.Write(&g_zExeInfo, sizeof(g_zExeInfo));
	pFile.Close();

	return 1;
}
