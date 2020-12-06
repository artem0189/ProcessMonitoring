#include "CompareWstring.h"
#include <sstream>
#include <cwctype>

BOOL TryConvertToDouble(CONST std::wstring& str, DOUBLE* result)
{
	std::wstringstream wss(str);
	wss >> *result;

	if (wss.eof() && !wss.fail()) {
		return TRUE;
	}
	return FALSE;
}

BOOL CompareWstring(CONST std::wstring& first, CONST std::wstring& second, INT sign)
{
	DOUBLE firstDigit, secondDigit;
	std::wstring firstWstring, secondWstring;

	firstWstring = first;
	secondWstring = second;
	if (TryConvertToDouble(firstWstring, &firstDigit) && TryConvertToDouble(secondWstring, &secondDigit)) {
		return sign * firstDigit > sign * secondDigit;
	}
	else
	{
		std::transform(firstWstring.begin(), firstWstring.end(), firstWstring.begin(), std::towlower);
		std::transform(secondWstring.begin(), secondWstring.end(), secondWstring.begin(), std::towlower);
		return sign == 1 ? firstWstring > secondWstring : firstWstring < secondWstring;
	}
}