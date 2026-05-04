#include "pch.h"
#include "MicToastSettings.h"
#include <windows.h>
#include <stdio.h>

int g_toastWidth = 360;
int g_toastHeight = 64;
int g_toastRadius = 20;
int g_toastFontSize = 28;
int g_toastDuration = 2000;
int g_toastX = CW_USEDEFAULT;
int g_toastY = CW_USEDEFAULT;
wchar_t g_muteText[64] = L"麦克风关闭";
wchar_t g_unmuteText[64] = L"麦克风启用";
bool g_displayIcon = false;

static const wchar_t* INI_PATH = L".\\plugin\\MicToast.ini";

void LoadToastConfig() {
    g_toastWidth = GetPrivateProfileIntW(L"Toast", L"Width", 360, INI_PATH);
    g_toastHeight = GetPrivateProfileIntW(L"Toast", L"Height", 64, INI_PATH);
    g_toastRadius = GetPrivateProfileIntW(L"Toast", L"Radius", 20, INI_PATH);
    g_toastFontSize = GetPrivateProfileIntW(L"Toast", L"FontSize", 28, INI_PATH);
    g_toastDuration = GetPrivateProfileIntW(L"Toast", L"Duration", 2000, INI_PATH);
    g_toastX = GetPrivateProfileIntW(L"Toast", L"X", CW_USEDEFAULT, INI_PATH);
    g_toastY = GetPrivateProfileIntW(L"Toast", L"Y", CW_USEDEFAULT, INI_PATH);
    GetPrivateProfileStringW(L"Toast", L"MuteText", L"麦克风关闭", g_muteText, 64, INI_PATH);
    GetPrivateProfileStringW(L"Toast", L"UnmuteText", L"麦克风启用", g_unmuteText, 64, INI_PATH);
    g_displayIcon = GetPrivateProfileIntW(L"Toast", L"Icon", 0, INI_PATH) != 0;
}

void SaveToastConfig() {
    wchar_t buf[32];
    wsprintfW(buf, L"%d", g_toastWidth);
    WritePrivateProfileStringW(L"Toast", L"Width", buf, INI_PATH);
    wsprintfW(buf, L"%d", g_toastHeight);
    WritePrivateProfileStringW(L"Toast", L"Height", buf, INI_PATH);
    wsprintfW(buf, L"%d", g_toastRadius);
    WritePrivateProfileStringW(L"Toast", L"Radius", buf, INI_PATH);
    wsprintfW(buf, L"%d", g_toastFontSize);
    WritePrivateProfileStringW(L"Toast", L"FontSize", buf, INI_PATH);
    wsprintfW(buf, L"%d", g_toastDuration);
    WritePrivateProfileStringW(L"Toast", L"Duration", buf, INI_PATH);
    wsprintfW(buf, L"%d", g_toastX);
    WritePrivateProfileStringW(L"Toast", L"X", buf, INI_PATH);
    wsprintfW(buf, L"%d", g_toastY);
    WritePrivateProfileStringW(L"Toast", L"Y", buf, INI_PATH);
    WritePrivateProfileStringW(L"Toast", L"MuteText", g_muteText, INI_PATH);
    WritePrivateProfileStringW(L"Toast", L"UnmuteText", g_unmuteText, INI_PATH);
    wsprintfW(buf, L"%d", g_displayIcon ? 1 : 0);
    WritePrivateProfileStringW(L"Toast", L"Icon", buf, INI_PATH);
}

LRESULT CALLBACK SettingsProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
    static HWND hEdtWidth, hEdtHeight, hEdtRadius, hEdtFont, hEdtDuration;
    static HWND hEdtPosX, hEdtPosY, hEdtMute, hEdtUnmute;
    static HWND hChkIcon;

    switch (msg) {
    case WM_CREATE: {
        CreateWindowW(L"STATIC", L"宽度", WS_VISIBLE | WS_CHILD, 20, 20, 80, 25, hwnd, NULL, NULL, NULL);
        hEdtWidth = CreateWindowW(L"EDIT", L"360", WS_VISIBLE | WS_CHILD | WS_BORDER, 120, 20, 100, 25, hwnd, (HMENU)1, NULL, NULL);

        CreateWindowW(L"STATIC", L"高度", WS_VISIBLE | WS_CHILD, 20, 60, 80, 25, hwnd, NULL, NULL, NULL);
        hEdtHeight = CreateWindowW(L"EDIT", L"64", WS_VISIBLE | WS_CHILD | WS_BORDER, 120, 60, 100, 25, hwnd, (HMENU)2, NULL, NULL);

        CreateWindowW(L"STATIC", L"圆角", WS_VISIBLE | WS_CHILD, 20, 100, 80, 25, hwnd, NULL, NULL, NULL);
        hEdtRadius = CreateWindowW(L"EDIT", L"20", WS_VISIBLE | WS_CHILD | WS_BORDER, 120, 100, 100, 25, hwnd, (HMENU)3, NULL, NULL);

        CreateWindowW(L"STATIC", L"字体", WS_VISIBLE | WS_CHILD, 20, 140, 80, 25, hwnd, NULL, NULL, NULL);
        hEdtFont = CreateWindowW(L"EDIT", L"28", WS_VISIBLE | WS_CHILD | WS_BORDER, 120, 140, 100, 25, hwnd, (HMENU)4, NULL, NULL);

        CreateWindowW(L"STATIC", L"时长(ms)", WS_VISIBLE | WS_CHILD, 20, 180, 80, 25, hwnd, NULL, NULL, NULL);
        hEdtDuration = CreateWindowW(L"EDIT", L"2000", WS_VISIBLE | WS_CHILD | WS_BORDER, 120, 180, 100, 25, hwnd, (HMENU)5, NULL, NULL);

        CreateWindowW(L"STATIC", L"X坐标", WS_VISIBLE | WS_CHILD, 20, 220, 80, 25, hwnd, NULL, NULL, NULL);
        hEdtPosX = CreateWindowW(L"EDIT", L"", WS_VISIBLE | WS_CHILD | WS_BORDER, 120, 220, 100, 25, hwnd, (HMENU)6, NULL, NULL);

        CreateWindowW(L"STATIC", L"Y坐标", WS_VISIBLE | WS_CHILD, 20, 260, 80, 25, hwnd, NULL, NULL, NULL);
        hEdtPosY = CreateWindowW(L"EDIT", L"", WS_VISIBLE | WS_CHILD | WS_BORDER, 120, 260, 100, 25, hwnd, (HMENU)7, NULL, NULL);

        CreateWindowW(L"STATIC", L"静音文字", WS_VISIBLE | WS_CHILD, 20, 300, 80, 25, hwnd, NULL, NULL, NULL);
        hEdtMute = CreateWindowW(L"EDIT", g_muteText, WS_VISIBLE | WS_CHILD | WS_BORDER, 120, 300, 200, 25, hwnd, (HMENU)8, NULL, NULL);

        CreateWindowW(L"STATIC", L"开启文字", WS_VISIBLE | WS_CHILD, 20, 340, 80, 25, hwnd, NULL, NULL, NULL);
        hEdtUnmute = CreateWindowW(L"EDIT", g_unmuteText, WS_VISIBLE | WS_CHILD | WS_BORDER, 120, 340, 200, 25, hwnd, (HMENU)9, NULL, NULL);

        // 图标复选框
        hChkIcon = CreateWindowW(L"BUTTON", L"使用图标显示（ico）", WS_VISIBLE | WS_CHILD | BS_AUTOCHECKBOX,
            20, 380, 200, 25, hwnd, (HMENU)10, NULL, NULL);
        if (g_displayIcon) SendMessageW(hChkIcon, BM_SETCHECK, BST_CHECKED, 0);

        CreateWindowW(L"BUTTON", L"保存并应用", WS_VISIBLE | WS_CHILD, 20, 420, 200, 30, hwnd, (HMENU)100, NULL, NULL);

        // 窗口高度调整
        SetWindowPos(hwnd, NULL, 0, 0, 360, 510, SWP_NOMOVE | SWP_NOZORDER);

        // 填充数值
        wchar_t buf[32];
        wsprintfW(buf, L"%d", g_toastWidth);  SetWindowTextW(hEdtWidth, buf);
        wsprintfW(buf, L"%d", g_toastHeight); SetWindowTextW(hEdtHeight, buf);
        wsprintfW(buf, L"%d", g_toastRadius); SetWindowTextW(hEdtRadius, buf);
        wsprintfW(buf, L"%d", g_toastFontSize); SetWindowTextW(hEdtFont, buf);
        wsprintfW(buf, L"%d", g_toastDuration); SetWindowTextW(hEdtDuration, buf);
        wsprintfW(buf, L"%d", g_toastX);      SetWindowTextW(hEdtPosX, buf);
        wsprintfW(buf, L"%d", g_toastY);      SetWindowTextW(hEdtPosY, buf);
        return 0;
    }

    case WM_COMMAND: {
        if (LOWORD(wp) == 100) {
            wchar_t buf[64];
            GetWindowTextW(hEdtWidth, buf, 32); g_toastWidth = _wtoi(buf);
            GetWindowTextW(hEdtHeight, buf, 32); g_toastHeight = _wtoi(buf);
            GetWindowTextW(hEdtRadius, buf, 32); g_toastRadius = _wtoi(buf);
            GetWindowTextW(hEdtFont, buf, 32); g_toastFontSize = _wtoi(buf);
            GetWindowTextW(hEdtDuration, buf, 32); g_toastDuration = _wtoi(buf);
            GetWindowTextW(hEdtPosX, buf, 32); g_toastX = _wtoi(buf);
            GetWindowTextW(hEdtPosY, buf, 32); g_toastY = _wtoi(buf);
            GetWindowTextW(hEdtMute, g_muteText, 64);
            GetWindowTextW(hEdtUnmute, g_unmuteText, 64);
            g_displayIcon = (SendMessageW(hChkIcon, BM_GETCHECK, 0, 0) == BST_CHECKED);

            SaveToastConfig();
            MessageBoxW(hwnd, L"设置已保存，下次弹窗生效", L"提示", MB_OK);
            DestroyWindow(hwnd);
        }
        return 0;
    }

    case WM_CLOSE:
        DestroyWindow(hwnd);
        return 0;
    }
    return DefWindowProcW(hwnd, msg, wp, lp);
}

void ShowSettingsWindow() {
    static bool registered = false;
    if (!registered) {
        WNDCLASSW wc = {};
        wc.lpfnWndProc = SettingsProc;
        wc.hInstance = GetModuleHandleW(NULL);
        wc.hCursor = LoadCursorW(NULL, IDC_ARROW);
        wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
        wc.lpszClassName = L"MicToastSettings";
        RegisterClassW(&wc);
        registered = true;
    }

    HWND hwnd = CreateWindowExW(0, L"MicToastSettings", L"MicToast 设置",
        WS_OVERLAPPEDWINDOW & ~WS_THICKFRAME & ~WS_MAXIMIZEBOX,
        CW_USEDEFAULT, CW_USEDEFAULT, 360, 510,
        NULL, NULL, GetModuleHandleW(NULL), NULL);
    ShowWindow(hwnd, SW_SHOW);
    UpdateWindow(hwnd);
}
