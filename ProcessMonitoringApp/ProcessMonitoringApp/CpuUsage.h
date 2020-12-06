#pragma once

#include <windows.h>

class CpuUsage
{
public:
	CpuUsage(HANDLE hProcess);
	DOUBLE GetUsage();
private:
	HANDLE hProcess;
	FILETIME m_ftPrevSysKernel, m_ftPrevSysUser;
	FILETIME m_ftPrevProcKernel, m_ftPrevProcUser;
	DOUBLE m_nCpuUsage;
	ULONGLONG m_dwLastRun;

	ULONGLONG SubstractTimes(CONST FILETIME& ftA, const FILETIME& ftB);
};