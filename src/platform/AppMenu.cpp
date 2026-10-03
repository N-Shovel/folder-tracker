#include "platform/AppMenu.h"

#include "platform/Shell.h"

MenuCommand showAppMenu(HWND owner, bool bubbleVisible) {
    HMENU menu = CreatePopupMenu();
    AppendMenuW(menu, MF_STRING, UINT_PTR(MenuCommand::ToggleBubble), bubbleVisible ? L"Hide bubble" : L"Show bubble");
    AppendMenuW(menu, MF_STRING | (startsWithWindows() ? MF_CHECKED : 0), UINT_PTR(MenuCommand::StartWithWindows),
                L"Start with Windows");
    AppendMenuW(menu, MF_SEPARATOR, 0, nullptr);
    AppendMenuW(menu, MF_STRING, UINT_PTR(MenuCommand::Quit), L"Quit Folder Tracker");

    POINT cursor;
    GetCursorPos(&cursor);
    SetForegroundWindow(owner);  // required, or the menu will not close when clicking elsewhere
    const UINT chosen = TrackPopupMenu(menu, TPM_RETURNCMD | TPM_RIGHTBUTTON | TPM_NONOTIFY, cursor.x, cursor.y, 0,
                                       owner, nullptr);
    PostMessageW(owner, WM_NULL, 0, 0);
    DestroyMenu(menu);
    return static_cast<MenuCommand>(chosen);
}
