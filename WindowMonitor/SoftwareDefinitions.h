#pragma once
#include <windows.h>
#include <vector>
#include <string>
#include <algorithm>

WNDCLASS wc;

PAINTSTRUCT ps;

HDC hdc;



// �������� ������� ��������� ������ ����
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

// ��������� ��� �������� ���������� �� ����
struct WindowInfo {
    HWND hwnd;
    std::wstring title;
};

// ���������� ������ ����
std::vector<WindowInfo> g_windows;


// �������� ������� ��������� ������ ������
LRESULT CALLBACK ButtonProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

// �������� ������� ��� ���������� ������ ����
void RefreshWindowList();


