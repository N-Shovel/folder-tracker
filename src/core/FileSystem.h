#pragma once
#include <filesystem>
#include <string>
#include <vector>

// Folder access that works differently on each system: windows/FileSystem.cpp, linux/FileSystem.cpp.

// Names of the real subfolders of `folder`, in no particular order. Symlinks (and Windows junctions)
// are skipped, so a link back up the tree cannot make a scan loop forever.
std::vector<std::wstring> listFolders(const std::wstring& folder);

std::wstring joinPath(const std::wstring& folder, const std::wstring& name);
bool isDirectory(const std::wstring& path);

// Paths are UTF-16 on Windows and UTF-8 bytes on Linux.
std::filesystem::path toPath(const std::wstring& path);
std::filesystem::path pathFromUtf8(const std::string& text);  // a path written in a git file
