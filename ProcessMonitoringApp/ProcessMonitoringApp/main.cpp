#include <windows.h>
#include <commctrl.h>
#include <tlhelp32.h>
#include <vector>
#include <string>
#include "Diagnostic.h"

#pragma comment(lib, "ComCtl32.Lib")

#define BORDER_WIDTH 16
#define LIST_VIEW_HEIGHT 400
#define LIST_VIEW_COLUMN_MIN_WIDTH 100
#define LIST_VIEW_MIN_WIDTH 200
#define MY_COMMAND (WM_COMMAND + 4)
#define MY_NOTIFY (WM_NOTIFY + 4)
#define IDC_MENU_ITEM 1000
#define IDC_LIST_VIEW 2001
#define IDC_DELETE_PROCESS_ITEM 3000

CONST WCHAR MainClassName[] = TEXT("MainClass");
CONST WCHAR MainWindowName[] = TEXT("Procces Monitoring");

Diagnostic* diagnostic;
std::vector<Process*>* processes;

volatile BOOL isUpdate = TRUE;
volatile BOOL isDelete = FALSE;
DWORD selectedProcessId = -1;
INT selectedItem = -1;

RECT mainWindowRect, listViewRect;

VOID CheckProcesses(HWND hListView);
DWORD WINAPI Update(HWND hListView);
HWND CreateListView(HWND hWndParent);
VOID InsertColumn(HWND hListView, INT index);
VOID UpdateColumn(HWND hListView, INT index);
VOID DeleteColumn(HWND hListView, INT index);
VOID InsertItem(HWND hListView, Process* process, INT index);
VOID UpdateItem(HWND hListView, Process* process, INT index);
VOID DeleteItem(HWND hListView, INT index);
VOID CreateCustomMenu(HWND hWndParent);
LRESULT CALLBACK MainWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK ListViewProc(HWND hListView, UINT uMsg, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData);


// ------------------------------- UpdateTool ------------------------------- //

VOID CheckProcesses(HWND hListView)
{
	INT i = 0;
	while (i < diagnostic->GetProcessesCount()) {
		if ((*processes)[i]->isCreated) {
			InsertItem(hListView, (*processes)[i], i);
		}
		else if ((*processes)[i]->isDeleted) {
			DeleteItem(hListView, i);
			diagnostic->DeleteProcess(i);
			i--;
		}
		else if ((*processes)[i]->isUpdated) {
			UpdateItem(hListView, (*processes)[i], i);
		}
		i++;
	}
	if (selectedItem != -1) {
		while (i > 0) {
			if (selectedProcessId == (*processes)[i - 1]->GetProcessId()) {
				ListView_SetItemState(hListView, i - 1, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
				break;
			}
			i--;
		}
		if (i == 0) {
			ListView_SetItemState(hListView, selectedItem, 0, LVIS_SELECTED | LVIS_FOCUSED);
			selectedItem = -1;
			selectedProcessId = -1;
		}
	}
}

DWORD WINAPI Update(HWND hListView)
{
	while (TRUE) {
		while (isDelete) {}
		isUpdate = FALSE;
		processes = diagnostic->Update();
		CheckProcesses(hListView);
		isUpdate = TRUE;
		Sleep(2000);
	}
}

// ------------------------------- UpdateTool ------------------------------- //

// ---------------------------------- Menu ---------------------------------- //

VOID CreateCustomMenu(HWND hWndParent)
{
	HMENU hMenu;
	HMENU hPopupColumnMenu, hPopupActionMenu;

	hMenu = CreateMenu();
	hPopupColumnMenu = CreateMenu();
	hPopupActionMenu = CreateMenu();

	AppendMenu(hMenu, MF_STRING | MF_POPUP, (UINT)hPopupColumnMenu, L"Столбцы");
	{
		for (int i = 0; i < diagnostic->header.size(); i++) {
			AppendMenu(hPopupColumnMenu, MF_STRING | diagnostic->header[i].isVisible ? MF_CHECKED : 0, IDC_MENU_ITEM + i + 1, diagnostic->header[i].name.c_str());
		}
	}
	AppendMenu(hMenu, MF_STRING | MF_POPUP, (UINT)hPopupActionMenu, L"Действия");
	{
		AppendMenu(hPopupActionMenu, MF_STRING, IDC_DELETE_PROCESS_ITEM, L"Удалить процесс");
	}

	SetMenu(hWndParent, hMenu);
}

// ---------------------------------- Menu ---------------------------------- //

// -------------------------------- ListView -------------------------------- //

HWND CreateListView(HWND hWndParent)
{
	HWND hListView;

	hListView = CreateWindowEx(NULL, WC_LISTVIEW, TEXT(""), WS_CHILD | LVS_REPORT | LVS_SINGLESEL | LVS_SHOWSELALWAYS, 0, 0, 15, LIST_VIEW_HEIGHT, hWndParent, (HMENU)IDC_LIST_VIEW, GetModuleHandle(NULL), NULL);
	SendMessage(hListView, LVM_SETEXTENDEDLISTVIEWSTYLE, 0, LVS_EX_FULLROWSELECT | LVS_EX_DOUBLEBUFFER | LVS_EX_ONECLICKACTIVATE);
	SetWindowSubclass(hListView, ListViewProc, 0, 0);
	GetWindowRect(hListView, &listViewRect);
	MapWindowPoints(HWND_DESKTOP, hWndParent, (LPPOINT)&listViewRect, 2);

	for (int i = 0; i < diagnostic->header.size(); i++) {
		if (diagnostic->header[i].isVisible) {
			InsertColumn(hListView, diagnostic->GetActiveHeaderIndex(i));
		}
	}

	ShowWindow(hListView, SW_SHOWDEFAULT);
	return hListView;
}

VOID InsertColumn(HWND hListView, INT index)
{
	LVCOLUMN lvc;

	lvc.mask = LVCF_WIDTH | LVCF_TEXT;
	lvc.cx = LIST_VIEW_COLUMN_MIN_WIDTH;
	lvc.pszText = const_cast<LPWSTR>(diagnostic->header[diagnostic->GetHeaderIndex(index)].name.c_str());

	ListView_InsertColumn(hListView, index, &lvc);
	UpdateColumn(hListView, index);
}

VOID UpdateColumn(HWND hListView, INT index)
{
	std::wstring temp;
	INT headerIndex = diagnostic->GetHeaderIndex(index);
	if (processes != NULL) {
		for (int i = 0; i < diagnostic->GetProcessesCount(); i++) {
			ListView_SetItemText(hListView, i, index, const_cast<LPWSTR>((temp = diagnostic->GetValue((*processes)[i], headerIndex)).c_str()));
		}
	}
}

VOID DeleteColumn(HWND hListView, INT index)
{
	ListView_DeleteColumn(hListView, index);
}

VOID InsertItem(HWND hListView, Process* process, INT index)
{
	LVITEM lvI;
	std::wstring temp;

	lvI.mask = LVIF_TEXT;
	lvI.pszText = LPSTR_TEXTCALLBACK;
	lvI.iItem = index;
	lvI.iSubItem = 0;

	ListView_InsertItem(hListView, &lvI);
	UpdateItem(hListView, process, index);
}

VOID UpdateItem(HWND hListView, Process* process, INT index)
{
	std::wstring temp;
	WCHAR buffer[256];

	for (int i = 0; i < diagnostic->header.size(); i++) {
		if (diagnostic->header[i].isVisible) {
			temp = diagnostic->GetValue(process, i);
			ListView_GetItemText(hListView, index, diagnostic->GetActiveHeaderIndex(i), buffer, 256);
			if ((std::wstring)buffer != temp) {
				ListView_SetItemText(hListView, index, diagnostic->GetActiveHeaderIndex(i), const_cast<LPWSTR>(temp.c_str()));
			}
		}
	}
}

VOID DeleteItem(HWND hListView, INT index)
{
	ListView_DeleteItem(hListView, index);
}

LRESULT CALLBACK ListViewProc(HWND hListView, UINT uMsg, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData)
{
	LPNMHEADER pNMHeader;
	LVHITTESTINFO lvhitinfo;

	switch (uMsg) {
	case LVM_SETITEMSTATE:
		selectedItem = wParam;
		selectedProcessId = (*processes)[selectedItem]->GetProcessId();
		break;
	case LVM_SETCOLUMNWIDTH:
		listViewRect.right += lParam - ListView_GetColumnWidth(hListView, wParam);
		MoveWindow(hListView, listViewRect.left, listViewRect.top, listViewRect.right - listViewRect.left, listViewRect.bottom - listViewRect.top, TRUE);
		break;
	case LVM_INSERTCOLUMN:
		listViewRect.right += LIST_VIEW_COLUMN_MIN_WIDTH;
		MoveWindow(hListView, listViewRect.left, listViewRect.top, listViewRect.right - listViewRect.left, listViewRect.bottom - listViewRect.top, TRUE);
		break;
	case LVM_DELETECOLUMN:
		listViewRect.right -= ListView_GetColumnWidth(hListView, wParam);
		if (Header_GetItemCount(ListView_GetHeader(hListView)) == 2) {
			if (ListView_GetColumnWidth(hListView, 0) < LIST_VIEW_MIN_WIDTH) {
				ListView_SetColumnWidth(hListView, 0, LIST_VIEW_MIN_WIDTH);
			}
		}
		MoveWindow(hListView, listViewRect.left, listViewRect.top, listViewRect.right - listViewRect.left, listViewRect.bottom - listViewRect.top, TRUE);
		break;
	case WM_NOTIFY:
		switch (((LPNMHDR)lParam)->code) {
		case HDN_TRACK:
			pNMHeader = (LPNMHEADER)lParam;
			if (pNMHeader->pitem->cxy >= LIST_VIEW_COLUMN_MIN_WIDTH) {
				ListView_SetColumnWidth(hListView, pNMHeader->iItem, pNMHeader->pitem->cxy);
				return FALSE;
			}
			return TRUE;
		}
		break;
	case MY_NOTIFY:
		switch (((LPNMHDR)lParam)->code) {
		case NM_CLICK:
			lvhitinfo.pt = ((LPNMITEMACTIVATE)lParam)->ptAction;
			ListView_SubItemHitTest(hListView, &lvhitinfo);
			if (lvhitinfo.iItem != -1) {
				ListView_SetItemState(hListView, lvhitinfo.iItem, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
			}
			break;
		case LVN_COLUMNCLICK:
			processes = diagnostic->SortByHeader(diagnostic->GetHeaderIndex(((LPNMLISTVIEW)lParam)->iSubItem));
			CheckProcesses(hListView);
			break;
		}
		break;
	case WM_SIZE:
		MoveWindow(GetParent(hListView), mainWindowRect.left, mainWindowRect.top, listViewRect.right + BORDER_WIDTH, listViewRect.bottom + BORDER_WIDTH, TRUE);
		break;
	}
	return DefSubclassProc(hListView, uMsg, wParam, lParam);
}

// -------------------------------- ListView -------------------------------- //

// ------------------------------- MainWindow ------------------------------- //

LRESULT CALLBACK MainWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	HWND hListView;
	LPWINDOWPOS pWindowpos;

	if (uMsg == WM_CREATE) {
		diagnostic = new Diagnostic();

		GetWindowRect(hWnd, &mainWindowRect);
		hListView = CreateListView(hWnd);

		processes = diagnostic->Update();
		for (int i = 0; i < diagnostic->GetProcessesCount(); i++) {
			InsertItem(hListView, (*processes)[i], i);
		}

		CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)Update, hListView, 0, NULL);
	}

	switch (uMsg) {
	case WM_COMMAND:
		if (LOWORD(wParam) >= IDC_MENU_ITEM + 1 && LOWORD(wParam) <= IDC_MENU_ITEM + diagnostic->header.size()) {
			boolean isVisible = diagnostic->header[LOWORD(wParam) - IDC_MENU_ITEM - 1].isVisible;
			diagnostic->header[LOWORD(wParam) - IDC_MENU_ITEM - 1].isVisible = !isVisible;
			if (isVisible) {
				CheckMenuItem(GetMenu(hWnd), LOWORD(wParam), GetMenuState(GetMenu(hWnd), LOWORD(wParam), MF_BYCOMMAND) & ~MF_CHECKED);
				DeleteColumn(GetDlgItem(hWnd, IDC_LIST_VIEW), diagnostic->GetActiveHeaderIndex(LOWORD(wParam) - IDC_MENU_ITEM - 1));
			}
			else {
				CheckMenuItem(GetMenu(hWnd), LOWORD(wParam), GetMenuState(GetMenu(hWnd), LOWORD(wParam), MF_BYCOMMAND) | MF_CHECKED);
				InsertColumn(GetDlgItem(hWnd, IDC_LIST_VIEW), diagnostic->GetActiveHeaderIndex(LOWORD(wParam) - IDC_MENU_ITEM - 1));
			}
			break;
		}
		switch (LOWORD(wParam)) {
		case IDC_DELETE_PROCESS_ITEM:
			if (selectedItem != -1) {
				while (!isUpdate) {}
				isDelete = TRUE;
				(*processes)[selectedItem]->StopProcess();
				diagnostic->DeleteProcess(selectedItem);
				DeleteItem(GetDlgItem(hWnd, IDC_LIST_VIEW), selectedItem);
				isDelete = FALSE;
			}
			break;
		}
		break;
	case WM_NOTIFY:
		SendMessage(((NMHDR*)lParam)->hwndFrom, MY_NOTIFY, wParam, lParam);
		break;
	case WM_WINDOWPOSCHANGED:
		pWindowpos = (LPWINDOWPOS)lParam;
		mainWindowRect = { pWindowpos->x, pWindowpos->y, pWindowpos->x + pWindowpos->cx, pWindowpos->y + pWindowpos->cy };
		break;
	case WM_DESTROY:
		delete diagnostic;
		PostQuitMessage(NULL);
		return NULL;
	}
	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

INT WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, INT nCmdShow)
{
	HWND hMainWindow;
	MSG msg;
	WNDCLASSEX wc;

	ZeroMemory(&wc, sizeof(wc));
	wc.cbSize = sizeof(wc);
	wc.hbrBackground = (HBRUSH)COLOR_WINDOW;
	wc.lpszClassName = MainClassName;
	wc.lpfnWndProc = MainWndProc;
	wc.hInstance = hInstance;

	RegisterClassEx(&wc);
	hMainWindow = CreateWindowEx(NULL, MainClassName, MainWindowName, (WS_OVERLAPPEDWINDOW | WS_VISIBLE) & ~(WS_SIZEBOX | WS_MAXIMIZEBOX), 400, 100, 300, 300, NULL, NULL, hInstance, NULL);

	CreateCustomMenu(hMainWindow);

	ShowWindow(hMainWindow, nCmdShow);
	UpdateWindow(hMainWindow);

	while (GetMessage(&msg, NULL, 0, 0)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return 0;
}

// ------------------------------- MainWindow ------------------------------- //