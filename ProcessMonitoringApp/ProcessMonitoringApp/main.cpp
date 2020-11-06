#include <windows.h>
#include <tlhelp32.h>

CONST WCHAR MainClassName[] = TEXT("MainClass");
CONST WCHAR MainWindowName[] = TEXT("Procces Monitoring");

VOID PrintMessage(LPCWSTR header, LPCWSTR message);
LRESULT CALLBACK MainWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

INT WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, INT nCmdShow)
{
	HWND hMainWindow;
	MSG msg;
	WNDCLASSEX wc;

	ZeroMemory(&wc, sizeof(wc));
	wc.cbSize = sizeof(wc);
	wc.lpszClassName = MainClassName;
	wc.lpfnWndProc = MainWndProc;
	wc.hInstance = hInstance;

	if (!RegisterClassEx(&wc)) {
		PrintMessage(TEXT("Error"), TEXT("Can't register a class"));
		return NULL;
	}

	hMainWindow = CreateWindowEx(NULL, MainClassName, MainWindowName, WS_OVERLAPPED | WS_SYSMENU, 400, 100, 400, 600, NULL, NULL, hInstance, NULL);

	if (!hMainWindow) {
		PrintMessage(TEXT("Error"), TEXT("Can't create window"));
		return NULL;
	}

	ShowWindow(hMainWindow, nCmdShow);
	UpdateWindow(hMainWindow);

	while (GetMessage(&msg, NULL, 0, 0)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return 0;
}

LRESULT CALLBACK MainWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (uMsg == WM_CREATE) {
		//
	}

	switch (uMsg) {
	case WM_DESTROY:
		PostQuitMessage(NULL);
		return NULL;
	}
	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

VOID PrintMessage(LPCWSTR header, LPCWSTR message)
{
	MessageBox(NULL, message, header, MB_OK);
}