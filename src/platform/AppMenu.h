#pragma once
#include <windows.h>

enum class MenuCommand { None, ToggleBubble, StartWithWindows, Quit };

// The right-click menu (on the bubble or the tray icon), shown at the mouse.
MenuCommand showAppMenu(HWND owner, bool bubbleVisible);
