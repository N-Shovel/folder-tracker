#include "linux/Tray.h"

#include <dlfcn.h>

#include "linux/AppIcon.h"

namespace {

// From libappindicator's app-indicator.h.
constexpr int kCategoryApplicationStatus = 0;
constexpr int kStatusActive = 1;

void* loadIndicatorLibrary() {
    for (const char* name : {"libayatana-appindicator3.so.1", "libappindicator3.so.1"}) {
        if (void* library = dlopen(name, RTLD_LAZY | RTLD_LOCAL)) return library;
    }
    return nullptr;
}

}  // namespace

Tray::Tray() {
    void* library = loadIndicatorLibrary();
    if (!library) return;  // never unloaded: it registers GObject types that must stay around

    using NewFn = GObject* (*)(const gchar*, const gchar*, int);
    using SetStatusFn = void (*)(GObject*, int);
    using SetStringFn = void (*)(GObject*, const gchar*);
    const auto create = reinterpret_cast<NewFn>(dlsym(library, "app_indicator_new"));
    const auto setStatus = reinterpret_cast<SetStatusFn>(dlsym(library, "app_indicator_set_status"));
    const auto setThemePath = reinterpret_cast<SetStringFn>(dlsym(library, "app_indicator_set_icon_theme_path"));
    const auto setTitle = reinterpret_cast<SetStringFn>(dlsym(library, "app_indicator_set_title"));
    setMenu_ = reinterpret_cast<decltype(setMenu_)>(dlsym(library, "app_indicator_set_menu"));
    setMiddleClick_ = reinterpret_cast<decltype(setMiddleClick_)>(dlsym(library, "app_indicator_set_secondary_activate_target"));
    if (!create || !setStatus || !setThemePath || !setMenu_) return;

    // The icon is looked up by name in its folder, the same way Electron apps do it.
    appIconPath();
    indicator_ = create("folder-tracker", kAppIconName, kCategoryApplicationStatus);
    if (!indicator_) return;
    setThemePath(indicator_, appIconFolder().c_str());
    if (setTitle) setTitle(indicator_, "Folder Tracker");
    setStatus(indicator_, kStatusActive);
}

Tray::~Tray() {
    if (indicator_) g_object_unref(indicator_);
    if (menu_) {
        gtk_widget_destroy(menu_);
        g_object_unref(menu_);
    }
}

void Tray::setMenu(GtkWidget* menu, GtkWidget* middleClickItem) {
    if (!indicator_) {
        gtk_widget_destroy(menu);
        return;
    }
    g_object_ref_sink(menu);
    setMenu_(indicator_, GTK_MENU(menu));
    if (setMiddleClick_) setMiddleClick_(indicator_, middleClickItem);

    if (menu_) {
        gtk_widget_destroy(menu_);
        g_object_unref(menu_);
    }
    menu_ = menu;
}
