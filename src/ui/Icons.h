#pragma once
#include "core/FolderNode.h"

// Icons are characters from Windows' built-in icon font (Segoe Fluent Icons / Segoe MDL2 Assets).
// Browse them with the "Character Map" app, or at learn.microsoft.com (search "Segoe Fluent Icons").
namespace icons {

inline constexpr wchar_t kFolder = 0xE8B7;
inline constexpr wchar_t kFolderOpen = 0xE838;
inline constexpr wchar_t kChevronRight = 0xE76C;
inline constexpr wchar_t kChevronDown = 0xE70D;
inline constexpr wchar_t kRefresh = 0xE72C;
inline constexpr wchar_t kDelete = 0xE74D;
inline constexpr wchar_t kOpenLink = 0xE8A7;
inline constexpr wchar_t kAddFolder = 0xE8F4;
inline constexpr wchar_t kSearch = 0xE721;

inline constexpr wchar_t kGitHub = 0xE753;     // cloud: backed up online
inline constexpr wchar_t kLocal = 0xEDA2;      // hard drive: only on this PC
inline constexpr wchar_t kUntracked = 0xE7BA;  // warning: no version control at all

constexpr wchar_t forStatus(Status status) {
    switch (status) {
        case Status::GitHub: return kGitHub;
        case Status::Local: return kLocal;
        default: return kUntracked;
    }
}

}  // namespace icons
