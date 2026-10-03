#pragma once
#include <memory>
#include <string>

#include "core/FolderNode.h"

// Builds the folder tree below `root` and gives every folder a Status.
// Slow for big folders: call it from a background thread.
std::unique_ptr<FolderNode> scanFolder(const std::wstring& root);
