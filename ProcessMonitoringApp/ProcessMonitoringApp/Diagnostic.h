#pragma once

#include <windows.h>
#include <vector>
#include "Process.h"

enum Sorts
{
	SORT_NONE,
	SORT_ASC,
	SORT_DESC
};

struct Header
{
	std::wstring name;
	BOOL isVisible;
	Sorts sortParam = SORT_NONE;
};

class Diagnostic
{
public:
	Diagnostic();
	~Diagnostic();

	std::vector<Header> header;

	std::vector<Process*>* Update();
	std::vector<Process*>* SortByHeader(INT headerNumber);
	DWORD GetProcessesCount();
	VOID DeleteProcess(INT index);
	INT GetHeaderIndex(INT activeHeaderIndex);
	INT GetActiveHeaderIndex(INT headerIndex);
	std::wstring GetValue(Process* process, INT headerIndex);
private:
	CRITICAL_SECTION crSection;
	DWORD processesCount;
	std::vector<Process*> processes;

	VOID UnchangeProcesses();
	VOID CheckNewProcess();
	VOID Sort(INT headerIndex, INT sign);
	INT GetSignByHeader(INT headerIndex);
};