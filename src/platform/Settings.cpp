#include "platform/Settings.h"

#include <shlobj.h>

namespace {

std::wstring settingsFolder() {
    PWSTR appData = nullptr;
    SHGetKnownFolderPath(FOLDERID_RoamingAppData, 0, nullptr, &appData);
    std::wstring folder = std::wstring(appData) + L"\\Folder Tracker";
    CoTaskMemFree(appData);
    return folder;
}

}  // namespace

Settings::Settings() {
    const std::wstring folder = settingsFolder();
    CreateDirectoryW(folder.c_str(), nullptr);
    file_ = folder + L"\\settings.ini";

    // Windows only stores non-English text in an .ini file that starts as UTF-16.
    HANDLE file = CreateFileW(file_.c_str(), GENERIC_WRITE, 0, nullptr, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (file != INVALID_HANDLE_VALUE) {
        const WORD byteOrderMark = 0xFEFF;
        DWORD written = 0;
        WriteFile(file, &byteOrderMark, sizeof(byteOrderMark), &written, nullptr);
        CloseHandle(file);
    }
}

std::vector<std::wstring> Settings::folders() const {
    std::vector<std::wstring> folders;
    const int count = GetPrivateProfileIntW(L"Folders", L"Count", 0, file_.c_str());
    for (int i = 0; i < count; ++i) {
        std::wstring folder = read(L"Folders", std::to_wstring(i).c_str());
        if (!folder.empty()) folders.push_back(std::move(folder));
    }
    return folders;
}

void Settings::setFolders(const std::vector<std::wstring>& folders) {
    WritePrivateProfileStringW(L"Folders", nullptr, nullptr, file_.c_str());  // clear the section
    write(L"Folders", L"Count", std::to_wstring(folders.size()));
    for (size_t i = 0; i < folders.size(); ++i) write(L"Folders", std::to_wstring(i).c_str(), folders[i]);
}

std::optional<POINT> Settings::bubblePosition() const {
    const std::wstring x = read(L"Bubble", L"X");
    const std::wstring y = read(L"Bubble", L"Y");
    if (x.empty() || y.empty()) return std::nullopt;
    return POINT{std::stol(x), std::stol(y)};
}

void Settings::setBubblePosition(POINT center) {
    write(L"Bubble", L"X", std::to_wstring(center.x));
    write(L"Bubble", L"Y", std::to_wstring(center.y));
}

std::wstring Settings::read(const wchar_t* section, const wchar_t* key) const {
    wchar_t buffer[2048] = {};
    GetPrivateProfileStringW(section, key, L"", buffer, DWORD(std::size(buffer)), file_.c_str());
    return buffer;
}

void Settings::write(const wchar_t* section, const wchar_t* key, const std::wstring& value) {
    WritePrivateProfileStringW(section, key, value.c_str(), file_.c_str());
}
