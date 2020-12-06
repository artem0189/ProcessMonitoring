#pragma once

#include <windows.h>
#include <string>
#include <tlhelp32.h>
#include "CpuUsage.h"
#include "MemoryUsage.h"
#include "DiskUsage.h"

class Process
{
public:
	Process(DWORD processId, INT position);
	~Process();
	BOOL isUpdated = FALSE;
	BOOL isDeleted = FALSE;
	BOOL isCreated = TRUE;
	INT position;

	VOID Update();
	VOID StopProcess();

	DWORD GetProcessId();
	std::wstring GetName();
	DOUBLE GetCpuUsage();
	ULONGLONG GetMemoryUsage();
	ULONGLONG GetDiskUsage();
private:
	HANDLE hProcess;
	CpuUsage* cpuUsage;
	MemoryUsage* memoryUsage;
	DiskUsage* diskUsage;
	
	DWORD processId;
	std::wstring name;
	DOUBLE cpuUsageValue;
	ULONGLONG memoryUsageValue;
	ULONGLONG diskUsageValue;
};