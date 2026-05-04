#include "pch.h"
#include <windows.h>
#include "../sdk/DemicPlugin.h"

static HWND g_hwnd = NULL;
static bool g_muted = false;
static DeMic_Host* g_host = nullptr;

#define WM_TOAST (WM_APP + 100)

LRESULT CALLBACK ToastProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
    switch (msg) {
    case WM_TOAST:
        g_muted = (bool)wp;
        ShowWindow(hwnd, SW_SHOWNOACTIVATE);
        InvalidateRect(hwnd, NULL, TRUE);
        KillTimer(hwnd, 1);
        SetTimer(hwnd, 1, 2000, NULL);
        return 0;

    case WM_TIMER:
        ShowWindow(hwnd, SW_HIDE);
        KillTimer(hwnd, 1);
        return 0;

    case WM_PAINT: {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);

        RECT rc;
        GetClientRect(hwnd, &rc);

        HBRUSH bg = CreateSolidBrush(RGB(30, 30, 30));
        HBRUSH oldBrush = (HBRUSH)SelectObject(hdc, bg);
        HPEN pen = CreatePen(PS_NULL, 0, 0);
        HPEN oldPen = (HPEN)SelectObject(hdc, pen);

        RoundRect(hdc, rc.left, rc.top, rc.right, rc.bottom, 25, 25);

        SelectObject(hdc, oldBrush);
        SelectObject(hdc, oldPen);

        DeleteObject(bg);
        DeleteObject(pen);

        SetBkMode(hdc, TRANSPARENT);
        SetTextColor(hdc, g_muted ? RGB(255, 60, 60) : RGB(50, 140, 255));

        HFONT font = CreateFontW(
            28, 0, 0, 0, FW_MEDIUM,
            FALSE, FALSE, FALSE,
            DEFAULT_CHARSET,
            OUT_DEFAULT_PRECIS,
            CLIP_DEFAULT_PRECIS,
            CLEARTYPE_QUALITY,
            DEFAULT_PITCH,
            L"Microsoft YaHei"
        );

        HFONT old = (HFONT)SelectObject(hdc, font);

        const wchar_t* txt = g_muted ? L"麦克风关闭" : L"麦克风启用";

        DrawTextW(hdc, txt, -1, &rc, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

        SelectObject(hdc, old);
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

BOOL PluginLoaded(DeMic_Host* host, DeMic_OnLoadedArgs* args) {
    g_host = host;

    WNDCLASSW wc = {};
    wc.lpfnWndProc = ToastProc;
    wc.hInstance = GetModuleHandle(NULL);
    wc.lpszClassName = L"MicToastWnd";

    RegisterClassW(&wc);

    int w = 125;
    int h = 35;
    int sw = GetSystemMetrics(SM_CXSCREEN);
    int sh = GetSystemMetrics(SM_CYSCREEN);

    g_hwnd = CreateWindowExW(
        WS_EX_TOPMOST | WS_EX_LAYERED | WS_EX_TOOLWINDOW,
        wc.lpszClassName,
        L"",
        WS_POPUP,
        (sw - w) / 2,
        sh - 180,
        w, h,
        NULL, NULL, wc.hInstance, NULL
    );

    HRGN hRgn = CreateRoundRectRgn(0, 0, w, h, 25, 25);
    SetWindowRgn(g_hwnd, hRgn, TRUE);
    DeleteObject(hRgn);

    SetLayeredWindowAttributes(g_hwnd, 0, 235, LWA_ALPHA);
    ShowWindow(g_hwnd, SW_HIDE);

    host->SetMicMuteStateListener(args->State, OnMicChanged);
    return TRUE;
}

void PluginUnload() {
    if (g_hwnd) {
        DestroyWindow(g_hwnd);
        g_hwnd = NULL;
    }
}

extern "C" __declspec(dllexport)
DeMic_PluginInfo* GetPluginInfo() {
    static DeMic_PluginInfo info = {
        DEMIC_CURRENT_SDK_VERSION,
        L"MicToast",
        { 1, 0 },
        PluginLoaded,
        nullptr,
        PluginUnload,
        nullptr
    };
    return &info;
}