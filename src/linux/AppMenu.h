#pragma once
#include <gtk/gtk.h>

#include <functional>

#include "core/Settings.h"

enum class MenuCommand { None, ShowPanel, ToggleBubble, StartAtLogin, ThemeSystem, ThemeDark, ThemeLight, Quit };

struct MenuState {
    bool bubbleVisible = true;
    bool startsAtLogin = false;
    ThemeMode theme = ThemeMode::System;
    bool forTray = false;  // adds "Open Folder Tracker", since clicking a tray icon only opens its menu
};

// The settings menu (right-click on the bubble, the ⚙ button, or the tray icon).
// `onCommand` runs when an item is chosen. Returns a new GtkMenu; `toggleBubbleItem` gets the
// "Hide bubble" item, which a middle-click on the tray icon activates.
GtkWidget* buildAppMenu(const MenuState& state, std::function<void(MenuCommand)> onCommand,
                        GtkWidget** toggleBubbleItem = nullptr);
