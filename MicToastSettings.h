#pragma once
#include <windows.h>

// 外部全局配置变量
extern int g_toastWidth;
extern int g_toastHeight;
extern int g_toastRadius;
extern int g_toastFontSize;
extern int g_toastDuration;
extern int g_toastX;
extern int g_toastY;

void LoadToastConfig();
void SaveToastConfig();
void ShowSettingsWindow();
