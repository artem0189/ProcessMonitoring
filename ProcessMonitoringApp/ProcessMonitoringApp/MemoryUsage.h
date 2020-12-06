#include <windows.h>

class MemoryUsage
{
public:
	MemoryUsage(HANDLE hProcess);
	ULONGLONG GetUsage();
private:
	HANDLE hProcess;
};