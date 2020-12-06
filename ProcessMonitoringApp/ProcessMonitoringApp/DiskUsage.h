#include <windows.h>

class DiskUsage
{
public:
	DiskUsage(HANDLE hProcess);
	ULONGLONG GetUsage();
private:
	HANDLE hProcess;
};