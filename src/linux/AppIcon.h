#pragma once
#include <string>

// The app icon as a PNG file (~/.local/share/folder-tracker/folder-tracker.png), drawn from the
// bubble on first use. The tray, the window and the autostart entry all point to it.
inline constexpr char kAppIconName[] = "folder-tracker";

const std::string& appIconFolder();
const std::string& appIconPath();
