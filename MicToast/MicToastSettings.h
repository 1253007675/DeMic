#pragma once
#include <windows.h>

extern int g_toastWidth;
extern int g_toastHeight;
extern int g_toastRadius;
extern int g_toastFontSize;
extern int g_toastDuration;
extern int g_toastX;
extern int g_toastY;
extern wchar_t g_muteText[64];
extern wchar_t g_unmuteText[64];
extern bool g_displayIcon;

void LoadToastConfig();
void SaveToastConfig();
void ShowSettingsWindow();
