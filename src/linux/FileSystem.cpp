#include "core/FileSystem.h"

#include <dirent.h>
#include <sys/stat.h>

#include "core/Text.h"

std::vector<std::wstring> listFolders(const std::wstring& folder) {
    std::vector<std::wstring> names;
    const std::string path = narrow(folder);
    DIR* dir = opendir(path.c_str());
    if (!dir) return names;

    while (const dirent* entry = readdir(dir)) {
        bool isFolder = entry->d_type == DT_DIR;  // DT_LNK for symlinks, which are skipped
        if (entry->d_type == DT_UNKNOWN) {        // some file systems don't fill d_type
            struct stat info;
            isFolder = lstat((path + "/" + entry->d_name).c_str(), &info) == 0 && S_ISDIR(info.st_mode);
        }
        if (isFolder) names.push_back(widen(entry->d_name));
    }
    closedir(dir);
    return names;
}

std::wstring joinPath(const std::wstring& folder, const std::wstring& name) {
    return folder.ends_with(L'/') ? folder + name : folder + L'/' + name;
}

bool isDirectory(const std::wstring& path) {
    struct stat info;
    return stat(narrow(path).c_str(), &info) == 0 && S_ISDIR(info.st_mode);
}

std::filesystem::path toPath(const std::wstring& path) { return std::filesystem::path(narrow(path)); }

std::filesystem::path pathFromUtf8(const std::string& text) { return std::filesystem::path(text); }
