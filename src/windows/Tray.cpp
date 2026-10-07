#include "windows/Tray.h"

#include <cwchar>

#include "windows/Messages.h"

Tray::Tray(HWND owner, HICON icon) {
    data_.cbSize = sizeof(data_);
    data_.hWnd = owner;
    data_.uID = 1;
    data_.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
    data_.uCallbackMessage = WM_TRAY_ICON;
    data_.hIcon = icon;
    wcscpy_s(data_.szTip, L"Folder Tracker");
    Shell_NotifyIconW(NIM_ADD, &data_);
}

Tray::~Tray() { Shell_NotifyIconW(NIM_DELETE, &data_); }

void Tray::restore() { Shell_NotifyIconW(NIM_ADD, &data_); }
