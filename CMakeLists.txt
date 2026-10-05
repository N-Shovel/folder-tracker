cmake_minimum_required(VERSION 3.20)
project(FolderTracker VERSION 1.1.0 LANGUAGES CXX RC)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

add_executable(FolderTracker WIN32
  src/main.cpp
  src/core/Git.cpp
  src/core/Scanner.cpp
  src/core/Text.cpp
  src/core/Workspace.cpp
  src/platform/AppMenu.cpp
  src/platform/Settings.cpp
  src/platform/Shell.cpp
  src/platform/Tray.cpp
  src/ui/BubbleArt.cpp
  src/ui/BubbleWindow.cpp
  src/ui/Graphics.cpp
  src/ui/LayeredSurface.cpp
  src/ui/Layout.cpp
  src/ui/Painter.cpp
  src/ui/PanelView.cpp
  src/ui/Theme.cpp
  resources/app.rc
)

set_target_properties(FolderTracker PROPERTIES OUTPUT_NAME "Folder Tracker")
target_include_directories(FolderTracker PRIVATE src resources)
target_compile_definitions(FolderTracker PRIVATE UNICODE _UNICODE NOMINMAX WIN32_LEAN_AND_MEAN _WIN32_WINNT=0x0A00)
target_link_libraries(FolderTracker PRIVATE d2d1 dwrite shcore shlwapi ole32 uuid)

if(MINGW)
  # One self-contained .exe: no MinGW DLLs needed next to it.
  target_link_options(FolderTracker PRIVATE -municode -static -s)
endif()
