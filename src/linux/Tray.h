#pragma once
#include <gtk/gtk.h>

// The icon in the panel's tray area (StatusNotifier / AppIndicator, the kind KDE, Ubuntu, Xfce and
// Cinnamon show; plain GNOME needs the "AppIndicator" extension). The library is loaded when the app
// starts rather than linked, so Folder Tracker still runs without it, just with no tray icon.
class Tray {
public:
    Tray();
    ~Tray();

    Tray(const Tray&) = delete;
    Tray& operator=(const Tray&) = delete;

    bool available() const { return indicator_ != nullptr; }

    // Replaces the menu shown when the icon is clicked. `middleClickItem` is activated by a middle-click.
    void setMenu(GtkWidget* menu, GtkWidget* middleClickItem);

private:
    GObject* indicator_ = nullptr;
    GtkWidget* menu_ = nullptr;
    void (*setMenu_)(GObject*, GtkMenu*) = nullptr;
    void (*setMiddleClick_)(GObject*, GtkWidget*) = nullptr;
};
