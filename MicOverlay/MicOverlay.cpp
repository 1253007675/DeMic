#include <windows.h>
#include "../sdk/DemicPlugin.h"

static HWND g_hwnd = nullptr;
static bool g_muted = false;
static DeMic_Host* g_host = nullptr;

#define WM_UPDATE_OVERLAY (WM_APP + 1)

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
    case WM_UPDATE_OVERLAY:
        g_muted = (bool)wParam;
        ShowWindow(hwnd, SW_SHOWNOACTIVATE);
        InvalidateRect(hwnd, NULL, TRUE);
        SetTimer(hwnd, 1, 1500, NULL);
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

        HBRUSH brush = CreateSolidBrush(RGB(20,20,20));
        FillRect(hdc, &rc, brush);
        DeleteObject(brush);

        SetBkMode(hdc, TRANSPARENT);
        SetTextColor(hdc, RGB(255,255,255));

        HFONT font = CreateFontW(28,0,0,0,FW_BOLD,FALSE,FALSE,FALSE,
            DEFAULT_CHARSET,OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,
            CLEARTYPE_QUALITY,DEFAULT_PITCH,L"Microsoft YaHei");

        SelectObject(hdc, font);

        const wchar_t* text = g_muted ?
            L" 麦克风已关闭" :
            L" 麦克风已开启";

        DrawTextW(hdc, text, -1, &rc,
            DT_CENTER | DT_VCENTER | DT_SINGLELINE);

        DeleteObject(font);
        EndPaint(hwnd, &ps);
        return 0;
    }
    }
    return DefWindowProc(hwnd, msg, wParam, lParam);
}

void OnMicChanged() {
    if (!g_host || !g_hwnd) return;
    BOOL muted = g_host->IsMuted();
    PostMessage(g_hwnd, WM_UPDATE_OVERLAY, muted, 0);
}

BOOL PluginLoaded(DeMic_Host* host, DeMic_OnLoadedArgs* args) {
    g_host = host;

    WNDCLASSW wc = {};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = GetModuleHandle(NULL);
    wc.lpszClassName = L"MicOverlayWnd";

    RegisterClassW(&wc);

    g_hwnd = CreateWindowExW(
        WS_EX_TOPMOST | WS_EX_LAYERED | WS_EX_TOOLWINDOW,
        wc.lpszClassName,
        L"",
        WS_POPUP,
        GetSystemMetrics(SM_CXSCREEN)-360,
        GetSystemMetrics(SM_CYSCREEN)-120,
        320,
        70,
        NULL,NULL,wc.hInstance,NULL
    );

    SetLayeredWindowAttributes(g_hwnd, 0, 230, LWA_ALPHA);
    ShowWindow(g_hwnd, SW_HIDE);

    host->SetMicMuteStateListener(args->State, OnMicChanged);

    return TRUE;
}

void PluginUnload() {
    if (g_hwnd) DestroyWindow(g_hwnd);
}

extern "C" __declspec(dllexport)
DeMic_PluginInfo* GetPluginInfo() {
    static DeMic_PluginInfo info = {
        DEMIC_CURRENT_SDK_VERSION,
        L"Mic Overlay",
        {1,0},
        PluginLoaded,
        nullptr,
        PluginUnload,
        nullptr
    };
    return &info;
}
