#include "DiskUsage.h"

DiskUsage::DiskUsage(HANDLE hProcess)
{
	this->hProcess = hProcess;
}

ULONGLONG DiskUsage::GetUsage()
{
	_IO_COUNTERS processDisk;

	if (GetProcessIoCounters(hProcess, &processDisk)) {
		return processDisk.ReadTransferCount + processDisk.WriteTransferCount;
	}
	return 0;
}