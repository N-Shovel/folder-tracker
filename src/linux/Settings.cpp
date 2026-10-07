// The Linux side of core/Settings.h: ~/.config/folder-tracker/settings.ini, read and written with GKeyFile.
#include <glib.h>
#include <glib/gstdio.h>

#include "core/Text.h"
#include "core/Settings.h"

namespace {

// Loads the file each time, so settings written by another copy of the app are never overwritten with old values.
class KeyFile {
public:
    explicit KeyFile(const std::wstring& path) : path_(narrow(path)), file_(g_key_file_new()) {
        g_key_file_load_from_file(file_, path_.c_str(), G_KEY_FILE_KEEP_COMMENTS, nullptr);
    }
    ~KeyFile() { g_key_file_free(file_); }

    KeyFile(const KeyFile&) = delete;
    KeyFile& operator=(const KeyFile&) = delete;

    GKeyFile* get() const { return file_; }
    void save() const { g_key_file_save_to_file(file_, path_.c_str(), nullptr); }

private:
    std::string path_;
    GKeyFile* file_;
};

}  // namespace

Settings::Settings() {
    gchar* folder = g_build_filename(g_get_user_config_dir(), "folder-tracker", nullptr);
    g_mkdir_with_parents(folder, 0700);
    gchar* file = g_build_filename(folder, "settings.ini", nullptr);
    file_ = widen(file);
    g_free(file);
    g_free(folder);
}

std::vector<std::wstring> Settings::folders() const {
    std::vector<std::wstring> folders;
    const std::wstring countText = read(L"Folders", L"Count");
    const int count = countText.empty() ? 0 : std::stoi(countText);
    for (int i = 0; i < count; ++i) {
        std::wstring folder = read(L"Folders", std::to_wstring(i).c_str());
        if (!folder.empty()) folders.push_back(std::move(folder));
    }
    return folders;
}

void Settings::setFolders(const std::vector<std::wstring>& folders) {
    {
        KeyFile file(file_);
        g_key_file_remove_group(file.get(), "Folders", nullptr);  // clear the section
        file.save();
    }
    write(L"Folders", L"Count", std::to_wstring(folders.size()));
    for (size_t i = 0; i < folders.size(); ++i) write(L"Folders", std::to_wstring(i).c_str(), folders[i]);
}

std::optional<ScreenPoint> Settings::bubblePosition() const {
    const std::wstring x = read(L"Bubble", L"X");
    const std::wstring y = read(L"Bubble", L"Y");
    if (x.empty() || y.empty()) return std::nullopt;
    return ScreenPoint{std::stoi(x), std::stoi(y)};
}

void Settings::setBubblePosition(ScreenPoint center) {
    write(L"Bubble", L"X", std::to_wstring(center.x));
    write(L"Bubble", L"Y", std::to_wstring(center.y));
}

ThemeMode Settings::themeMode() const {
    const std::wstring value = read(L"Appearance", L"Theme");
    if (value == L"dark") return ThemeMode::Dark;
    if (value == L"light") return ThemeMode::Light;
    return ThemeMode::System;
}

void Settings::setThemeMode(ThemeMode mode) {
    write(L"Appearance", L"Theme", mode == ThemeMode::Dark ? L"dark" : mode == ThemeMode::Light ? L"light" : L"system");
}

std::wstring Settings::read(const wchar_t* section, const wchar_t* key) const {
    KeyFile file(file_);
    gchar* value = g_key_file_get_string(file.get(), narrow(section).c_str(), narrow(key).c_str(), nullptr);
    if (!value) return {};
    std::wstring result = widen(value);
    g_free(value);
    return result;
}

void Settings::write(const wchar_t* section, const wchar_t* key, const std::wstring& value) {
    KeyFile file(file_);
    g_key_file_set_string(file.get(), narrow(section).c_str(), narrow(key).c_str(), narrow(value).c_str());
    file.save();
}
