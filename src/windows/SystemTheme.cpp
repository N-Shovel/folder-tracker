#include <windows.h>

#include "ui/Theme.h"

// Settings > Personalization > Colors > "Choose your app mode".
bool theme::systemUsesLightTheme() {
    DWORD value = 1;
    DWORD size = sizeof(value);
    RegGetValueW(HKEY_CURRENT_USER, L"Software\\Microsoft\\Windows\\CurrentVersion\\Themes\\Personalize",
                 L"AppsUseLightTheme", RRF_RT_REG_DWORD, nullptr, &value, &size);
    return value != 0;
}
