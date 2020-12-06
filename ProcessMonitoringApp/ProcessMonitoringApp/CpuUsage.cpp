#include "CpuUsage.h";

CpuUsage::CpuUsage(HANDLE hProcess)
{
	this->hProcess = hProcess;
	m_dwLastRun = 0;

	ZeroMemory(&m_ftPrevSysKernel, sizeof(FILETIME));
	ZeroMemory(&m_ftPrevSysUser, sizeof(FILETIME));
	ZeroMemory(&m_ftPrevProcKernel, sizeof(FILETIME));
	ZeroMemory(&m_ftPrevProcUser, sizeof(FILETIME));
}

ULONGLONG CpuUsage::SubstractTimes(CONST FILETIME& ftA, CONST FILETIME& ftB)
{
	ULARGE_INTEGER a = { ftA.dwLowDateTime, ftA.dwHighDateTime };
	ULARGE_INTEGER b = { ftB.dwLowDateTime, ftB.dwHighDateTime };

	return a.QuadPart - b.QuadPart;
}

DOUBLE CpuUsage::GetUsage()
{
	FILETIME ftSysIdle, ftSysKernel, ftSysUser;
	FILETIME ftProcCreation, ftProcExit, ftProcKernel, ftProcUser;

	GetSystemTimes(&ftSysIdle, &ftSysKernel, &ftSysUser);
	GetProcessTimes(hProcess, &ftProcCreation, &ftProcExit, &ftProcKernel, &ftProcUser);

	if (m_dwLastRun != 0) {
		ULONGLONG ftSysKernelDiff = SubstractTimes(ftSysKernel, m_ftPrevSysKernel);
		ULONGLONG ftSysUserDiff = SubstractTimes(ftSysUser, m_ftPrevSysUser);

		ULONGLONG ftProcKernelDiff = SubstractTimes(ftProcKernel, m_ftPrevProcKernel);
		ULONGLONG ftProcUserDiff = SubstractTimes(ftProcUser, m_ftPrevProcUser);

		ULONGLONG nTotalSys = ftSysKernelDiff + ftSysUserDiff;
		ULONGLONG nTotalProc = ftProcKernelDiff + ftProcUserDiff;

		if (nTotalSys > 0) {
			m_nCpuUsage = (100.0 * nTotalProc) / nTotalSys;
		}
	}

	m_ftPrevSysKernel = ftSysKernel;
	m_ftPrevSysUser = ftSysUser;
	m_ftPrevProcKernel = ftProcKernel;
	m_ftPrevProcUser = ftProcUser;

	m_dwLastRun = GetTickCount64();

	return m_nCpuUsage;
}