#include <Windows.h>
#include <string>
#include "SoftwareColors.h"
#include "SoftwareDefinitions.h"
#include "resource.h"

void RefreshWindowList() {
	g_windows.clear();

	// ������� ������ ����
	SendMessage(g_hWndListBox, LB_RESETCONTENT, 0, 0);

	// ��������� ������ ���� ������� ����
	HWND hwnd = GetTopWindow(nullptr);
	while (hwnd != nullptr) {
		if (IsWindowVisible(hwnd)) {
			// ���������, �� �������� �� ���� ��������� ��� ����� ������������
			LONG exStyle = GetWindowLong(hwnd, GWL_EXSTYLE);
			if (!(exStyle & WS_EX_TOOLWINDOW)) {
				wchar_t title[256];
				GetWindowText(hwnd, title, sizeof(title) / sizeof(wchar_t));

				// ���������, �� �������� �� ��������� ������������ ��������� ��� �������������
				std::wstring windowTitle = title;
				if (windowTitle != L"���������" && windowTitle != L"Microsoft Text Input Application") {
					g_windows.push_back({ hwnd, title });

					// ���������� ��������� ���� � ������
					SendMessage(g_hWndListBox, LB_ADDSTRING, 0, (LPARAM)title);
				}
			}
		}
		hwnd = GetNextWindow(hwnd, GW_HWNDNEXT);
	}
}

// ������� ��������� ��� ���������� ���� �� ����������
bool CompareWindowTitles(const WindowInfo& a, const WindowInfo& b) {
	return a.title < b.title;
}

void ABSort() {
	// ��������� ������ ���� �� ���������� � ���������� �������
	std::sort(g_windows.begin(), g_windows.end(), CompareWindowTitles);

	// ������� ������ ���� �� ������
	SendMessage(g_hWndListBox, LB_RESETCONTENT, 0, 0);

	// ������� ��������������� ������ ���� �� �����
	for (const auto& window : g_windows) {
		SendMessage(g_hWndListBox, LB_ADDSTRING, 0, (LPARAM)window.title.c_str());
	}
}

void BASort() {
	// ��������� ������ ���� �� ���������� � �������� ���������� �������
	std::sort(g_windows.begin(), g_windows.end(), [](const WindowInfo& a, const WindowInfo& b) {
		return a.title > b.title;
		});

	// ������� ������ ���� �� ������
	SendMessage(g_hWndListBox, LB_RESETCONTENT, 0, 0);

	// ������� ��������������� ������ ���� �� �����
	for (const auto& window : g_windows) {
		SendMessage(g_hWndListBox, LB_ADDSTRING, 0, (LPARAM)window.title.c_str());
	}
}

void OpenSelectedWindow() {
	// �������� ������ ���������� �������� � ������
	int selectedIndex = SendMessage(g_hWndListBox, LB_GETCURSEL, 0, 0);

	if (selectedIndex != LB_ERR) { // ���������, ��� ������� ��� ������
		// �������� ��������� ���� �� �������
		wchar_t title[256];
		SendMessage(g_hWndListBox, LB_GETTEXT, selectedIndex, (LPARAM)title);

		// ���� ���� �� ���������
		for (const auto& window : g_windows) {
			if (window.title == title) {
				HWND hWnd = window.hwnd;

				// ���������, ��� ���� ���������� � ������
				if (IsWindow(hWnd) && IsWindowVisible(hWnd)) {
					// ������������� ����
					ShowWindow(hWnd, SW_RESTORE);
					SetForegroundWindow(hWnd);
				}
				break; // ���������� ����� �� ����� ����� ���������� ���������������� ����
			}
		}
	}
}


void CloseSelectedWindow() {
	// �������� ������ ���������� �������� � ������
	int selectedIndex = SendMessage(g_hWndListBox, LB_GETCURSEL, 0, 0);

	if (selectedIndex != LB_ERR) { // ���������, ��� ������� ��� ������
		// �������� ��������� ���� �� �������
		wchar_t title[256];
		SendMessage(g_hWndListBox, LB_GETTEXT, selectedIndex, (LPARAM)title);

		// ���� ���� �� ���������
		for (const auto& window : g_windows) {
			if (window.title == title) {
				HWND hWnd = window.hwnd;

				// ���������, ��� ���� ���������� � ������
				if (IsWindow(hWnd) && IsWindowVisible(hWnd)) {
					// ��������� ����
					SendMessage(hWnd, WM_CLOSE, 0, 0);
				}
				break; // ���������� ����� �� ����� ����� ���������� ���������������� ����
			}
		}
	}
}

int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrevInst, LPSTR args, int ncmdshow) {

	fontRectangle = CreateFontA(
		60, 20, 0, 0, FW_MEDIUM,
		FALSE, FALSE, FALSE, DEFAULT_CHARSET,
		OUT_OUTLINE_PRECIS, CLIP_DEFAULT_PRECIS, ANTIALIASED_QUALITY,
		FF_DECORATIVE, "SpecialFont"
	);

	fontStatic = CreateFontA(
		30, 10, 0, 0, FW_MEDIUM,
		FALSE, FALSE, FALSE, DEFAULT_CHARSET,
		OUT_OUTLINE_PRECIS, CLIP_DEFAULT_PRECIS, ANTIALIASED_QUALITY,
		FF_DECORATIVE, "SpecialStaticFont"
	);

	// �������� ������ ���� WNDCLASS � ������� ��� ����������
	WNDCLASS SoftwareMainClass = NewWindowClass((HBRUSH)COLOR_WINDOW, LoadCursor(NULL, IDC_HAND), hInst,
		LoadIcon(hInst, MAKEINTRESOURCE(IDI_ICON1)), L"MainWndClass", SoftwareMainProcedure);

	// ����������� ������ ����
	if (!RegisterClassW(&SoftwareMainClass)) { return -1; }
	MSG SoftwareMainMessage = { 0 };

	// �������� WS_OVERLAPPEDWINDOW ��������� ������ ������, ��������, ������� �� ���� + ����������� ���� + ����������� ������ � ����� + ��������� ����
	CreateWindow(L"MainWndClass", L"Window Monitor", WS_OVERLAPPEDWINDOW | WS_VISIBLE, 100, 100, 800, 800, NULL, NULL, NULL, NULL);
	while (GetMessage(&SoftwareMainMessage, NULL, NULL, NULL))
	{
		TranslateMessage(&SoftwareMainMessage);
		DispatchMessage(&SoftwareMainMessage);
	}

	return 0;

}


WNDCLASS NewWindowClass(HBRUSH BGColor, HCURSOR Cursor, HINSTANCE hInst, HICON Icon, LPCWSTR Name, WNDPROC Procedure) {

	WNDCLASS NWC = { 0 };

	NWC.hCursor = Cursor;
	NWC.hIcon = Icon;
	NWC.hInstance = hInst;
	NWC.lpszClassName = Name;
	NWC.hbrBackground = BGColor;
	NWC.lpfnWndProc = Procedure;

	return NWC;

}

LRESULT CALLBACK SoftwareMainProcedure(HWND hWnd, UINT msg, WPARAM wp, LPARAM lp) {
	int y;
	int cellHeight;
	int maxWidth;
	int cellWidth;
	int maxHeight;
	switch (msg)
	{
	case WM_COMMAND:
		switch (wp) {
		case OnRefreshField:
			g_windows.clear();
			RefreshWindowList();
			break;
		case OnReadColor:

			colorR = GetDlgItemInt(hWnd, DigIndexColorR, FALSE, false);
			colorG = GetDlgItemInt(hWnd, DigIndexColorG, FALSE, false);
			colorB = GetDlgItemInt(hWnd, DigIndexColorB, FALSE, false);


			//brushRectangle = CreateSolidBrush(RGB(colorR, colorG, colorB));

			fontColor = RGB(colorR, colorG, colorB);

			RedrawWindow(hWnd, NULL, NULL, RDW_UPDATENOW | RDW_INVALIDATE);

			break;
		case ID_OPEN_WINDOW:
			// ������� ����
			OpenSelectedWindow();
			break;
		case ID_CLOSE_WINDOW:
			// ������� ����
			CloseSelectedWindow();
			break;
		case OnExitSoftware:
			PostQuitMessage(0);
			break;	
		case OnNameABSortField:
			ABSort();
			break;
		case OnNameBASortField:
			BASort();
			break;
		default:
			break;

		}
		break;

	case WM_PAINT:

		BeginPaint(hWnd, &ps);

		//FillRect(ps.hdc, &windowRectangle, brushRectangle);
		GradientRect(ps.hdc, &windowRectangle, Color(0, 148, 153), Color(0, 184, 74));

		RefreshWindowList();

		SetBkMode(ps.hdc, TRANSPARENT);
		SetTextColor(ps.hdc, fontColor);
		SelectObject(ps.hdc, fontRectangle);
		// DT_NOCLIP - ���� ����� �� ������ � �������������, �� ������ �� ��� ������� � ����� �����
		DrawTextA(ps.hdc, "Window list", 15, &windowRectangle, DT_SINGLELINE | DT_CENTER | DT_VCENTER | DT_NOCLIP);


		
		EndPaint(hWnd, &ps);
		break;
	case WM_CREATE:
		MainWndAddMenus(hWnd);
		MainWndAddWidgets(hWnd);
		RefreshWindowList();

		SendMessageA(hStaticControl, WM_SETFONT, (WPARAM)fontStatic, TRUE);
		break;
	case WM_CTLCOLORSTATIC:

		hdcStatic = (HDC)wp;
		SetBkColor(hdcStatic, RGB(0, 200, 0));
		return (INT_PTR)CreateSolidBrush(RGB(0, 255, 0));

		break;
	case WM_CTLCOLOREDIT:

		hdcStatic = (HDC)wp;
		SetBkColor(hdcStatic, RGB(0, 200, 0));
		return (INT_PTR)CreateSolidBrush(RGB(0, 200, 0));

		break;

	case WM_CONTEXTMENU:
		if ((HWND)wp == g_hWndListBox) {
			// �������� ���������� ������� ����
			POINT cursor;
			GetCursorPos(&cursor);

			// ������� ����������� ����
			HMENU hPopupMenu = CreatePopupMenu();
			AppendMenu(hPopupMenu, MF_STRING, ID_OPEN_WINDOW, L"Open");
			AppendMenu(hPopupMenu, MF_STRING, ID_CLOSE_WINDOW, L"Close");

			// ���������� ����������� ����
			TrackPopupMenu(hPopupMenu, TPM_LEFTALIGN | TPM_TOPALIGN | TPM_LEFTBUTTON, cursor.x, cursor.y, 0, hWnd, NULL);

			// ����������� ������� ������������ ����
			DestroyMenu(hPopupMenu);
		}
		break;

	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default: return DefWindowProc(hWnd, msg, wp, lp);

	}
}

void MainWndAddMenus(HWND hWnd) {
	HMENU RootMenu = CreateMenu();

	HMENU SubMenu = CreateMenu();
	HMENU SubActionMenu = CreateMenu();


	AppendMenu(SubMenu, MF_STRING, OnRefreshField, L"Refresh");
	AppendMenu(SubMenu, MF_SEPARATOR, NULL, NULL);
	AppendMenu(SubMenu, MF_POPUP, (UINT_PTR)SubActionMenu, L"Name Sort");
	AppendMenu(SubMenu, MF_SEPARATOR, NULL, NULL);
	AppendMenu(SubMenu, MF_STRING, OnExitSoftware, L"Exit");

	AppendMenu(SubActionMenu, MF_STRING, OnNameABSortField, L"A-Z");
	AppendMenu(SubActionMenu, MF_STRING, OnNameBASortField, L"Z-A");


	AppendMenu(RootMenu, MF_POPUP, (UINT_PTR)SubMenu, L"Menu");

	SetMenu(hWnd, RootMenu);
}

void MainWndAddWidgets(HWND hWnd) {

	// WS_VISIBLE - ������� ����� �� �������, WS_CHILD - ������� �������� �������� �� �������� ����(�������� ������ ����� ����)
	hStaticControl = CreateWindowA("static", "Hello, Wind!", WS_VISIBLE | WS_CHILD | ES_CENTER, 5, 5, 80, 30, hWnd, NULL, NULL, NULL);
	windowRectangle = { 780, 50, 5, 110 };


	CreateWindowA("edit", "0", WS_VISIBLE | WS_CHILD | ES_CENTER | ES_NUMBER, 270, 10, 80, 20, hWnd, (HMENU)DigIndexColorR, NULL, NULL);
	CreateWindowA("edit", "0", WS_VISIBLE | WS_CHILD | ES_CENTER | ES_NUMBER, 385, 10, 80, 20, hWnd, (HMENU)DigIndexColorG, NULL, NULL);
	CreateWindowA("edit", "0", WS_VISIBLE | WS_CHILD | ES_CENTER | ES_NUMBER, 500, 10, 80, 20, hWnd, (HMENU)DigIndexColorB, NULL, NULL);


	CreateWindowA("button", "Set color", WS_VISIBLE | WS_CHILD | ES_CENTER, 95, 5, 170, 30, hWnd, (HMENU)OnReadColor, NULL, NULL);

	g_hWndListBox = CreateWindowW(L"LISTBOX", L"", WS_VISIBLE | WS_CHILD | WS_BORDER | LBS_NOINTEGRALHEIGHT | LBS_HASSTRINGS | LBS_NOTIFY,
		5, 115, 770, 600, hWnd, nullptr, nullptr, nullptr);


}

