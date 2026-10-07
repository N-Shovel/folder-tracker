#include "windows/AppMenu.h"

#include "windows/Shell.h"

MenuCommand showAppMenu(HWND owner, bool bubbleVisible, ThemeMode theme) {
    HMENU themeMenu = CreatePopupMenu();
    AppendMenuW(themeMenu, MF_STRING, UINT_PTR(MenuCommand::ThemeSystem), L"Same as Windows");
    AppendMenuW(themeMenu, MF_STRING, UINT_PTR(MenuCommand::ThemeDark), L"Dark");
    AppendMenuW(themeMenu, MF_STRING, UINT_PTR(MenuCommand::ThemeLight), L"Light");
    CheckMenuRadioItem(themeMenu, UINT(MenuCommand::ThemeSystem), UINT(MenuCommand::ThemeLight),
                       UINT(MenuCommand::ThemeSystem) + UINT(theme), MF_BYCOMMAND);

    HMENU menu = CreatePopupMenu();
    AppendMenuW(menu, MF_STRING, UINT_PTR(MenuCommand::ToggleBubble), bubbleVisible ? L"Hide bubble" : L"Show bubble");
    AppendMenuW(menu, MF_STRING | (startsWithWindows() ? MF_CHECKED : 0), UINT_PTR(MenuCommand::StartWithWindows),
                L"Start with Windows");
    AppendMenuW(menu, MF_POPUP, UINT_PTR(themeMenu), L"Theme");
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
