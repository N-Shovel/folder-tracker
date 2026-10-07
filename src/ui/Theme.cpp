#include "ui/Theme.h"

namespace theme {
namespace {

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
    const bool light = mode == ThemeMode::System ? systemUsesLightTheme() : mode == ThemeMode::Light;
    current = light ? &kLight : &kDark;
}

Color statusColor(Status status) {
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
