#pragma once
#include <windows.h>

#include <optional>
#include <string>
#include <vector>

enum class ThemeMode { System, Dark, Light };

// Saved between launches in %APPDATA%\Folder Tracker\settings.ini
class Settings {
public:
    Settings();

    std::vector<std::wstring> folders() const;
    void setFolders(const std::vector<std::wstring>& folders);

    std::optional<POINT> bubblePosition() const;  // screen position of the bubble's center
    void setBubblePosition(POINT center);

    ThemeMode themeMode() const;
    void setThemeMode(ThemeMode mode);

private:
    std::wstring read(const wchar_t* section, const wchar_t* key) const;
    void write(const wchar_t* section, const wchar_t* key, const std::wstring& value);

    std::wstring file_;
};
