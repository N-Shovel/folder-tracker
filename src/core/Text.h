#pragma once
#include <string>
#include <string_view>

// Text helpers. trim() is shared (core/Text.cpp); the rest work differently on each system
// (windows/Text.cpp, linux/Text.cpp).

std::wstring widen(std::string_view utf8);
std::string narrow(std::wstring_view text);  // back to UTF-8
std::wstring toLower(std::wstring_view text);
std::string trim(std::string_view text);

// The last part of a path: "C:\Projects\app" or "/home/me/app" -> "app".
std::wstring folderName(const std::wstring& path);

// Compares names the way file managers sort them: "Project 2" before "Project 10".
bool naturalLess(const std::wstring& a, const std::wstring& b);
