// Folder Tracker for Linux: a floating bubble that shows which project folders are on GitHub,
// git only, or not using git at all.
#include <gtk/gtk.h>

#include <memory>

#include "core/Workspace.h"
#include "linux/BubbleWindow.h"
#include "core/Settings.h"

namespace {

// Only one Folder Tracker at a time: GtkApplication passes a second launch to the running one,
// which opens its panel.
constexpr char kAppId[] = "io.github.NShovel.FolderTracker";

struct App {
    Settings settings;
    Workspace workspace{settings};
    std::unique_ptr<BubbleWindow> window;
};

void onActivate(GtkApplication* gtkApp, gpointer data) {
    auto* app = static_cast<App*>(data);
    if (app->window) {
        app->window->showPanel();
        return;
    }
    app->window = std::make_unique<BubbleWindow>(app->settings, app->workspace);
    if (!app->window->create(gtkApp)) {
        g_application_quit(G_APPLICATION(gtkApp));
        return;
    }
    app->workspace.loadSaved();
}

}  // namespace

int main(int argc, char** argv) {
    // The bubble places itself on screen and floats above other windows, which Wayland doesn't let
    // apps do. On Wayland desktops it runs through XWayland instead, which every major one includes.
    gdk_set_allowed_backends("x11,*");

    App app;
    GtkApplication* gtkApp = gtk_application_new(kAppId, GApplicationFlags(0));
    g_signal_connect(gtkApp, "activate", G_CALLBACK(onActivate), &app);
    const int status = g_application_run(G_APPLICATION(gtkApp), argc, argv);
    app.window.reset();
    g_object_unref(gtkApp);
    return status;
}
