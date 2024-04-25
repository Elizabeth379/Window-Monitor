#include "SoftwareDefinitions.h"
#include "resource.h"

// �������� ������� ��������� ������ ����
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

// ��������� ��� �������� ���������� �� ����
struct WindowInfo {
    HWND hwnd;
    std::wstring title;
};

// ���������� ������ ����
std::vector<WindowInfo> g_windows;

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
        HDC hdc = BeginPaint(hwnd, &ps);

        // ����������� ������ ����
        int y = 5;
        for (const auto& window : g_windows) {
            TextOut(hdc, 5, y, window.title.c_str(), static_cast<int>(window.title.length()));
            y += 20;
        }

        EndPaint(hwnd, &ps);
        break;
    }
    default:
        return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }
    return 0;
}
