#include "Diagnostic.h"
#include <tlhelp32.h>
#include <psapi.h>
#include "CompareWstring.h"

Diagnostic::Diagnostic()
{
	Process* process;
	DWORD aProcesses[1024], cbNeeded;

	EnumProcesses(aProcesses, sizeof(aProcesses), &cbNeeded);
	for (int i = 0; i < cbNeeded / sizeof(DWORD); i++) {
		process = new Process(aProcesses[i], processesCount);
		if (!process->isDeleted) {
			processes.push_back(process);
			processesCount++;
		}
		else {
			delete process;
		}
	}

	InitializeCriticalSection(&crSection);
	header = { { L"Имя", TRUE, SORT_ASC }, { L"ИД Процесса", TRUE, SORT_NONE }, { L"ЦП %", TRUE, SORT_NONE }, { L"Память КБ", TRUE, SORT_NONE }, { L"Диск Б", TRUE, SORT_NONE } };
}

Diagnostic::~Diagnostic()
{
	DeleteCriticalSection(&crSection);
	for (int i = 0; i < processesCount; i++) {
		delete processes[i];
	}
}

DWORD Diagnostic::GetProcessesCount()
{
	return processesCount;
}

VOID Diagnostic::UnchangeProcesses()
{
	for (int i = 0; i < processes.size(); i++) {
		processes[i]->isUpdated = FALSE;
		processes[i]->position = i;
	}
}

std::vector<Process*>* Diagnostic::Update()
{
	EnterCriticalSection(&crSection);

	UnchangeProcesses();
	for (int i = 0; i < processesCount; i++) {
		processes[i]->Update();
	}

	CheckNewProcess();

	for (int i = 0; i < header.size(); i++) {
		if (header[i].sortParam != SORT_NONE) {
			Sort(i, GetSignByHeader(i));
			break;
		}
	}

	LeaveCriticalSection(&crSection);

	return &processes;
}

VOID Diagnostic::CheckNewProcess()
{
	INT position;
	Process* process;
	DWORD aProcesses[1024], cbNeeded;

	EnumProcesses(aProcesses, sizeof(aProcesses), &cbNeeded);
	for (int i = 0; i < cbNeeded / sizeof(DWORD); i++) {
		position = 0;
		while (position < processesCount) {
			if (processes[position]->GetProcessId() == aProcesses[i]) {
				break;
			}
			position++;
		}
		if (position >= processesCount) {
			process = new Process(aProcesses[i], processesCount);
			if (!process->isDeleted) {
				processes.push_back(process);
				processesCount++;
			}
			else {
				delete process;
			}
		}
	}
}

VOID Diagnostic::DeleteProcess(INT index)
{
	if (index < processesCount) {
		delete processes[index];
		processes.erase(processes.begin() + index);
		processesCount--;
	}
}

std::wstring Diagnostic::GetValue(Process* process, INT headerIndex)
{
	std::wstring temp;
	std::wstring result = L"";

	switch (headerIndex) {
	case 0:
		result = process->GetName();
		break;
	case 1:
		result = std::to_wstring(process->GetProcessId());
		break;
	case 2:
		temp = std::to_wstring(process->GetCpuUsage());
		result = temp.substr(0, temp.find(L".") + 3 + 1);
		break;
	case 3:
		result = std::to_wstring(process->GetMemoryUsage());
		break;
	case 4:
		result = std::to_wstring(process->GetDiskUsage());
		break;
	}

	return result;
}

INT Diagnostic::GetHeaderIndex(INT activeHeaderIndex)
{
	for (int i = 0; i < header.size(); i++) {
		if (header[i].isVisible) {
			activeHeaderIndex--;
			if (activeHeaderIndex == -1) {
				return i;
			}
		}
	}
	return 0;
}

INT Diagnostic::GetActiveHeaderIndex(INT headerIndex)
{
	INT activeHeaderIndex = 0;
	for (int i = 0; i < headerIndex; i++) {
		if (header[i].isVisible) {
			activeHeaderIndex++;
		}
	}

	return activeHeaderIndex;
}

std::vector<Process*>* Diagnostic::SortByHeader(INT headerIndex)
{
	Sorts previousValue;

	EnterCriticalSection(&crSection);

	previousValue = header[headerIndex].sortParam;
	for (int i = 0; i < header.size(); i++) {
		header[i].sortParam = SORT_NONE;
	}

	switch (previousValue) {
	case SORT_NONE:
		header[headerIndex].sortParam = SORT_ASC;
		break;
	case SORT_ASC:
		header[headerIndex].sortParam = SORT_DESC;
		break;
	case SORT_DESC:
		header[headerIndex].sortParam = SORT_ASC;
		break;
	}

	Sort(headerIndex, GetSignByHeader(headerIndex));

	LeaveCriticalSection(&crSection);

	return &processes;
}

VOID Diagnostic::Sort(INT headerIndex, INT sign)
{
	for (int i = 0; i < processesCount; i++) {
		for (int j = i + 1; j < processesCount; j++) {
			if (CompareWstring(GetValue(processes[i], headerIndex), GetValue(processes[j], headerIndex), sign)) {
				std::swap(processes[i], processes[j]);
			}
		}
	}

	for (int i = 0; i < processesCount; i++) {
		if (processes[i]->position != i) {
			processes[i]->isUpdated = TRUE;
		}
	}
}

INT Diagnostic::GetSignByHeader(INT headerIndex)
{
	INT sign;

	switch (header[headerIndex].sortParam) {
	case SORT_ASC:
		sign = 1;
		break;
	case SORT_DESC:
		sign = -1;
		break;
	}

	return sign;
}