#pragma once
#include <gtk/gtk.h>

#include <optional>
#include <string>

// Small helpers that talk to the Linux desktop.
std::optional<std::wstring> pickFolder(GtkWindow* owner);  // the desktop's "Select Folder" dialog
void openUrl(const std::wstring& url);                      // opens in the default browser

// An entry in ~/.config/autostart, which every freedesktop desktop (GNOME, KDE, Xfce, ...) runs at login.
bool startsAtLogin();
void setStartAtLogin(bool enabled);
