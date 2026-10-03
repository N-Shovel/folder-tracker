#include "core/Workspace.h"

#include <algorithm>
#include <thread>

#include "Messages.h"
#include "core/Scanner.h"
#include "platform/Settings.h"
#include "platform/Shell.h"

namespace {

void countTree(const FolderNode& node, StatusCounts& counts) {
    if (node.status == Status::GitHub) ++counts.github;
    if (node.status == Status::Local) ++counts.local;
    if (node.status == Status::Untracked) ++counts.untracked;
    for (const auto& child : node.children) countTree(*child, counts);
}

}  // namespace

Workspace::Workspace(Settings& settings) : settings_(settings) {}

void Workspace::attach(HWND window) { window_ = window; }

void Workspace::loadSaved() {
    for (const auto& path : settings_.folders()) roots_.push_back({.path = path});
    rescanAll();
}

StatusCounts Workspace::counts() const {
    StatusCounts counts;
    for (const auto& root : roots_) {
        if (root.tree) countTree(*root.tree, counts);
    }
    return counts;
}

void Workspace::addFolder(const std::wstring& path) {
    const bool known = std::ranges::any_of(roots_, [&](const RootFolder& root) { return root.path == path; });
    if (known) return;

    roots_.push_back({.path = path});
    save();
    rescan(roots_.size() - 1);
}

void Workspace::removeFolder(size_t index) {
    if (index >= roots_.size()) return;
    roots_.erase(roots_.begin() + index);
    save();
}

void Workspace::rescan(size_t index) {
    if (index >= roots_.size()) return;
    RootFolder& root = roots_[index];
    root.scanning = true;
    root.error.clear();

    std::thread([window = window_, path = root.path, scanId = ++root.scanId] {
        auto result = std::make_unique<ScanResult>();
        result->path = path;
        result->scanId = scanId;
        if (isDirectory(path)) result->tree = scanFolder(path);
        else result->error = L"Folder not found";

        if (PostMessageW(window, WM_SCAN_FINISHED, 0, reinterpret_cast<LPARAM>(result.get()))) {
            result.release();  // the window owns it now
        }
    }).detach();
}

void Workspace::rescanAll() {
    for (size_t i = 0; i < roots_.size(); ++i) rescan(i);
}

void Workspace::finishScan(std::unique_ptr<ScanResult> result) {
    const auto root = std::ranges::find_if(roots_, [&](const RootFolder& r) { return r.path == result->path; });
    if (root == roots_.end() || root->scanId != result->scanId) return;  // removed, or a newer scan is running

    root->scanning = false;
    root->error = result->error;
    if (result->tree) root->tree = std::move(result->tree);
}

void Workspace::save() const {
    std::vector<std::wstring> paths;
    for (const auto& root : roots_) paths.push_back(root.path);
    settings_.setFolders(paths);
}
