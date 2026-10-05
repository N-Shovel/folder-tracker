#pragma once
#include <windows.h>

#include "platform/Settings.h"

enum class MenuCommand { None, ToggleBubble, StartWithWindows, ThemeSystem, ThemeDark, ThemeLight, Quit };

// The right-click menu (on the bubble or the tray icon), shown at the mouse.
MenuCommand showAppMenu(HWND owner, bool bubbleVisible, ThemeMode theme);
