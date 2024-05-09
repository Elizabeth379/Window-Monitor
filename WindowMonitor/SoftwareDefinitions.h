#pragma once
#include <windows.h>
#include <vector>
#include <string>
#include <algorithm>
#include <chrono>
#include <iostream>
#include <fstream>

#define OnExitSoftware 1
#define OnRefreshField 2
#define OnReadColor 3
#define ID_OPEN_WINDOW 4
#define ID_CLOSE_WINDOW 5
#define OnNameABSortField 6
#define OnNameBASortField 7
#define OnEarlierSortField 8
#define OnLaterSortField 9
#define ID_PREVIEW_WINDOW 10
#define ID_SEARCH_BUTTON 11

#define DigIndexColorR 200
#define DigIndexColorG 201
#define DigIndexColorB 202


#define TextBufferSize 256

char Buffer[TextBufferSize];
unsigned num;

HWND hStaticControl;
HWND hEditControl;
HWND hNumberControl;
HWND searchEdit;

HWND g_hWndListBox;


char filename[260];
OPENFILENAMEA ofn;

int colorR, colorG, colorB;
HFONT fontRectangle, fontStatic;
COLORREF fontColor;

HBRUSH brushRectangle;
RECT windowRectangle;
PAINTSTRUCT ps;
HDC hdcStatic;

LRESULT CALLBACK SoftwareMainProcedure(HWND hWnd, UINT msg, WPARAM wp, LPARAM lp);
WNDCLASS NewWindowClass(HBRUSH BGColor, HCURSOR Cursor, HINSTANCE hInst, HICON Icon, LPCWSTR Name, WNDPROC Procedure);

void MainWndAddMenus(HWND hWnd);
void MainWndAddWidgets(HWND hWnd);


WNDCLASS wc;


HDC hdc;


// Прототип функции обратного вызова окна
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK PreviewWindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

// Структура для хранения информации об окне
struct WindowInfo {
    HWND hwnd;
    std::wstring title;
    std::chrono::time_point<std::chrono::system_clock> openTime;
};

// Глобальный список окон
std::vector<WindowInfo> g_windows;


// Прототип функции обратного вызова кнопки
LRESULT CALLBACK ButtonProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

// Прототип функции для обновления списка окон
void RefreshWindowList();
void OpenSelectedWindow();
void CloseSelectedWindow();

void ABSort();
void BASort();
bool CompareWindowTitles(const WindowInfo& a, const WindowInfo& b);
void EarlierSort();
void LaterSort();
bool CompareWindowOpenTime(const WindowInfo& a, const WindowInfo& b);
bool CompareWindowOpenTimeReverse(const WindowInfo& a, const WindowInfo& b);

HFONT hFont;
HFONT hOldFont;


