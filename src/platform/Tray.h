#pragma once
#include <windows.h>
#include <shellapi.h>

// The icon in the taskbar tray (near the clock). Clicks arrive at the owner as WM_TRAY_ICON.
class Tray {
public:
    Tray(HWND owner, HICON icon);
    ~Tray();

    Tray(const Tray&) = delete;
    Tray& operator=(const Tray&) = delete;

    void restore();  // puts the icon back after Explorer restarts

private:
    NOTIFYICONDATAW data_{};
};
