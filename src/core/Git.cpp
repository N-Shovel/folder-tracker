#include "core/Git.h"

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <optional>
#include <sstream>
#include <vector>

#include "core/Text.h"

namespace fs = std::filesystem;

namespace {

std::string readFile(const fs::path& path) {
    std::ifstream file(path, std::ios::binary);
    return {std::istreambuf_iterator<char>(file), {}};
}

// ".git" is usually a folder. In worktrees and submodules it is a file saying "gitdir: <path>".
std::optional<fs::path> findGitDir(const fs::path& folder) {
    const fs::path dotGit = folder / L".git";
    std::error_code error;
    if (fs::is_directory(dotGit, error)) return dotGit;
    if (!fs::is_regular_file(dotGit, error)) return std::nullopt;

    const std::string text = trim(readFile(dotGit));
    const std::string prefix = "gitdir:";
    if (!text.starts_with(prefix)) return std::nullopt;

    fs::path target(widen(trim(text.substr(prefix.size()))));
    return target.is_relative() ? folder / target : target;
}

// Linked worktrees keep the shared config in the folder named by "commondir".
fs::path configFile(const fs::path& gitDir) {
    const std::string common = trim(readFile(gitDir / L"commondir"));
    if (common.empty()) return gitDir / L"config";

    fs::path commonDir(widen(common));
    return (commonDir.is_relative() ? gitDir / commonDir : commonDir) / L"config";
}

// Collects every "url = ..." line inside [remote "..."] sections.
std::vector<std::string> remoteUrls(const std::string& config) {
    std::vector<std::string> urls;
    std::istringstream lines(config);
    bool inRemote = false;

    for (std::string line; std::getline(lines, line);) {
        line = trim(line);
        if (line.empty() || line[0] == '#' || line[0] == ';') continue;
        if (line[0] == '[') {
            inRemote = line.starts_with("[remote ");
            continue;
        }
        const auto equals = line.find('=');
        if (inRemote && equals != std::string::npos && trim(line.substr(0, equals)) == "url") {
            urls.push_back(trim(line.substr(equals + 1)));
        }
    }
    return urls;
}

// git@github.com:user/repo.git  ->  https://github.com/user/repo
std::string toWebUrl(std::string url) {
    if (url.starts_with("git@")) {
        const auto colon = url.find(':');
        if (colon != std::string::npos) url = "https://" + url.substr(4, colon - 4) + "/" + url.substr(colon + 1);
    }
    if (url.starts_with("ssh://")) url.replace(0, 6, "https://");

    // Drop any embedded username or token: https://user:token@host -> https://host
    const auto scheme = url.find("://");
    if (scheme != std::string::npos) {
        const auto at = url.find('@', scheme + 3);
        const auto slash = url.find('/', scheme + 3);
        if (at != std::string::npos && (slash == std::string::npos || at < slash)) {
            url.erase(scheme + 3, at - scheme - 2);
        }
    }
    if (url.ends_with(".git")) url.resize(url.size() - 4);
    return url;
}

}  // namespace

GitInfo readGitInfo(const std::wstring& folder) {
    GitInfo info;
    const auto gitDir = findGitDir(folder);
    if (!gitDir) return info;
    info.isRepo = true;

    const auto urls = remoteUrls(readFile(configFile(*gitDir)));
    const auto github = std::ranges::find_if(urls, [](const std::string& url) {
        return url.find("github.com") != std::string::npos;
    });
    info.onGitHub = github != urls.end();

    const std::string remote = info.onGitHub ? *github : (urls.empty() ? "" : urls.front());
    if (!remote.empty()) info.webUrl = widen(toWebUrl(remote));
    return info;
}
