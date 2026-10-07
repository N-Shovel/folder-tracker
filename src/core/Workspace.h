#pragma once
#include <functional>
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

// Sent from a scan thread to the window, which passes it back to finishScan().
struct ScanResult {
    std::wstring path;
    unsigned scanId = 0;
    std::unique_ptr<FolderNode> tree;
    std::wstring error;
};

// Called on a scan thread. Hands the result to the UI thread; returns true if it took ownership.
using ScanPoster = std::function<bool(ScanResult* result)>;

// The added folders and their scans. Scans run on background threads.
class Workspace {
public:
    explicit Workspace(Settings& settings);

    void attach(ScanPoster post);  // how scan results get back to the window
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
    ScanPoster post_;
    std::vector<RootFolder> roots_;
};
