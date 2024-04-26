#include "SoftwareDefinitions.h"
#include "resource.h"

// ������� ��� ���������� ������ ����
void RefreshWindowList() {
    g_windows.clear();

    // ��������� ������ ���� ������� ����
    HWND hwnd = GetTopWindow(nullptr);
    while (hwnd != nullptr) {
        if (IsWindowVisible(hwnd)) {
            wchar_t title[256];
            GetWindowText(hwnd, title, sizeof(title) / sizeof(wchar_t));
            g_windows.push_back({ hwnd, title });
        }
        hwnd = GetNextWindow(hwnd, GW_HWNDNEXT);
    }

    // ���������� ������ ���� �� ����������
    std::sort(g_windows.begin(), g_windows.end(), [](const WindowInfo& a, const WindowInfo& b) {
        return a.title < b.title;
        });
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    // ����������� ������ ����
    const wchar_t CLASS_NAME[] = L"WindowListClass";

    wc = {};

    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;
    wc.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ICON1)); // �������� ������

    RegisterClass(&wc);

    // �������� ����
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

    // ����������� ����
    ShowWindow(hwnd, nCmdShow);

    // ������ ����� ��������� ���������
    MSG msg = {};
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 0;
}


// ������� ��������� ��������� ����
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    int y;
    int cellHeight;
    int maxWidth;
    int cellWidth;
    int maxHeight;


    switch (uMsg) {
    case WM_CREATE:
        SendMessage(hwnd, WM_SETICON, ICON_BIG, (LPARAM)wc.hIcon);
        RefreshWindowList();
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    case WM_PAINT: {
        PAINTSTRUCT ps;
        hdc = BeginPaint(hwnd, &ps);

        // ��������� ������ � ������� ��������
        hFont = CreateFont(14, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
            OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE,
            L"Segoe UI");
        hOldFont = static_cast<HFONT>(SelectObject(hdc, hFont));

        // ��������� ����� ���� � �����
        SetBkColor(hdc, RGB(255, 255, 255));
        SetTextColor(hdc, RGB(0, 0, 0));

        // ����������� �������� ���������� ������� ����
        RECT clientRect;
        GetClientRect(hwnd, &clientRect);

        // ����������� �������� � ������� ��� ����������� ������ ���� � ������ �������� ����, ������� � �����
        cellHeight = 20;
        y = 0;
        maxWidth = clientRect.right - clientRect.left; // ������ ���������� ������� ����
        cellWidth = maxWidth; // ������ ������ ������ ����� ������ ���������� �������
        maxHeight = clientRect.bottom - clientRect.top; // ������ ���������� ������� ����

        // ����������� ������ ����
        for (const auto& window : g_windows) {
            RECT textRect;
            textRect.left = maxWidth / 2; // ������ ������ ���������� ���� �� �����������
            textRect.top = y; // ������� ������ �� ���������
            textRect.right = maxWidth; // ������ ������� ������ ����� ������ ������� ����
            textRect.bottom = textRect.top + cellHeight; // ������ ������� ������

            // ����������� ����� ������ ������ ����
            DrawText(hdc, window.title.c_str(), -1, &textRect, DT_LEFT | DT_WORDBREAK | DT_EDITCONTROL);

            y += cellHeight; // ��������� � ��������� ������� ��� ��������� ������
        }

        // �������������� ����������� ������
        SelectObject(hdc, hOldFont);
        DeleteObject(hFont);

        EndPaint(hwnd, &ps);
        break;

    case WM_COMMAND:
        switch (LOWORD(wParam)) {
        case IDI_ICON2:
            RefreshWindowList();
            InvalidateRect(hwnd, NULL, TRUE); // ����������� ���� ����� ���������� ������
            break;
        }
        break;
    }
    default:
        return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }
    return 0;
}
