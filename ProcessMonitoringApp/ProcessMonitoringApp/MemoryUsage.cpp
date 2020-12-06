#include "MemoryUsage.h"
#include <psapi.h>

MemoryUsage::MemoryUsage(HANDLE hProcess)
{
	this->hProcess = hProcess;
}

ULONGLONG MemoryUsage::GetUsage()
{
	_PROCESS_MEMORY_COUNTERS_EX processMemory;

	if (GetProcessMemoryInfo(hProcess, (PPROCESS_MEMORY_COUNTERS)&processMemory, sizeof(processMemory))) {
		return processMemory.PrivateUsage / 1024;
	}
	return 0;
}