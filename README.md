# Folder Tracker

### Hi, I'm Nimrod <img src="https://raw.githubusercontent.com/MartinHeinz/MartinHeinz/master/wave.gif" width="28" alt="waving hand">

A floating desktop bubble for Windows and Linux, written in C++ (plain Win32 + Direct2D on Windows, GTK 3 + Cairo on Linux). Click it and a small panel grows out of it, showing your project folders and whether each one is **on GitHub** (cloud), **git only** (drive), or has **no git** (warning). Uses about 6 MB of memory.

<p align="center"><img src="docs/screenshot.png" alt="Folder Tracker panel open, listing example projects with their git status" width="360"></p>

## Download

| Version | Released | What's new | |
|---|---|---|---|
| **1.2.0** (latest) | Oct 8, 2026 | Linux version | <a href="https://github.com/N-Shovel/folder-tracker/releases/download/v1.2.0/FolderTracker.exe"><img src="https://img.shields.io/badge/WINDOWS-v1.2.0-2ea043?style=for-the-badge" alt="Download v1.2.0 for Windows"></a> <a href="https://github.com/N-Shovel/folder-tracker/releases/download/v1.2.0/FolderTracker-linux-x86_64"><img src="https://img.shields.io/badge/LINUX-v1.2.0-2ea043?style=for-the-badge" alt="Download v1.2.0 for Linux"></a> |
| 1.1.0 | Oct 6, 2026 | New dark icon and bubble, ⚙ settings button, dark / light theme setting | <a href="https://github.com/N-Shovel/folder-tracker/releases/download/v1.1.0/FolderTracker.exe"><img src="https://img.shields.io/badge/WINDOWS-v1.1.0-555555?style=for-the-badge" alt="Download v1.1.0 for Windows"></a> |
| 1.0.0 | Oct 6, 2026 | First release | <a href="https://github.com/N-Shovel/folder-tracker/releases/download/v1.0.0/FolderTracker.exe"><img src="https://img.shields.io/badge/WINDOWS-v1.0.0-555555?style=for-the-badge" alt="Download v1.0.0 for Windows"></a> |

One file each, no install needed, and you don't need the source code. Keep the file somewhere it can stay (for example an `Apps` folder) rather than in Downloads: **Start with Windows** / **Start at login** starts it from wherever it is.

- **Windows** 10 and 11: `FolderTracker.exe`, about 1.4 MB.
- **Linux** (64-bit, Ubuntu 22.04 / Debian 12 / Fedora 36 or newer, any desktop): `FolderTracker-linux-x86_64`. It uses GTK 3, which every desktop Linux already has. On Wayland desktops it runs through XWayland, so it can float and stay where you put it.

## How to use

1. **Run it.** A round bubble appears near the bottom-right of your screen.
   - **Windows:** run `FolderTracker.exe`. Windows may warn on first launch because the app is not signed: click **More info → Run anyway**.
   - **Linux:** the file has to be made runnable once. Open a terminal, go to the folder you saved it in, and run:
     ```
     cd ~/Downloads
     chmod +x FolderTracker-linux-x86_64
     ./FolderTracker-linux-x86_64
     ```
     (Change `~/Downloads` if you saved it somewhere else.) The last line starts it.

     Without a terminal: right-click the file, choose **Properties**, and turn on the option that lets it run as a program (called something like **Allow executing file as program** or **Is executable**, depending on your desktop). Then double-click it. If double-clicking doesn't start it, use the terminal steps.

     The downloaded file isn't added to your app menu. Start it from its folder, or turn on **Start at login** (see Settings).
2. **Click the bubble** to open the panel. Click it again, click anywhere else, or press Esc to close it.
3. **Add a folder.** Click **Add a folder** (or the folder button in the panel header) and pick the folder that holds your projects, for example `Documents\Projects`. You can add more than one.
4. **Read the status** next to each folder:

   | Icon | Status | Meaning |
   |---|---|---|
   | ☁️ green | **GitHub** | Has git and is linked to GitHub, so it's backed up online. Click the arrow button to open it on GitHub. |
   | 🖴 amber | **Git only** | Has git, but isn't on GitHub yet. It exists only on this computer. |
   | ⚠️ gray | **No git** | Not using git at all. |

   Folders with a ▸ arrow have projects inside; click the row to expand it.
5. **Find a project** by typing anywhere while the panel is open; Esc clears the search. The **GitHub / Git only / No git** tabs show just that group, with a count for each.
6. **Keep it up to date.** Click ↻ in the header to rescan everything, or use the ↻ and 🗑 buttons on a folder you added to rescan or remove just that one (removing only takes it off the list; nothing is deleted).
7. **Move the bubble** by dragging it anywhere. The panel always opens toward the middle of the screen.

### Settings

Click ⚙ in the panel, or right-click the bubble or the tray icon near the clock (on Linux, click the tray icon):

- **Hide bubble**: keep only the tray icon. Click the tray icon to bring the bubble back (on Linux, choose **Show bubble** in its menu, or middle-click it).
- **Start with Windows** / **Start at login** (Linux): open Folder Tracker automatically when you sign in. It remembers where the file is, so if you move the file later, turn this off and on again.
- **Theme**: same as the system, dark or light.
- **Quit Folder Tracker**

Opening the app again while it's already running just opens the panel (and brings back a hidden bubble).

> **Linux tray icon:** KDE, Ubuntu, Xfce, Cinnamon and most other desktops show it. Plain GNOME needs the [AppIndicator extension](https://extensions.gnome.org/extension/615/appindicator-support/). Without a tray, run Folder Tracker again to bring back a hidden bubble.

### Remove it

1. Turn off **Start with Windows** / **Start at login**, then choose **Quit Folder Tracker**.
2. Delete the file.
3. To also remove your saved folder list, delete `%APPDATA%\Folder Tracker` on Windows, or `~/.config/folder-tracker` and `~/.local/share/folder-tracker` (where it keeps its icon) on Linux.

## Build

The same commands work on both systems:

```
cmake -S . -B build -G Ninja
cmake --build build
```

**Windows** needs a MinGW-w64 g++ (C++20), CMake and Ninja. The app is `build\Folder Tracker.exe`, a single file with nothing else needed. Quit the running app before rebuilding, or the build cannot replace the .exe.

**Linux** needs g++ 11 or newer, CMake, Ninja and the GTK 3 development files. On Ubuntu / Debian:

```
sudo apt install g++ cmake ninja-build pkg-config libgtk-3-dev
```

On Fedora: `sudo dnf install gcc-c++ cmake ninja-build gtk3-devel`. The app is `build/folder-tracker`. `sudo cmake --install build` also adds it to your app menu.

Settings (added folders, bubble position) are saved in `%APPDATA%\Folder Tracker\settings.ini` on Windows and `~/.config/folder-tracker/settings.ini` on Linux.

## Where things are

Windows and Linux each have their own folder, so a change to one never touches the other. Only `core/` and `ui/` are shared, and they contain no Windows or Linux code.

```
src/
  core/                  shared: scanning, no drawing here
    Scanner.cpp          walks folders and decides each folder's status
    ScanConfig.h         scan depth and ignored folders (node_modules, dist, ...)
    Git.cpp              finds .git and reads its remotes straight from .git/config
    Workspace.cpp        the added folders; runs scans on background threads
    FolderNode.h         the folder tree data
    Settings.h           what gets saved (each system saves it its own way)
    FileSystem.h         listing folders (each system does it its own way)
    Text.h               text helpers
  ui/                    shared: what the bubble and panel look like
    Theme.h              colors, sizes, fonts, animation timing (change the look here)
    Icons.h              which icons the panel uses
    PanelView.cpp        draws the panel: header, tabs, search, folder tree
    BubbleArt.cpp        draws the round bubble
    Layout.cpp           where each part of the panel goes
    Painter.h            simple drawing commands (rectangles, circles, text, icons)
  windows/               the Windows version only
    main.cpp             start-up, one-instance-only check, message loop
    BubbleWindow.cpp     the floating window: dragging, clicks, keyboard, animation
    D2DPainter.cpp       Painter.h done with Direct2D / DirectWrite
    LayeredSurface.cpp   the see-through window image
    Graphics.cpp         shared Direct2D / DirectWrite setup
    Settings.cpp         %APPDATA%\Folder Tracker\settings.ini
    Shell.cpp            folder picker, open links, start with Windows
    Tray.cpp             tray icon
    AppMenu.cpp          right-click menu
    FileSystem.cpp, Text.cpp, SystemTheme.cpp   folders, text, light/dark mode
  linux/                 the Linux version only
    main.cpp             start-up; GtkApplication keeps it to one copy
    BubbleWindow.cpp     the floating GTK window: dragging, clicks, keyboard, animation
    CairoPainter.cpp     Painter.h done with Cairo, Pango and the desktop's icon theme
    Settings.cpp         ~/.config/folder-tracker/settings.ini
    Shell.cpp            folder picker, open links, start at login
    Tray.cpp             tray icon (AppIndicator, loaded only if installed)
    AppMenu.cpp          settings menu
    AppIcon.cpp          draws the app icon from the bubble
    FileSystem.cpp, Text.cpp, SystemTheme.cpp   folders, text, light/dark mode
resources/
  icon.svg               app icon (shared)
  windows/               icon.ico, version info
  linux/                 .desktop file for the app menu
```

## Code signing policy

Release builds are made on GitHub Actions from this repository's source ([release.yml](.github/workflows/release.yml)). Free code signing provided by [SignPath.io](https://about.signpath.io), certificate by [SignPath Foundation](https://signpath.org).

- Committers and reviewers: [N-Shovel](https://github.com/N-Shovel)
- Approvers: [N-Shovel](https://github.com/N-Shovel)

Every signing request is approved by hand before release.

**Privacy:** Folder Tracker does not send any information to any networked system. It reads folders and `.git/config` files on your computer, and only opens a web page in your browser when you click a GitHub link. Settings stay in `%APPDATA%\Folder Tracker\settings.ini` (Windows) or `~/.config/folder-tracker/settings.ini` (Linux).

## License

<a href="LICENSE"><img src="https://img.shields.io/badge/LICENSE-MIT-8957e5?style=for-the-badge" alt="License: MIT"></a>

Free to use, change and share. See [LICENSE](LICENSE) for the full text.
