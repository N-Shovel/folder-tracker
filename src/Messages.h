#pragma once
#include <windows.h>

// Custom window messages shared between parts of the app.
inline constexpr UINT WM_SCAN_FINISHED = WM_APP + 1;  // lParam: ScanResult* (receiver takes ownership)
inline constexpr UINT WM_TRAY_ICON = WM_APP + 2;      // lParam: the mouse message on the tray icon
inline constexpr UINT WM_SHOW_PANEL = WM_APP + 3;     // sent when Folder Tracker is opened a second time

inline constexpr wchar_t kWindowClass[] = L"FolderTrackerBubble";
