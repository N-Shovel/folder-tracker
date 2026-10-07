cmake_minimum_required(VERSION 3.20)
project(FolderTracker VERSION 1.2.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

# Shared by both versions: scanning folders and git, and drawing the bubble and panel.
set(SHARED_SOURCES
  src/core/Git.cpp
  src/core/Scanner.cpp
  src/core/Text.cpp
  src/core/Workspace.cpp
  src/ui/BubbleArt.cpp
  src/ui/Layout.cpp
  src/ui/PanelView.cpp
  src/ui/Theme.cpp
)

if(WIN32)
  # Windows version: everything in src/windows and resources/windows.
  enable_language(RC)
  add_executable(FolderTracker WIN32
    ${SHARED_SOURCES}
    src/windows/main.cpp
    src/windows/AppMenu.cpp
    src/windows/BubbleWindow.cpp
    src/windows/D2DPainter.cpp
    src/windows/FileSystem.cpp
    src/windows/Graphics.cpp
    src/windows/LayeredSurface.cpp
    src/windows/Settings.cpp
    src/windows/Shell.cpp
    src/windows/SystemTheme.cpp
    src/windows/Text.cpp
    src/windows/Tray.cpp
    resources/windows/app.rc
  )

  set_target_properties(FolderTracker PROPERTIES OUTPUT_NAME "Folder Tracker")
  target_include_directories(FolderTracker PRIVATE src resources/windows)
  target_compile_definitions(FolderTracker PRIVATE UNICODE _UNICODE NOMINMAX WIN32_LEAN_AND_MEAN _WIN32_WINNT=0x0A00)
  target_link_libraries(FolderTracker PRIVATE d2d1 dwrite shcore shlwapi ole32 uuid)

  if(MINGW)
    # One self-contained .exe: no MinGW DLLs needed next to it.
    target_link_options(FolderTracker PRIVATE -municode -static -s)
  endif()
else()
  # Linux version: everything in src/linux and resources/linux.
  find_package(PkgConfig REQUIRED)
  pkg_check_modules(GTK3 REQUIRED IMPORTED_TARGET gtk+-3.0)
  find_package(Threads REQUIRED)

  add_executable(FolderTracker
    ${SHARED_SOURCES}
    src/linux/main.cpp
    src/linux/AppIcon.cpp
    src/linux/AppMenu.cpp
    src/linux/BubbleWindow.cpp
    src/linux/CairoPainter.cpp
    src/linux/FileSystem.cpp
    src/linux/Settings.cpp
    src/linux/Shell.cpp
    src/linux/SystemTheme.cpp
    src/linux/Text.cpp
    src/linux/Tray.cpp
  )

  set_target_properties(FolderTracker PROPERTIES OUTPUT_NAME "folder-tracker")
  target_include_directories(FolderTracker PRIVATE src)
  # The tray library is loaded at run time (dlopen), so GTK 3 is the only thing the app needs installed.
  target_link_libraries(FolderTracker PRIVATE PkgConfig::GTK3 Threads::Threads ${CMAKE_DL_LIBS})

  if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    # Carry the C++ library inside the program, so it also runs on distros with an older one.
    target_link_options(FolderTracker PRIVATE -static-libstdc++ -static-libgcc $<$<CONFIG:Release>:-s>)
  endif()

  include(GNUInstallDirs)
  install(TARGETS FolderTracker RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
  install(FILES resources/linux/folder-tracker.desktop DESTINATION ${CMAKE_INSTALL_DATADIR}/applications)
  install(FILES resources/icon.svg DESTINATION ${CMAKE_INSTALL_DATADIR}/icons/hicolor/scalable/apps
          RENAME folder-tracker.svg)
endif()
