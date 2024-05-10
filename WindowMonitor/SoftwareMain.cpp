#include <Windows.h>
#include <string>
#include "SoftwareColors.h"
#include "SoftwareDefinitions.h"
#include "resource.h"
#include <gdiplus.h>
#pragma comment (lib,"Gdiplus.lib")


void PreviewSelectedWindow() {
	int selectedIndex = SendMessage(g_hWndListBox, LB_GETCURSEL, 0, 0);

	if (selectedIndex != LB_ERR) {
		wchar_t title[256];
		SendMessage(g_hWndListBox, LB_GETTEXT, selectedIndex, (LPARAM)title);

		for (const auto& window : g_windows) {
			if (window.title == title) {
				HWND hWnd = window.hwnd;

				if (IsWindow(hWnd) && IsWindowVisible(hWnd)) {
					RECT rect;
					GetClientRect(hWnd, &rect);

					// Create a bitmap to capture the screenshot
					HDC hdcScreen = GetDC(NULL);
					HDC hdcWindow = GetDC(hWnd);

					// Capture the screenshot
					HDC hdcMemDC = CreateCompatibleDC(hdcScreen);
					HBITMAP hBitmap = CreateCompatibleBitmap(hdcScreen, rect.right - rect.left, rect.bottom - rect.top);
					HGDIOBJ oldBitmap = SelectObject(hdcMemDC, hBitmap);

					PrintWindow(hWnd, hdcMemDC, PW_CLIENTONLY);
					SelectObject(hdcMemDC, oldBitmap);
					DeleteDC(hdcMemDC);

					// Create a new window to display the screenshot
					HWND hWndPreview = CreateWindowEx(
						WS_EX_CLIENTEDGE,
						L"STATIC",
						L"Preview",
						WS_VISIBLE | WS_OVERLAPPEDWINDOW | SS_BITMAP,
						CW_USEDEFAULT, CW_USEDEFAULT, rect.right - rect.left, rect.bottom - rect.top,
						NULL, NULL, GetModuleHandle(NULL), NULL);

					// Set the captured bitmap as the background of the new window
					SendMessage(hWndPreview, STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)hBitmap);

					// Load custom icon
					HICON hIcon = LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_ICON2));

					SendMessage(hWndPreview, WM_SETICON, ICON_SMALL, (LPARAM)hIcon);
					SendMessage(hWndPreview, WM_SETICON, ICON_BIG, (LPARAM)hIcon);

					SetWindowLongPtr(hWndPreview, GWLP_WNDPROC, (LONG_PTR)PreviewWindowProc);

					// Cleanup
					DeleteObject(hBitmap);
					ReleaseDC(hWnd, hdcWindow);
					ReleaseDC(NULL, hdcScreen);
				}
				break;
			}
		}
	}
	else {
		std::cout << "No item selected." << std::endl;
	}
}


LRESULT CALLBACK PreviewWindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
	switch (message) {
	case WM_CTLCOLORSTATIC: {
		HBRUSH hBrush = CreateSolidBrush(RGB(0, 0, 0)); 
		HBRUSH hOldBrush = (HBRUSH)SelectObject((HDC)wParam, hBrush);
		SetTextColor((HDC)wParam, RGB(255, 255, 255)); 
		SetBkColor((HDC)wParam, RGB(0, 0, 0));
		return (LRESULT)hBrush;
	}
	case WM_DESTROY:
		DestroyWindow(hWnd);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}



std::chrono::time_point<std::chrono::system_clock> GetWindowOpenTime(HWND hwnd) {
	FILETIME creationTime, exitTime, kernelTime, userTime;

	// Получаем идентификатор потока, связанного с окном
	DWORD dwProcessId;
	DWORD dwThreadId = GetWindowThreadProcessId(hwnd, &dwProcessId);

	// Получаем временные данные процесса, связанного с окном
	if (GetProcessTimes(OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, dwProcessId), &creationTime, &exitTime, &kernelTime, &userTime)) {
		// Преобразуем время создания процесса в системное время
		ULARGE_INTEGER uli;
		uli.LowPart = creationTime.dwLowDateTime;
		uli.HighPart = creationTime.dwHighDateTime;
		auto systemTime = std::chrono::system_clock::from_time_t(uli.QuadPart / 10000000 - 11644473600);

		return systemTime;
	}
	else {
		// Если не удалось получить временные данные, возвращаем текущее системное время
		return std::chrono::system_clock::now();
	}
}

void RefreshWindowList() {
	g_windows.clear();

	// Очистка списка окон
	SendMessage(g_hWndListBox, LB_RESETCONTENT, 0, 0);

	// Получение списка всех верхних окон
	HWND hwnd = GetTopWindow(nullptr);
	while (hwnd != nullptr) {
		if (IsWindowVisible(hwnd)) {
			// Проверяем, не является ли окно системным или окном инструментов
			LONG exStyle = GetWindowLong(hwnd, GWL_EXSTYLE);
			if (!(exStyle & WS_EX_TOOLWINDOW)) {
				wchar_t title[256];
				std::chrono::time_point<std::chrono::system_clock> openTime;
				GetWindowText(hwnd, title, sizeof(title) / sizeof(wchar_t));
				openTime = GetWindowOpenTime(hwnd);

				// Проверяем, не является ли заголовок определенным системным или нежелательным
				std::wstring windowTitle = title;
				if (windowTitle != L"Параметры" && windowTitle != L"Microsoft Text Input Application") {
					g_windows.push_back({ hwnd, title, openTime });

					// Добавление заголовка окна в список
					SendMessage(g_hWndListBox, LB_ADDSTRING, 0, (LPARAM)title);
				}
			}
		}
		hwnd = GetNextWindow(hwnd, GW_HWNDNEXT);
	}
}

// Функция сравнения для сортировки окон по заголовкам
bool CompareWindowTitles(const WindowInfo& a, const WindowInfo& b) {
	return a.title < b.title;
}

void ABSort() {
	// Сортируем список окон по заголовкам в алфавитном порядке
	std::sort(g_windows.begin(), g_windows.end(), CompareWindowTitles);

	// Очищаем список окон на экране
	SendMessage(g_hWndListBox, LB_RESETCONTENT, 0, 0);

	// Выводим отсортированный список окон на экран
	for (const auto& window : g_windows) {
		SendMessage(g_hWndListBox, LB_ADDSTRING, 0, (LPARAM)window.title.c_str());
	}
}

void BASort() {
	// Сортируем список окон по заголовкам в обратном алфавитном порядке
	std::sort(g_windows.begin(), g_windows.end(), [](const WindowInfo& a, const WindowInfo& b) {
		return a.title > b.title;
		});

	// Очищаем список окон на экране
	SendMessage(g_hWndListBox, LB_RESETCONTENT, 0, 0);

	// Выводим отсортированный список окон на экран
	for (const auto& window : g_windows) {
		SendMessage(g_hWndListBox, LB_ADDSTRING, 0, (LPARAM)window.title.c_str());
	}
}

bool CompareWindowOpenTime(const WindowInfo& a, const WindowInfo& b) {
	return a.openTime.time_since_epoch() < b.openTime.time_since_epoch();
}


void EarlierSort() {
	std::sort(g_windows.begin(), g_windows.end(), CompareWindowOpenTime);

	SendMessage(g_hWndListBox, LB_RESETCONTENT, 0, 0);

	// Выводим отсортированный список окон на экран
	for (const auto& window : g_windows) {
		SendMessage(g_hWndListBox, LB_ADDSTRING, 0, (LPARAM)window.title.c_str());
	}
}

bool CompareWindowOpenTimeReverse(const WindowInfo& a, const WindowInfo& b) {
	return a.openTime.time_since_epoch() > b.openTime.time_since_epoch();
}

void LaterSort() {
	std::sort(g_windows.begin(), g_windows.end(), CompareWindowOpenTimeReverse);

	SendMessage(g_hWndListBox, LB_RESETCONTENT, 0, 0);

	// Выводим отсортированный список окон на экран
	for (const auto& window : g_windows) {
		SendMessage(g_hWndListBox, LB_ADDSTRING, 0, (LPARAM)window.title.c_str());
	}
}

void OpenSelectedWindow() {
	// Получаем индекс выбранного элемента в списке
	int selectedIndex = SendMessage(g_hWndListBox, LB_GETCURSEL, 0, 0);

	if (selectedIndex != LB_ERR) { // Проверяем, что элемент был выбран
		// Получаем заголовок окна по индексу
		wchar_t title[256];
		SendMessage(g_hWndListBox, LB_GETTEXT, selectedIndex, (LPARAM)title);

		// Ищем окно по заголовку
		for (const auto& window : g_windows) {
			if (window.title == title) {
				HWND hWnd = window.hwnd;

				// Проверяем, что окно существует и видимо
				if (IsWindow(hWnd) && IsWindowVisible(hWnd)) {
					// Разворачиваем окно
					ShowWindow(hWnd, SW_RESTORE);
					SetForegroundWindow(hWnd);
				}
				break; // Необходимо выйти из цикла после нахождения соответствующего окна
			}
		}
	}
}


void CloseSelectedWindow() {
	// Получаем индекс выбранного элемента в списке
	int selectedIndex = SendMessage(g_hWndListBox, LB_GETCURSEL, 0, 0);

	if (selectedIndex != LB_ERR) { // Проверяем, что элемент был выбран
		// Получаем заголовок окна по индексу
		wchar_t title[256];
		SendMessage(g_hWndListBox, LB_GETTEXT, selectedIndex, (LPARAM)title);

		// Ищем окно по заголовку
		for (const auto& window : g_windows) {
			if (window.title == title) {
				HWND hWnd = window.hwnd;

				// Проверяем, что окно существует и видимо
				if (IsWindow(hWnd) && IsWindowVisible(hWnd)) {
					// Закрываем окно
					SendMessage(hWnd, WM_CLOSE, 0, 0);
				}
				break; // Необходимо выйти из цикла после нахождения соответствующего окна
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

	// Создание класса окна WNDCLASS и задание его параметров
	WNDCLASS SoftwareMainClass = NewWindowClass((HBRUSH)COLOR_WINDOW, LoadCursor(NULL, IDC_HAND), hInst,
		LoadIcon(hInst, MAKEINTRESOURCE(IDI_ICON1)), L"MainWndClass", SoftwareMainProcedure);

	// Регистрация класса окна
	if (!RegisterClassW(&SoftwareMainClass)) { return -1; }
	MSG SoftwareMainMessage = { 0 };

	// параметр WS_OVERLAPPEDWINDOW добавляет кнопки размер, свернуть, закрыть на окно + перемещение окна + стандартные бордюр и рамка + системное меню
	CreateWindow(L"MainWndClass", L"Window Monitor", WS_OVERLAPPEDWINDOW | WS_VISIBLE, 100, 100, 800, 880, NULL, NULL, NULL, NULL);
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
			// Открыть окно
			OpenSelectedWindow();
			break;
		case ID_CLOSE_WINDOW:
			// Закрыть окно
			CloseSelectedWindow();
			RefreshWindowList();
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
		case OnEarlierSortField:
			EarlierSort();
			break;
		case OnLaterSortField:
			LaterSort();
			break;
		case ID_PREVIEW_WINDOW:
			PreviewSelectedWindow();
			break;
		case ID_SEARCH_BUTTON:
			if (HIWORD(wp) == BN_CLICKED) {
				// Получаем текст из строки поиска
				wchar_t searchText[256];
				GetWindowText(searchEdit, searchText, 256);

				// Очищаем список окон на экране
				SendMessage(g_hWndListBox, LB_RESETCONTENT, 0, 0);

				// Выводим окна, содержащие введенный текст в заголовке
				for (const auto& window : g_windows) {
					if (window.title.find(searchText) != std::wstring::npos) {
						SendMessage(g_hWndListBox, LB_ADDSTRING, 0, (LPARAM)window.title.c_str());
					}
				}
			}
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
		// DT_NOCLIP - если текст не влезет в прямоугольник, то выйдет за его пределы и будет виден
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
		//SetBkColor(hdcStatic, RGB(0, 200, 0));
		return (INT_PTR)CreateSolidBrush(RGB(0, 200, 0));

		break;

	case WM_CONTEXTMENU:
		if ((HWND)wp == g_hWndListBox) {
			// Получаем координаты курсора мыши
			POINT cursor;
			GetCursorPos(&cursor);

			// Создаем контекстное меню
			HMENU hPopupMenu = CreatePopupMenu();
			AppendMenu(hPopupMenu, MF_STRING, ID_PREVIEW_WINDOW, L"Preview");
			AppendMenu(hPopupMenu, MF_STRING, ID_OPEN_WINDOW, L"Open");
			AppendMenu(hPopupMenu, MF_STRING, ID_CLOSE_WINDOW, L"Close");

			// Отображаем контекстное меню
			TrackPopupMenu(hPopupMenu, TPM_LEFTALIGN | TPM_TOPALIGN | TPM_LEFTBUTTON, cursor.x, cursor.y, 0, hWnd, NULL);

			// Освобождаем ресурсы контекстного меню
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
	HMENU SubTimeActionMenu = CreateMenu();


	AppendMenu(SubMenu, MF_STRING, OnRefreshField, L"Refresh");
	AppendMenu(SubMenu, MF_SEPARATOR, NULL, NULL);
	AppendMenu(SubMenu, MF_POPUP, (UINT_PTR)SubActionMenu, L"Name Sort");
	AppendMenu(SubMenu, MF_POPUP, (UINT_PTR)SubTimeActionMenu, L"Open time Sort");
	AppendMenu(SubMenu, MF_SEPARATOR, NULL, NULL);
	AppendMenu(SubMenu, MF_STRING, OnExitSoftware, L"Exit");

	AppendMenu(SubActionMenu, MF_STRING, OnNameABSortField, L"A-Z");
	AppendMenu(SubActionMenu, MF_STRING, OnNameBASortField, L"Z-A");

	AppendMenu(SubTimeActionMenu, MF_STRING, OnEarlierSortField, L"Earlier");
	AppendMenu(SubTimeActionMenu, MF_STRING, OnLaterSortField, L"Later");

	AppendMenu(RootMenu, MF_POPUP, (UINT_PTR)SubMenu, L"Menu");

	SetMenu(hWnd, RootMenu);
}

void MainWndAddWidgets(HWND hWnd) {

	// WS_VISIBLE - элемент видим по дефолту, WS_CHILD - элемент является дочерним от базового окна(наодится внутри этого окна)
	hStaticControl = CreateWindowA("static", "Hello, Wind!", WS_VISIBLE | WS_CHILD | ES_CENTER, 5, 5, 80, 30, hWnd, NULL, NULL, NULL);
	windowRectangle = { 780, 50, 5, 110 };


	CreateWindowA("edit", "0", WS_VISIBLE | WS_CHILD | ES_CENTER | ES_NUMBER, 270, 10, 80, 20, hWnd, (HMENU)DigIndexColorR, NULL, NULL);
	CreateWindowA("edit", "0", WS_VISIBLE | WS_CHILD | ES_CENTER | ES_NUMBER, 385, 10, 80, 20, hWnd, (HMENU)DigIndexColorG, NULL, NULL);
	CreateWindowA("edit", "0", WS_VISIBLE | WS_CHILD | ES_CENTER | ES_NUMBER, 500, 10, 80, 20, hWnd, (HMENU)DigIndexColorB, NULL, NULL);


	CreateWindowA("button", "Set color", WS_VISIBLE | WS_CHILD | ES_CENTER, 95, 5, 170, 30, hWnd, (HMENU)OnReadColor, NULL, NULL);

	g_hWndListBox = CreateWindowW(L"LISTBOX", L"", WS_VISIBLE | WS_CHILD | WS_BORDER | LBS_NOINTEGRALHEIGHT | LBS_HASSTRINGS | LBS_NOTIFY,
		5, 190, 770, 610, hWnd, nullptr, nullptr, nullptr);

	// Создание строки поиска
	 searchEdit = CreateWindowW(L"EDIT", L"", WS_VISIBLE | WS_CHILD | WS_BORDER | ES_AUTOHSCROLL,
		5, 135, 600, 25, hWnd, nullptr, nullptr, nullptr);

	 // Создание кнопки с изображением
	 HWND searchButton = CreateWindowW(L"BUTTON", L"", WS_VISIBLE | WS_CHILD | BS_ICON,
		 610, 125, 80, 45, hWnd, (HMENU)ID_SEARCH_BUTTON, nullptr, nullptr);

	 // Загрузка и установка иконки на кнопку
	 HICON hIcon = LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_ICON3));
	 SendMessage(searchButton, BM_SETIMAGE, (WPARAM)IMAGE_ICON, (LPARAM)hIcon);

}

