#pragma once
#include <optional>
#include <string>
#include <vector>

enum class ThemeMode { System, Dark, Light };

struct ScreenPoint {
    int x = 0;
    int y = 0;
};

// Saved between launches in settings.ini:
//   Windows  %APPDATA%\Folder Tracker\settings.ini      (windows/Settings.cpp)
//   Linux    ~/.config/folder-tracker/settings.ini      (linux/Settings.cpp)
class Settings {
public:
    Settings();

    std::vector<std::wstring> folders() const;
    void setFolders(const std::vector<std::wstring>& folders);

    std::optional<ScreenPoint> bubblePosition() const;  // screen position of the bubble's center
    void setBubblePosition(ScreenPoint center);

    ThemeMode themeMode() const;
    void setThemeMode(ThemeMode mode);

private:
    std::wstring read(const wchar_t* section, const wchar_t* key) const;
    void write(const wchar_t* section, const wchar_t* key, const std::wstring& value);

    std::wstring file_;
};
