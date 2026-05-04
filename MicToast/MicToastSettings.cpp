#include "pch.h"
#include "MicToastSettings.h"
#include <windows.h>
#include <cstdlib>

int g_toastWidth = 360;
int g_toastHeight = 64;
int g_toastRadius = 20;
int g_fontSize = 28;
int g_duration = 2000;

static const wchar_t* INI_PATH = L".\\plugin\\MicToast.ini";

void LoadToastConfig() {
    g_toastWidth = GetPrivateProfileIntW(L"Toast", L"Width", 360, INI_PATH);
    g_toastHeight = GetPrivateProfileIntW(L"Toast", L"Height", 64, INI_PATH);
    g_toastRadius = GetPrivateProfileIntW(L"Toast", L"Radius", 20, INI_PATH);
    g_fontSize = GetPrivateProfileIntW(L"Toast", L"FontSize", 28, INI_PATH);
    g_duration = GetPrivateProfileIntW(L"Toast", L"Duration", 2000, INI_PATH);
}

void SaveToastConfig() {
    wchar_t buf[32];

    wsprintfW(buf, L"%d", g_toastWidth);
    WritePrivateProfileStringW(L"Toast", L"Width", buf, INI_PATH);

    wsprintfW(buf, L"%d", g_toastHeight);
    WritePrivateProfileStringW(L"Toast", L"Height", buf, INI_PATH);

    wsprintfW(buf, L"%d", g_toastRadius);
    WritePrivateProfileStringW(L"Toast", L"Radius", buf, INI_PATH);

    wsprintfW(buf, L"%d", g_fontSize);
    WritePrivateProfileStringW(L"Toast", L"FontSize", buf, INI_PATH);

    wsprintfW(buf, L"%d", g_duration);
    WritePrivateProfileStringW(L"Toast", L"Duration", buf, INI_PATH);
}

LRESULT CALLBACK SettingsProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
    switch (msg) {
    case WM_CREATE:
        CreateWindowW(L"STATIC", L"宽度", WS_VISIBLE|WS_CHILD, 20,20,80,25, hwnd, NULL, NULL, NULL);
        CreateWindowW(L"EDIT", L"360", WS_VISIBLE|WS_CHILD|WS_BORDER, 120,20,100,25, hwnd, (HMENU)1, NULL, NULL);

        CreateWindowW(L"STATIC", L"高度", WS_VISIBLE|WS_CHILD, 20,60,80,25, hwnd, NULL, NULL, NULL);
        CreateWindowW(L"EDIT", L"64", WS_VISIBLE|WS_CHILD|WS_BORDER, 120,60,100,25, hwnd, (HMENU)2, NULL, NULL);

        CreateWindowW(L"STATIC", L"圆角", WS_VISIBLE|WS_CHILD, 20,100,80,25, hwnd, NULL, NULL, NULL);
        CreateWindowW(L"EDIT", L"20", WS_VISIBLE|WS_CHILD|WS_BORDER, 120,100,100,25, hwnd, (HMENU)3, NULL, NULL);

        CreateWindowW(L"STATIC", L"字体", WS_VISIBLE|WS_CHILD, 20,140,80,25, hwnd, NULL, NULL, NULL);
        CreateWindowW(L"EDIT", L"28", WS_VISIBLE|WS_CHILD|WS_BORDER, 120,140,100,25, hwnd, (HMENU)4, NULL, NULL);

        CreateWindowW(L"STATIC", L"时长", WS_VISIBLE|WS_CHILD, 20,180,80,25, hwnd, NULL, NULL, NULL);
        CreateWindowW(L"EDIT", L"2000", WS_VISIBLE|WS_CHILD|WS_BORDER, 120,180,100,25, hwnd, (HMENU)5, NULL, NULL);

        CreateWindowW(L"BUTTON", L"保存", WS_VISIBLE|WS_CHILD, 80,230,120,35, hwnd, (HMENU)100, NULL, NULL);
        return 0;

    case WM_COMMAND:
        if (LOWORD(wp) == 100) {
            wchar_t buf[32];

            GetWindowTextW(GetDlgItem(hwnd,1), buf, 32);
            g_toastWidth = _wtoi(buf);

            GetWindowTextW(GetDlgItem(hwnd,2), buf, 32);
            g_toastHeight = _wtoi(buf);

            GetWindowTextW(GetDlgItem(hwnd,3), buf, 32);
            g_toastRadius = _wtoi(buf);

            GetWindowTextW(GetDlgItem(hwnd,4), buf, 32);
            g_fontSize = _wtoi(buf);

            GetWindowTextW(GetDlgItem(hwnd,5), buf, 32);
            g_duration = _wtoi(buf);

            SaveToastConfig();

            MessageBoxW(hwnd, L"保存成功", L"MicToast", MB_OK);
        }
        return 0;
    }
    return DefWindowProc(hwnd, msg, wp, lp);
}

void ShowSettingsWindow() {
    WNDCLASSW wc = {};
    wc.lpfnWndProc = SettingsProc;
    wc.hInstance = GetModuleHandle(NULL);
    wc.lpszClassName = L"MicToastSettingsWnd";

    RegisterClassW(&wc);

    CreateWindowW(
        wc.lpszClassName,
        L"MicToast 设置",
        WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU,
        500, 300, 280, 330,
        NULL, NULL, wc.hInstance, NULL
    );
}

