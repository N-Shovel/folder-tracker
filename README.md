# Folder Tracker

### Hi, I'm Nimrod <img src="https://raw.githubusercontent.com/MartinHeinz/MartinHeinz/master/wave.gif" width="28" alt="waving hand">

A floating desktop bubble, written in C++ with plain Win32 + Direct2D. Click it and a small panel grows out of it, showing your project folders and whether each one is **on GitHub** (cloud), **git only** (drive), or has **no git** (warning). Uses about 6 MB of memory.

<p align="center"><img src="docs/screenshot.png" alt="Folder Tracker panel open, listing example projects with their git status" width="360"></p>

## Download

| Version | Released | What's new | |
|---|---|---|---|
| **1.1.0** (latest) | Oct 6, 2026 | New dark icon and bubble, ⚙ settings button, dark / light theme setting | <a href="https://github.com/N-Shovel/folder-tracker/releases/download/v1.1.0/FolderTracker.exe"><img src="https://img.shields.io/badge/DOWNLOAD-v1.1.0-2ea043?style=for-the-badge" alt="Download v1.1.0"></a> |
| 1.0.0 | Oct 6, 2026 | First release | <a href="https://github.com/N-Shovel/folder-tracker/releases/download/v1.0.0/FolderTracker.exe"><img src="https://img.shields.io/badge/DOWNLOAD-v1.0.0-555555?style=for-the-badge" alt="Download v1.0.0"></a> |

One file, about 1.4 MB. Just run it, no install needed.

> **Windows only for now** (Windows 10 and 11). A Linux version is still in the works.

## How to use

1. **Run `FolderTracker.exe`.** Windows may warn on first launch because the app is not signed: click **More info → Run anyway**. A round bubble appears near the bottom-right of your screen.
2. **Click the bubble** to open the panel. Click it again, click anywhere else, or press Esc to close it.
3. **Add a folder.** Click **Add a folder** (or the folder button in the panel header) and pick the folder that holds your projects, for example `Documents\Projects`. You can add more than one.
4. **Read the status** next to each folder:

   | Icon | Status | Meaning |
   |---|---|---|
   | ☁️ green | **GitHub** | Has git and is linked to GitHub, so it's backed up online. Click the arrow button to open it on GitHub. |
   | 🖴 amber | **Git only** | Has git, but isn't on GitHub yet. It exists only on this PC. |
   | ⚠️ gray | **No git** | Not using git at all. |

   Folders with a ▸ arrow have projects inside; click the row to expand it.
5. **Find a project** by typing anywhere while the panel is open; Esc clears the search. The **GitHub / Git only / No git** tabs show just that group, with a count for each.
6. **Keep it up to date.** Click ↻ in the header to rescan everything, or use the ↻ and 🗑 buttons on a folder you added to rescan or remove just that one (removing only takes it off the list; nothing is deleted).
7. **Move the bubble** by dragging it anywhere. The panel always opens toward the middle of the screen.

### Settings

Click ⚙ in the panel, or right-click the bubble or the tray icon near the clock:

- **Hide bubble**: keep only the tray icon. Click the tray icon to bring the bubble back.
- **Start with Windows**: open Folder Tracker automatically when you sign in.
- **Theme**: same as Windows, dark or light.
- **Quit Folder Tracker**

Opening the app again while it's already running just opens the panel.

## Build

Needs a MinGW-w64 g++ (C++20), CMake and Ninja.

```
cmake -S . -B build -G Ninja
cmake --build build
```

The app is `build\Folder Tracker.exe`, a single file with nothing else needed. Quit the running app before rebuilding, or the build cannot replace the .exe.

Settings (added folders, bubble position) are saved in `%APPDATA%\Folder Tracker\settings.ini`.

## Where things are

```
src/
  main.cpp               start-up, one-instance-only check, message loop
  Messages.h             custom window messages
  core/                  no drawing here
    Scanner.cpp          walks folders and decides each folder's status
    ScanConfig.h         scan depth and ignored folders (node_modules, dist, ...)
    Git.cpp              finds .git and reads its remotes straight from .git/config
    Workspace.cpp        the added folders; runs scans on background threads
    FolderNode.h         the folder tree data
  platform/              talking to Windows
    Settings.cpp         settings.ini
    Shell.cpp            folder picker, open links, start with Windows
    Tray.cpp             tray icon
    AppMenu.cpp          right-click menu
  ui/
    Theme.h              colors, sizes, fonts, animation timing (change the look here)
    Icons.h              which icon goes where
    BubbleWindow.cpp     the floating window: dragging, clicks, keyboard, animation
    PanelView.cpp        draws the panel: header, tabs, search, folder tree
    BubbleArt.cpp        draws the round bubble
    Layout.cpp           where each part of the panel goes
    Painter.cpp          simple drawing commands (rectangles, circles, text, icons)
    LayeredSurface.cpp   the see-through window image
    Graphics.cpp         shared Direct2D / DirectWrite setup
resources/               app icon and version info
```

## Code signing policy

Release builds are made on GitHub Actions from this repository's source ([release.yml](.github/workflows/release.yml)). Free code signing provided by [SignPath.io](https://about.signpath.io), certificate by [SignPath Foundation](https://signpath.org).

- Committers and reviewers: [N-Shovel](https://github.com/N-Shovel)
- Approvers: [N-Shovel](https://github.com/N-Shovel)

Every signing request is approved by hand before release.

**Privacy:** Folder Tracker does not send any information to any networked system. It reads folders and `.git/config` files on your PC, and only opens a web page in your browser when you click a GitHub link. Settings stay in `%APPDATA%\Folder Tracker\settings.ini`.

## License

<a href="LICENSE"><img src="https://img.shields.io/badge/LICENSE-MIT-8957e5?style=for-the-badge" alt="License: MIT"></a>

Free to use, change and share. See [LICENSE](LICENSE) for the full text.
