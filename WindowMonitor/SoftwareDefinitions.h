#pragma once
#include <windows.h>
#include <vector>
#include <string>
#include <algorithm>

WNDCLASS wc;

PAINTSTRUCT ps;

HDC hdc;



// Прототип функции обратного вызова окна
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

// Структура для хранения информации об окне
struct WindowInfo {
    HWND hwnd;
    std::wstring title;
};

// Глобальный список окон
std::vector<WindowInfo> g_windows;


// Прототип функции обратного вызова кнопки
LRESULT CALLBACK ButtonProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

// Прототип функции для обновления списка окон
void RefreshWindowList();


