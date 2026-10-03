#pragma once
#include <windows.h>

#include <memory>
#include <string>
#include <vector>

#include "core/FolderNode.h"

class Settings;

// One folder the user added, plus its latest scan.
struct RootFolder {
    std::wstring path;
    std::unique_ptr<FolderNode> tree;
    bool scanning = false;
    std::wstring error;
    unsigned scanId = 0;  // ignores results from an older scan that finishes late
};

struct StatusCounts {
    int github = 0;
    int local = 0;
    int untracked = 0;
};

// Sent from a scan thread to the window as WM_SCAN_FINISHED.
struct ScanResult {
    std::wstring path;
    unsigned scanId = 0;
    std::unique_ptr<FolderNode> tree;
    std::wstring error;
};

// The added folders and their scans. Scans run on background threads.
class Workspace {
public:
    explicit Workspace(Settings& settings);

    void attach(HWND window);  // scan results are posted to this window
    void loadSaved();

    const std::vector<RootFolder>& roots() const { return roots_; }
    StatusCounts counts() const;

    void addFolder(const std::wstring& path);
    void removeFolder(size_t index);
    void rescan(size_t index);
    void rescanAll();
    void finishScan(std::unique_ptr<ScanResult> result);

private:
    void save() const;

    Settings& settings_;
    HWND window_ = nullptr;
    std::vector<RootFolder> roots_;
};
