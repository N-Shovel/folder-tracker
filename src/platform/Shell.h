#pragma once
#include <windows.h>

#include <optional>
#include <string>

// Small helpers that talk to Windows itself.
std::optional<std::wstring> pickFolder(HWND owner);  // the standard "Select Folder" dialog
void openUrl(const std::wstring& url);                // opens in the default browser
bool isDirectory(const std::wstring& path);

bool startsWithWindows();
void setStartWithWindows(bool enabled);
