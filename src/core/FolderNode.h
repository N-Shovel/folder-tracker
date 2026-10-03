#pragma once
#include <memory>
#include <string>
#include <vector>

// What a folder is, as far as Folder Tracker is concerned.
enum class Status {
    GitHub,     // a git repo connected to GitHub
    Local,      // a git repo with no GitHub remote (git init only)
    Untracked,  // a project folder with no git anywhere inside it
    Container,  // a plain folder that holds repos
    Inside,     // a subfolder of a repo or of an untracked project
};

struct FolderNode {
    std::wstring name;
    std::wstring path;
    Status status = Status::Container;
    bool isRepo = false;
    bool onGitHub = false;
    bool hasReposBelow = false;
    std::wstring webUrl;  // page of the repo's remote, empty if none
    std::vector<std::unique_ptr<FolderNode>> children;
};
