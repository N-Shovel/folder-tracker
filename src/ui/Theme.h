#pragma once
#include "core/FolderNode.h"
#include "core/Settings.h"
#include "ui/Geometry.h"

// Look and feel: colors, sizes, fonts and animation timing. Every other UI file reads from here.
// All sizes are in DIPs (pixels at 100% scaling).
namespace theme {

constexpr Color rgb(unsigned hex, float alpha = 1.0f) {
    return {((hex >> 16) & 0xFF) / 255.0f, ((hex >> 8) & 0xFF) / 255.0f, (hex & 0xFF) / 255.0f, alpha};
}

constexpr Color withAlpha(Color color, float alpha) { return {color.r, color.g, color.b, color.a * alpha}; }

struct Palette {
    Color surface, surfaceRaised, surfaceHover, border;
    Color text, textMuted, accent, danger, onAccent;
    Color github, local, untracked;
    Color shadow;
};

inline constexpr Palette kDark{
    .surface = rgb(0x161b22), .surfaceRaised = rgb(0x1c222b), .surfaceHover = rgb(0x262e3a), .border = rgb(0x30363d),
    .text = rgb(0xe6edf3), .textMuted = rgb(0x8b949e), .accent = rgb(0x58a6ff), .danger = rgb(0xf85149),
    .onAccent = rgb(0xffffff),
    .github = rgb(0x3fb950), .local = rgb(0xd29922), .untracked = rgb(0x8b949e),
    .shadow = rgb(0x000000, 0.5f),
};

inline constexpr Palette kLight{
    .surface = rgb(0xffffff), .surfaceRaised = rgb(0xf6f8fa), .surfaceHover = rgb(0xeef1f4), .border = rgb(0xd0d7de),
    .text = rgb(0x1f2328), .textMuted = rgb(0x59636e), .accent = rgb(0x0969da), .danger = rgb(0xcf222e),
    .onAccent = rgb(0xffffff),
    .github = rgb(0x1a7f37), .local = rgb(0x9a6700), .untracked = rgb(0x6e7781),
    .shadow = rgb(0x000000, 0.22f),
};

// Bubble
inline constexpr Color kBubbleFrom = rgb(0x262c36);  // background, top-left
inline constexpr Color kBubbleTo = rgb(0x010409);    // background, bottom-right
inline constexpr Color kBubbleBorder = rgb(0x3d444d);
inline constexpr Color kBranchFrom = rgb(0x58a6ff);  // branch line, bottom-left
inline constexpr Color kBranchTo = rgb(0x3fb950);    // branch line, top-right
inline constexpr float kBubbleSize = 56;
inline constexpr float kBubbleHoverScale = 1.06f;
inline constexpr float kBubblePressScale = 0.95f;

// Window and panel
inline constexpr float kWindowWidth = 360;
inline constexpr float kWindowHeight = 540;
inline constexpr float kWindowGap = 12;  // room around the panel for its shadow
inline constexpr float kPanelRadius = 18;
inline constexpr float kPanelPadding = 12;
inline constexpr float kHeaderHeight = 56;
inline constexpr float kControlHeight = 28;  // filter tabs and search box
inline constexpr float kControlGap = 6;
inline constexpr float kRadiusSmall = 6;
inline constexpr float kRadiusMedium = 9;

// Folder list
inline constexpr float kRowHeight = 26;
inline constexpr float kRootRowHeight = 34;
inline constexpr float kIndent = 16;
inline constexpr float kBadgeRadius = 10;

// Text (the font itself is the system's interface font: Segoe UI on Windows, the desktop's font on Linux)
inline constexpr float kTitleSize = 15;
inline constexpr float kBodySize = 13;
inline constexpr float kSmallSize = 12;
inline constexpr float kIconSize = 14;
inline constexpr float kIconSmallSize = 12;
inline constexpr float kIconTinySize = 10;

// Animation
inline constexpr int kOpenMs = 420;
inline constexpr int kCloseMs = 280;
inline constexpr float kCloseFadePortion = 0.4f;  // the panel fades out during the last 40% of closing
inline constexpr float kDragThreshold = 4;        // DIPs the mouse must move before a press counts as a drag

const Palette& palette();           // light or dark, from the theme mode
void setThemeMode(ThemeMode mode);  // System follows the Windows / desktop setting
void refreshPalette();              // call when the system's light/dark setting changes

// Each system answers this its own way: windows/SystemTheme.cpp, linux/SystemTheme.cpp.
bool systemUsesLightTheme();

Color statusColor(Status status);
bool hasBadge(Status status);

}  // namespace theme
