#include <gtk/gtk.h>

#include <string>

#include "ui/Theme.h"

// GNOME (and desktops that follow it) store "prefer-dark" in color-scheme. Elsewhere, a dark
// GTK theme usually has "dark" in its name (Adwaita-dark, Breeze-Dark, Yaru-dark, ...).
bool theme::systemUsesLightTheme() {
    GSettingsSchemaSource* source = g_settings_schema_source_get_default();
    if (GSettingsSchema* schema = source ? g_settings_schema_source_lookup(source, "org.gnome.desktop.interface", TRUE)
                                         : nullptr) {
        const bool hasColorScheme = g_settings_schema_has_key(schema, "color-scheme");
        g_settings_schema_unref(schema);
        if (hasColorScheme) {
            GSettings* settings = g_settings_new("org.gnome.desktop.interface");
            gchar* scheme = g_settings_get_string(settings, "color-scheme");
            const std::string value = scheme;
            g_free(scheme);
            g_object_unref(settings);
            if (value == "prefer-dark") return false;
            if (value == "prefer-light") return true;
        }
    }

    GtkSettings* gtk = gtk_settings_get_default();
    if (!gtk) return true;
    gboolean preferDark = FALSE;
    gchar* themeName = nullptr;
    g_object_get(gtk, "gtk-application-prefer-dark-theme", &preferDark, "gtk-theme-name", &themeName, nullptr);
    gchar* lower = themeName ? g_ascii_strdown(themeName, -1) : nullptr;
    const bool darkName = lower && std::string(lower).find("dark") != std::string::npos;
    g_free(lower);
    g_free(themeName);
    return !preferDark && !darkName;
}
