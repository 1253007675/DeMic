#include "pch.h"
#include "../sdk/DemicPlugin.h"
#include <windows.h>
#include "MicToastSettings.h"

static DeMic_Host* g_host = nullptr;
static HWND g_hwnd = NULL;
static bool g_muted = false;
#define WM_TOAST (WM_APP + 100)

void OnMicChanged();

// 窗口过程
LRESULT CALLBACK ToastProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
    switch (msg) {
    case WM_CREATE:
        // 加载配置并设置窗口位置与大小
        LoadToastConfig();
        if (g_toastX == CW_USEDEFAULT || g_toastY == CW_USEDEFAULT) {
            int sw = GetSystemMetrics(SM_CXSCREEN);
            int sh = GetSystemMetrics(SM_CYSCREEN);
            g_toastX = (sw - g_toastWidth) / 2;
            g_toastY = sh - 180;
        }
        SetWindowPos(hwnd, NULL, g_toastX, g_toastY, g_toastWidth, g_toastHeight, SWP_NOZORDER | SWP_NOACTIVATE);
        SetTimer(hwnd, 1, g_toastDuration, NULL);
        return 0;

    case WM_TOAST:
        g_muted = (bool)wp;
        ShowWindow(hwnd, SW_SHOWNOACTIVATE);
        InvalidateRect(hwnd, NULL, TRUE);
        KillTimer(hwnd, 1);
        SetTimer(hwnd, 1, g_toastDuration, NULL);
        return 0;

    case WM_TIMER:
        ShowWindow(hwnd, SW_HIDE);
        KillTimer(hwnd, 1);
        return 0;

    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);
        RECT rc;
        GetClientRect(hwnd, &rc);

        HBRUSH bg = CreateSolidBrush(RGB(30, 30, 30));
        HBRUSH oldBrush = (HBRUSH)SelectObject(hdc, bg);
        HPEN pen = CreatePen(PS_NULL, 0, 0);
        HPEN oldPen = (HPEN)SelectObject(hdc, pen);
        RoundRect(hdc, rc.left, rc.top, rc.right, rc.bottom, g_toastRadius, g_toastRadius);
        SelectObject(hdc, oldBrush);
        SelectObject(hdc, oldPen);
        DeleteObject(bg);
        DeleteObject(pen);

        SetBkMode(hdc, TRANSPARENT);
        SetTextColor(hdc, g_muted ? RGB(255, 60, 60) : RGB(50, 140, 255));
        HFONT font = CreateFontW(
            g_toastFontSize, 0, 0, 0, FW_MEDIUM, FALSE, FALSE, FALSE,
            DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
            CLEARTYPE_QUALITY, DEFAULT_PITCH, L"Microsoft YaHei"
        );
        HFONT oldFont = (HFONT)SelectObject(hdc, font);
        const wchar_t* txt = g_muted ? L"麦克风关闭" : L"麦克风启用";
        DrawTextW(hdc, txt, -1, &rc, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
        SelectObject(hdc, oldFont);
        DeleteObject(font);
        EndPaint(hwnd, &ps);
        return 0;
    }

    case WM_DESTROY:
        KillTimer(hwnd, 1);
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProc(hwnd, msg, wp, lp);
}

void OnMicChanged() {
    if (!g_host || !g_hwnd) return;
    BOOL muted = g_host->IsMuted();
    PostMessage(g_hwnd, WM_TOAST, muted, 0);
}

// ---------- SDK 规定入口 ----------
static void OnMenuItemCmd(UINT id) {
    if (id == 0) { // 0 代表根菜单项被点击
        ShowSettingsWindow();
    }
}

static void OnUnload() {
    if (g_hwnd) {
        DestroyWindow(g_hwnd);
        g_hwnd = NULL;
    }
}

static BOOL OnLoaded(DeMic_Host* host, DeMic_OnLoadedArgs* args) {
    g_host = host;

    // 注册窗口类
    WNDCLASSW wc = {};
    wc.lpfnWndProc = ToastProc;
    wc.hInstance = GetModuleHandle(NULL);
    wc.lpszClassName = L"MicToastWnd";
    RegisterClassW(&wc);

    // 加载配置
    LoadToastConfig();
    int w = g_toastWidth;
    int h = g_toastHeight;
    int sw = GetSystemMetrics(SM_CXSCREEN);
    int sh = GetSystemMetrics(SM_CYSCREEN);
    if (g_toastX == CW_USEDEFAULT || g_toastY == CW_USEDEFAULT) {
        g_toastX = (sw - w) / 2;
        g_toastY = sh - 180;
    }

    g_hwnd = CreateWindowExW(
        WS_EX_TOPMOST | WS_EX_LAYERED | WS_EX_TOOLWINDOW,
        wc.lpszClassName, L"",
        WS_POPUP,
        g_toastX, g_toastY, w, h,
        NULL, NULL, wc.hInstance, NULL
    );

    HRGN hRgn = CreateRoundRectRgn(0, 0, w, h, g_toastRadius, g_toastRadius);
    SetWindowRgn(g_hwnd, hRgn, TRUE);
    DeleteObject(hRgn);
    SetLayeredWindowAttributes(g_hwnd, 0, 235, LWA_ALPHA);
    ShowWindow(g_hwnd, SW_HIDE);

    // --- 关键：创建根菜单项 ---
    MENUITEMINFOW rootMenuItem = { sizeof(rootMenuItem), 0 };
    rootMenuItem.fMask = MIIM_STRING;
    const wchar_t* title = L"调整MicToast"; // 菜单上显示的文字
    rootMenuItem.dwTypeData = const_cast<wchar_t*>(title);
    rootMenuItem.cch = (UINT)wcslen(title);
    if (!host->CreateRootMenuItem(args->State, &rootMenuItem)) {
        return FALSE; // 创建菜单失败则插件加载失败
    }

    host->SetMicMuteStateListener(args->State, OnMicChanged);
    return TRUE;
}

// 导出信息结构体
extern "C" __declspec(dllexport) DeMic_PluginInfo* GetPluginInfo() {
    static DeMic_PluginInfo info = {
        DEMIC_CURRENT_SDK_VERSION,
        L"MicToast",
        { 1, 0 },
        OnLoaded,       // OnLoaded
        OnMenuItemCmd,  // OnMenuItemCmd（设置为非NULL才显示菜单）
        OnUnload,       // OnUnload
        nullptr         // OnPreTranslateMessage
    };
    return &info;
}
