#pragma once
#include <string>
#include <string_view>

std::wstring widen(std::string_view utf8);
std::wstring toLower(std::wstring_view text);
std::string trim(std::string_view text);
