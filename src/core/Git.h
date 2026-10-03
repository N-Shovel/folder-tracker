#pragma once
#include <string>

struct GitInfo {
    bool isRepo = false;
    bool onGitHub = false;
    std::wstring webUrl;  // e.g. https://github.com/user/repo
};

// Reads .git/config directly (no git program needed) to find the repo's remotes.
GitInfo readGitInfo(const std::wstring& folder);
