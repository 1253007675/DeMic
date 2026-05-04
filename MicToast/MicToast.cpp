#include "pch.h"

static DeMic_Host* g_host = nullptr;
static HWND g_hwnd = NULL;
static bool g_muted = false;
#define WM_TOAST (WM_APP + 100)

static HICON g_hIconMic = NULL;
static HICON g_hIconMuted = NULL;
static WCHAR g_pluginDir[MAX_PATH] = {0};   // 保存插件所在目录

void OnMicChanged();

// 从插件所在目录加载图标
void LoadIcons() {
    if (g_hIconMic)  { DestroyIcon(g_hIconMic); g_hIconMic = NULL; }
    if (g_hIconMuted) { DestroyIcon(g_hIconMuted); g_hIconMuted = NULL; }

    WCHAR pathMic[MAX_PATH], pathMuted[MAX_PATH];
    wsprintfW(pathMic, L"%s\\microphone.ico", g_pluginDir);
    wsprintfW(pathMuted, L"%s\\microphone_muted.ico", g_pluginDir);

    g_hIconMic = (HICON)LoadImageW(NULL, pathMic, IMAGE_ICON, 0, 0, LR_LOADFROMFILE);
    g_hIconMuted = (HICON)LoadImageW(NULL, pathMuted, IMAGE_ICON, 0, 0, LR_LOADFROMFILE);
}

LRESULT CALLBACK ToastProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
    switch (msg) {
    case WM_CREATE:
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

        if (g_displayIcon) {
            HICON hIcon = g_muted ? g_hIconMuted : g_hIconMic;
            if (hIcon) {
                int iconSize = min(rc.bottom - rc.top, rc.right - rc.left) - 10;
                int x = (rc.right - rc.left - iconSize) / 2;
                int y = (rc.bottom - rc.top - iconSize) / 2;
                DrawIconEx(hdc, x, y, hIcon, iconSize, iconSize, 0, NULL, DI_NORMAL);
            } else {
                // 图标加载失败时回退到文字
                SetBkMode(hdc, TRANSPARENT);
                SetTextColor(hdc, g_muted ? RGB(255, 60, 60) : RGB(50, 140, 255));
                HFONT font = CreateFontW(
                    g_toastFontSize, 0, 0, 0, FW_MEDIUM, FALSE, FALSE, FALSE,
                    DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                    CLEARTYPE_QUALITY, DEFAULT_PITCH, L"Microsoft YaHei"
                );
                HFONT oldFont = (HFONT)SelectObject(hdc, font);
                const wchar_t* txt = g_muted ? g_muteText : g_unmuteText;
                DrawTextW(hdc, txt, -1, &rc, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
                SelectObject(hdc, oldFont);
                DeleteObject(font);
            }
        } else {
            SetBkMode(hdc, TRANSPARENT);
            SetTextColor(hdc, g_muted ? RGB(255, 60, 60) : RGB(50, 140, 255));
            HFONT font = CreateFontW(
                g_toastFontSize, 0, 0, 0, FW_MEDIUM, FALSE, FALSE, FALSE,
                DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                CLEARTYPE_QUALITY, DEFAULT_PITCH, L"Microsoft YaHei"
            );
            HFONT oldFont = (HFONT)SelectObject(hdc, font);
            const wchar_t* txt = g_muted ? g_muteText : g_unmuteText;
            DrawTextW(hdc, txt, -1, &rc, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
            SelectObject(hdc, oldFont);
            DeleteObject(font);
        }

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

static void OnMenuItemCmd(UINT id) {
    ShowSettingsWindow();
}

static void OnUnload() {
    if (g_hwnd) {
        DestroyWindow(g_hwnd);
        g_hwnd = NULL;
    }
    if (g_hIconMic)  { DestroyIcon(g_hIconMic); g_hIconMic = NULL; }
    if (g_hIconMuted) { DestroyIcon(g_hIconMuted); g_hIconMuted = NULL; }
}

static BOOL OnLoaded(DeMic_Host* host, DeMic_OnLoadedArgs* args) {
    g_host = host;

    // 获取本插件 DLL 所在目录
    HMODULE hModule = NULL;
    GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
        (LPCWSTR)&OnLoaded, &hModule);
    GetModuleFileNameW(hModule, g_pluginDir, MAX_PATH);
    // 去掉文件名，只保留目录
    WCHAR* pSlash = wcsrchr(g_pluginDir, L'\\');
    if (pSlash) *pSlash = L'\0';

    WNDCLASSW wc = {};
    wc.lpfnWndProc = ToastProc;
    wc.hInstance = GetModuleHandle(NULL);
    wc.lpszClassName = L"MicToastWnd";
    RegisterClassW(&wc);

    LoadToastConfig();
    LoadIcons();   // 从插件目录加载图标

    int w = g_toastWidth;
    int h = g_toastHeight;
    int sw = GetSystemMetrics(SM_CXSCREEN);
    int sh = GetSystemMetrics(SM_CYSCREEN);
    if (g_toastX == CW_USEDEFAULT || g_toastY == CW_USEDEFAULT) {
        g_toastX = (sw - w) / 2;
        g_toastY = sh - 180;
    }

    g_hwnd = CreateWindowExW(
        WS_EX_TOPMOST | WS_EX_TOOLWINDOW,
        wc.lpszClassName, L"",
        WS_POPUP,
        g_toastX, g_toastY, w, h,
        NULL, NULL, wc.hInstance, NULL
    );

    HRGN hRgn = CreateRoundRectRgn(0, 0, w, h, g_toastRadius, g_toastRadius);
    SetWindowRgn(g_hwnd, hRgn, TRUE);
    DeleteObject(hRgn);
    ShowWindow(g_hwnd, SW_HIDE);

    MENUITEMINFOW rootMenuItem = { sizeof(rootMenuItem), 0 };
    rootMenuItem.fMask = MIIM_STRING;
    const wchar_t* title = L"调整MicToast";
    rootMenuItem.dwTypeData = const_cast<wchar_t*>(title);
    rootMenuItem.cch = (UINT)wcslen(title);
    host->CreateRootMenuItem(args->State, &rootMenuItem);

    host->SetMicMuteStateListener(args->State, OnMicChanged);
    return TRUE;
}

extern "C" __declspec(dllexport) DeMic_PluginInfo* GetPluginInfo() {
    static DeMic_PluginInfo info = {
        DEMIC_CURRENT_SDK_VERSION,
        L"MicToast",
        { 1, 0 },
        OnLoaded,
        OnMenuItemCmd,
        OnUnload,
        nullptr
    };
    return &info;
}
