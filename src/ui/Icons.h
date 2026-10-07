#pragma once
#include "core/FolderNode.h"

// Every icon the panel uses. Each Painter backend decides how to draw them:
// Windows uses its built-in icon font (Segoe Fluent Icons / Segoe MDL2 Assets),
// Linux uses the desktop's symbolic icon theme (Adwaita and friends).
enum class Icon {
    Folder,
    FolderOpen,
    ChevronRight,
    ChevronDown,
    Refresh,
    Delete,
    OpenLink,
    AddFolder,
    Search,
    Settings,
    GitHub,     // cloud: backed up online
    Local,      // hard drive: only on this PC
    Untracked,  // warning: no version control at all
};

namespace icons {

constexpr Icon forStatus(Status status) {
    switch (status) {
        case Status::GitHub: return Icon::GitHub;
        case Status::Local: return Icon::Local;
        default: return Icon::Untracked;
    }
}

}  // namespace icons
