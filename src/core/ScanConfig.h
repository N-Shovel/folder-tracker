#pragma once
#include <string_view>

// What the scanner looks at. Edit these to scan deeper or skip more folders.
namespace scan_config {

// How many folder levels below each added folder get scanned.
inline constexpr int kMaxDepth = 6;

// Never shown or scanned (dependencies, build output, caches).
// Hidden folders (starting with ".") are skipped as well.
inline constexpr std::wstring_view kIgnoredFolders[] = {
    L"node_modules", L"dist", L"build", L"out", L"target", L"bin", L"obj", L"coverage",
    L"__pycache__", L"venv", L"env",
    // Unity / game engine caches
    L"Library", L"Temp", L"Logs",
};

}  // namespace scan_config
