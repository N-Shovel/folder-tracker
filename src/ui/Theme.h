#pragma once
#include <d2d1.h>

#include "core/FolderNode.h"

// Look and feel: colors, sizes, fonts and animation timing. Every other UI file reads from here.
// All sizes are in DIPs (pixels at 100% Windows scaling).
namespace theme {

constexpr D2D1_COLOR_F rgb(unsigned hex, float alpha = 1.0f) {
    return {((hex >> 16) & 0xFF) / 255.0f, ((hex >> 8) & 0xFF) / 255.0f, (hex & 0xFF) / 255.0f, alpha};
}

constexpr D2D1_COLOR_F withAlpha(D2D1_COLOR_F color, float alpha) {
    return {color.r, color.g, color.b, color.a * alpha};
}

struct Palette {
    D2D1_COLOR_F surface, surfaceRaised, surfaceHover, border;
    D2D1_COLOR_F text, textMuted, accent, danger, onAccent;
    D2D1_COLOR_F github, local, untracked;
    D2D1_COLOR_F shadow;
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
inline constexpr D2D1_COLOR_F kBubbleFrom = rgb(0x2f81f7);
inline constexpr D2D1_COLOR_F kBubbleTo = rgb(0x3fb950);
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

// Text
inline constexpr wchar_t kFontFamily[] = L"Segoe UI";
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

const Palette& palette();  // light or dark, following the Windows setting
void refreshPalette();     // call when Windows settings change

D2D1_COLOR_F statusColor(Status status);
bool hasBadge(Status status);

}  // namespace theme
