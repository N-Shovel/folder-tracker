// The Windows side of core/Text.h.
#include <windows.h>
#include <shlwapi.h>

#include "core/Text.h"

std::wstring widen(std::string_view utf8) {
    if (utf8.empty()) return {};
    const int length = MultiByteToWideChar(CP_UTF8, 0, utf8.data(), int(utf8.size()), nullptr, 0);
    std::wstring result(length, L'\0');
    MultiByteToWideChar(CP_UTF8, 0, utf8.data(), int(utf8.size()), result.data(), length);
    return result;
}

std::string narrow(std::wstring_view text) {
    if (text.empty()) return {};
    const int length = WideCharToMultiByte(CP_UTF8, 0, text.data(), int(text.size()), nullptr, 0, nullptr, nullptr);
    std::string result(length, '\0');
    WideCharToMultiByte(CP_UTF8, 0, text.data(), int(text.size()), result.data(), length, nullptr, nullptr);
    return result;
}

std::wstring toLower(std::wstring_view text) {
    std::wstring result(text);
    CharLowerBuffW(result.data(), DWORD(result.size()));
    return result;
}

std::wstring folderName(const std::wstring& path) { return PathFindFileNameW(path.c_str()); }

bool naturalLess(const std::wstring& a, const std::wstring& b) { return StrCmpLogicalW(a.c_str(), b.c_str()) < 0; }

