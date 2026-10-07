// The Linux side of core/Text.h. Here wchar_t holds one Unicode code point (UTF-32).
#include <cwctype>

#include "core/Text.h"

std::wstring widen(std::string_view utf8) {
    std::wstring result;
    result.reserve(utf8.size());
    for (size_t i = 0; i < utf8.size();) {
        const unsigned char lead = utf8[i];
        const int extra = lead < 0x80 ? 0 : (lead >> 5) == 0x6 ? 1 : (lead >> 4) == 0xE ? 2 : (lead >> 3) == 0x1E ? 3 : -1;
        if (extra < 0 || i + extra >= utf8.size()) {
            result += L'�';  // invalid or cut-off sequence
            ++i;
            continue;
        }
        char32_t code = extra == 0 ? lead : lead & (0x3F >> extra);
        bool valid = true;
        for (int k = 1; k <= extra; ++k) {
            const unsigned char next = utf8[i + k];
            if ((next & 0xC0) != 0x80) {
                valid = false;
                break;
            }
            code = (code << 6) | (next & 0x3F);
        }
        if (!valid) {
            result += L'�';
            ++i;
            continue;
        }
        result += wchar_t(code);
        i += extra + 1;
    }
    return result;
}

std::string narrow(std::wstring_view text) {
    std::string result;
    result.reserve(text.size());
    for (const wchar_t ch : text) {
        const auto code = char32_t(ch);
        if (code < 0x80) {
            result += char(code);
        } else if (code < 0x800) {
            result += char(0xC0 | (code >> 6));
            result += char(0x80 | (code & 0x3F));
        } else if (code < 0x10000) {
            result += char(0xE0 | (code >> 12));
            result += char(0x80 | ((code >> 6) & 0x3F));
            result += char(0x80 | (code & 0x3F));
        } else {
            result += char(0xF0 | (code >> 18));
            result += char(0x80 | ((code >> 12) & 0x3F));
            result += char(0x80 | ((code >> 6) & 0x3F));
            result += char(0x80 | (code & 0x3F));
        }
    }
    return result;
}

std::wstring toLower(std::wstring_view text) {
    std::wstring result(text);
    for (wchar_t& ch : result) ch = wchar_t(std::towlower(wint_t(ch)));
    return result;
}

std::wstring folderName(const std::wstring& path) {
    const size_t end = path.find_last_not_of(L'/');
    if (end == std::wstring::npos) return path;  // "/" itself
    const size_t slash = path.rfind(L'/', end);
    const size_t start = slash == std::wstring::npos ? 0 : slash + 1;
    return path.substr(start, end - start + 1);
}

bool naturalLess(const std::wstring& a, const std::wstring& b) {
    size_t i = 0, j = 0;
    while (i < a.size() && j < b.size()) {
        if (std::iswdigit(wint_t(a[i])) && std::iswdigit(wint_t(b[j]))) {
            // Compare whole numbers: skip leading zeros, then the longer number is bigger.
            while (i < a.size() && a[i] == L'0') ++i;
            while (j < b.size() && b[j] == L'0') ++j;
            size_t endA = i, endB = j;
            while (endA < a.size() && std::iswdigit(wint_t(a[endA]))) ++endA;
            while (endB < b.size() && std::iswdigit(wint_t(b[endB]))) ++endB;
            if (endA - i != endB - j) return endA - i < endB - j;
            const int order = a.compare(i, endA - i, b, j, endB - j);
            if (order != 0) return order < 0;
            i = endA;
            j = endB;
            continue;
        }
        const wint_t x = std::towlower(wint_t(a[i]));
        const wint_t y = std::towlower(wint_t(b[j]));
        if (x != y) return x < y;
        ++i;
        ++j;
    }
    if (i < a.size() || j < b.size()) return j < b.size();  // the shorter one comes first
    return a < b;                                          // equal ignoring case: keep a stable order
}

