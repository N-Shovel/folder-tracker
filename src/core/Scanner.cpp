#include "core/Scanner.h"

#include <algorithm>
#include <optional>
#include <vector>

#include "core/FileSystem.h"
#include "core/Git.h"
#include "core/ScanConfig.h"
#include "core/Text.h"

namespace {

bool isIgnored(std::wstring_view name) {
    return name.starts_with(L'.') || std::ranges::find(scan_config::kIgnoredFolders, name) !=
                                         std::end(scan_config::kIgnoredFolders);
}

std::vector<std::wstring> listSubfolders(const std::wstring& folder) {
    std::vector<std::wstring> names = listFolders(folder);
    std::erase_if(names, [](const std::wstring& name) { return isIgnored(name); });  // also drops "." and ".."
    // Same order as the file manager ("Project 2" before "Project 10").
    std::ranges::sort(names, naturalLess);
    return names;
}

std::unique_ptr<FolderNode> readTree(const std::wstring& folder, int depth) {
    auto node = std::make_unique<FolderNode>();
    node->path = folder;
    node->name = folderName(folder);

    const GitInfo git = readGitInfo(folder);
    node->isRepo = git.isRepo;
    node->onGitHub = git.onGitHub;
    node->webUrl = git.webUrl;

    if (depth < scan_config::kMaxDepth) {
        for (const auto& name : listSubfolders(folder)) {
            node->children.push_back(readTree(joinPath(folder, name), depth + 1));
        }
    }
    return node;
}

// Sets hasReposBelow; returns true if this folder or anything below it is a repo.
bool markReposBelow(FolderNode& node) {
    bool any = false;
    for (auto& child : node.children) any |= markReposBelow(*child);
    node.hasReposBelow = any;
    return node.isRepo || any;
}

void assignStatus(FolderNode& node, std::optional<Status> parent) {
    if (node.isRepo) {
        node.status = node.onGitHub ? Status::GitHub : Status::Local;
    } else if (parent && *parent != Status::Container) {
        node.status = Status::Inside;
    } else if (!node.hasReposBelow) {
        node.status = Status::Untracked;
    } else {
        node.status = Status::Container;
    }
    for (auto& child : node.children) assignStatus(*child, node.status);
}

}  // namespace

std::unique_ptr<FolderNode> scanFolder(const std::wstring& root) {
    auto tree = readTree(root, 0);
    markReposBelow(*tree);
    assignStatus(*tree, std::nullopt);
    return tree;
}
