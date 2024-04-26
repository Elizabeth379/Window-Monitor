#include "SoftwareDefinitions.h"
#include "resource.h"

// Функция для обновления списка окон
void RefreshWindowList() {
    g_windows.clear();

    // Получение списка всех верхних окон
    HWND hwnd = GetTopWindow(nullptr);
    while (hwnd != nullptr) {
        if (IsWindowVisible(hwnd)) {
            wchar_t title[256];
            GetWindowText(hwnd, title, sizeof(title) / sizeof(wchar_t));
            g_windows.push_back({ hwnd, title });
        }
        hwnd = GetNextWindow(hwnd, GW_HWNDNEXT);
    }

    // Сортировка списка окон по заголовкам
    std::sort(g_windows.begin(), g_windows.end(), [](const WindowInfo& a, const WindowInfo& b) {
        return a.title < b.title;
        });
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    // Регистрация класса окна
    const wchar_t CLASS_NAME[] = L"WindowListClass";

    wc = {};

    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;
    wc.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ICON1)); // Загрузка иконки

    RegisterClass(&wc);

    // Создание окна
    HWND hwnd = CreateWindowEx(
        0,                              // Optional window styles.
        CLASS_NAME,                     // Window class
        L"Window List",                 // Window text
        WS_OVERLAPPEDWINDOW,            // Window style

        // Size and position
        CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,

        NULL,       // Parent window    
        NULL,       // Menu
        hInstance,  // Instance handle
        NULL        // Additional application data
    );

    HWND button = CreateWindow(
        L"BUTTON",                      // Predefined class; Unicode assumed
        L"Refresh List",                // Button text
        WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,  // Styles
        10,                             // x position
        10,                             // y position
        120,                            // Button width
        30,                             // Button height
        hwnd,                           // Parent window
        (HMENU)IDI_ICON2,      // No menu.
        (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE),
        NULL);                          // Pointer not needed.


    if (hwnd == NULL) {
        return 0;
    }

    // Отображение окна
    ShowWindow(hwnd, nCmdShow);

    // Запуск цикла обработки сообщений
    MSG msg = {};
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 0;
}


// Функция обработки сообщений окна
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    int y;
    switch (uMsg) {
    case WM_CREATE:
        SendMessage(hwnd, WM_SETICON, ICON_BIG, (LPARAM)wc.hIcon);
        RefreshWindowList();
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    case WM_PAINT: {
        ps;
        hdc = BeginPaint(hwnd, &ps);

        // Отображение списка окон
        y = 5;
        for (const auto& window : g_windows) {
            TextOut(hdc, 5, y, window.title.c_str(), static_cast<int>(window.title.length()));
            y += 20;
        }

        EndPaint(hwnd, &ps);
        break;
    case WM_COMMAND:
        switch (LOWORD(wParam)) {
        case IDI_ICON2:
            RefreshWindowList();
            InvalidateRect(hwnd, NULL, TRUE); // Перерисовка окна после обновления списка
            break;
        }
        break;
    }
    default:
        return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }
    return 0;
}
