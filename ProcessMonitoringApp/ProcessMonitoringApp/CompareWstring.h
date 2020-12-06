#include <windows.h>
#include <algorithm>
#include <string>

BOOL TryConvertToDouble(CONST std::wstring& str, DOUBLE* result);

BOOL CompareWstring(CONST std::wstring& first, CONST std::wstring& second, INT sign);