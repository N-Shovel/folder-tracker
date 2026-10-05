#include "ui/Theme.h"

#include <windows.h>

namespace theme {
namespace {

bool windowsUsesLightTheme() {
    DWORD value = 1;
    DWORD size = sizeof(value);
    RegGetValueW(HKEY_CURRENT_USER, L"Software\\Microsoft\\Windows\\CurrentVersion\\Themes\\Personalize",
                 L"AppsUseLightTheme", RRF_RT_REG_DWORD, nullptr, &value, &size);
    return value != 0;
}

const Palette* current = nullptr;
ThemeMode mode = ThemeMode::System;

}  // namespace

const Palette& palette() {
    if (!current) refreshPalette();
    return *current;
}

void setThemeMode(ThemeMode newMode) {
    mode = newMode;
    refreshPalette();
}

void refreshPalette() {
    const bool light = mode == ThemeMode::System ? windowsUsesLightTheme() : mode == ThemeMode::Light;
    current = light ? &kLight : &kDark;
}

D2D1_COLOR_F statusColor(Status status) {
    switch (status) {
        case Status::GitHub: return palette().github;
        case Status::Local: return palette().local;
        case Status::Untracked: return palette().untracked;
        default: return palette().textMuted;
    }
}

bool hasBadge(Status status) {
    return status == Status::GitHub || status == Status::Local || status == Status::Untracked;
}

}  // namespace theme
