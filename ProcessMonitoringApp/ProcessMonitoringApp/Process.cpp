#pragma once

#include "Process.h"
#include <psapi.h>

std::wstring GetFileName(HANDLE hProcess);

Process::Process(DWORD processId, INT position)
{
	this->processId = processId;
	this->position = position;
	cpuUsageValue = 0;

	hProcess = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION | PROCESS_TERMINATE, FALSE, processId);
	if (hProcess != NULL) {
		name = GetFileName(hProcess);
		cpuUsage = new CpuUsage(hProcess);
		memoryUsage = new MemoryUsage(hProcess);
		diskUsage = new DiskUsage(hProcess);
	}
	else {
		isDeleted = TRUE;
	}
}

Process::~Process()
{
	CloseHandle(hProcess);
}

DWORD Process::GetProcessId()
{
	return processId;
}

std::wstring Process::GetName()
{
	return name;
}

DOUBLE Process::GetCpuUsage()
{
	return cpuUsageValue;
}

ULONGLONG Process::GetMemoryUsage()
{
	return memoryUsageValue;
}

ULONGLONG Process::GetDiskUsage()
{
	return diskUsageValue;
}

VOID Process::Update()
{
	DWORD dwExitCode;
	DOUBLE cpuUsageNew;
	ULONGLONG memoryUsageNew, diskUsageNew;

	GetExitCodeProcess(hProcess, &dwExitCode);
	if (dwExitCode != STILL_ACTIVE) {
		isDeleted = TRUE;
		return;
	}

	cpuUsageNew = cpuUsage->GetUsage();
	if (std::abs(cpuUsageNew - cpuUsageValue) > 0.001) {
		cpuUsageValue = cpuUsageNew;
		isUpdated = TRUE;
	}

	memoryUsageNew = memoryUsage->GetUsage();
	if (memoryUsageNew != memoryUsageValue) {
		memoryUsageValue = memoryUsageNew;
		isUpdated = TRUE;
	}

	diskUsageNew = diskUsage->GetUsage();
	if (diskUsageNew != diskUsageValue) {
		diskUsageValue = diskUsageNew;
		isUpdated = TRUE;
	}

	isCreated = FALSE;
}

VOID Process::StopProcess()
{
	TerminateProcess(hProcess, 0);
}

std::wstring GetFileName(HANDLE hProcess)
{
	WCHAR buffer[MAX_PATH];
	std::wstring filePath = L"", fileName = L"";

	if (GetModuleFileNameEx(hProcess, 0, buffer, MAX_PATH)) {
		filePath = buffer;
		size_t i = filePath.rfind(L'\\', filePath.length());
		if (i != std::wstring::npos) {
			fileName = filePath.substr(i + 1, filePath.length() - i);
		}
	}
	return fileName;
}