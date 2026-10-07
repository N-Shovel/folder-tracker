#include "core/FileSystem.h"

#include <windows.h>

#include "core/Text.h"

std::vector<std::wstring> listFolders(const std::wstring& folder) {
    std::vector<std::wstring> names;
    WIN32_FIND_DATAW entry;
    HANDLE find = FindFirstFileExW(joinPath(folder, L"*").c_str(), FindExInfoBasic, &entry,
                                   FindExSearchLimitToDirectories, nullptr, FIND_FIRST_EX_LARGE_FETCH);
    if (find == INVALID_HANDLE_VALUE) return names;

    do {
        const bool isFolder = entry.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY;
        const bool isLink = entry.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT;  // symlinks, junctions
        if (isFolder && !isLink) names.emplace_back(entry.cFileName);
    } while (FindNextFileW(find, &entry));
    FindClose(find);
    return names;
}

std::wstring joinPath(const std::wstring& folder, const std::wstring& name) {
    return folder.ends_with(L'\\') ? folder + name : folder + L'\\' + name;
}

bool isDirectory(const std::wstring& path) {
    const DWORD attributes = GetFileAttributesW(path.c_str());
    return attributes != INVALID_FILE_ATTRIBUTES && (attributes & FILE_ATTRIBUTE_DIRECTORY);
}

std::filesystem::path toPath(const std::wstring& path) { return std::filesystem::path(path); }

std::filesystem::path pathFromUtf8(const std::string& text) { return std::filesystem::path(widen(text)); }
