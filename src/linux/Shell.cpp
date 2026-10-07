#include "linux/Shell.h"

#include <glib/gstdio.h>
#include <unistd.h>

#include <climits>
#include <cstdlib>

#include "core/Text.h"
#include "linux/AppIcon.h"

namespace {

std::string autostartFile() {
    gchar* path = g_build_filename(g_get_user_config_dir(), "autostart", "folder-tracker.desktop", nullptr);
    std::string result = path;
    g_free(path);
    return result;
}

// The program to start at login. Inside an AppImage, that's the AppImage file, not the temporary copy it runs from.
std::string executablePath() {
    if (const char* appImage = std::getenv("APPIMAGE")) return appImage;
    char buffer[PATH_MAX] = {};
    const ssize_t length = readlink("/proc/self/exe", buffer, sizeof(buffer) - 1);
    return length > 0 ? std::string(buffer, length) : std::string("folder-tracker");
}

// Quotes a path for the Exec= line of a .desktop file.
std::string quoteForExec(const std::string& path) {
    std::string quoted = "\"";
    for (const char ch : path) {
        if (ch == '"' || ch == '`' || ch == '$' || ch == '\\') quoted += '\\';
        if (ch == '%') quoted += '%';  // a lone % starts a field code
        quoted += ch;
    }
    return quoted + "\"";
}

}  // namespace

std::optional<std::wstring> pickFolder(GtkWindow* owner) {
    GtkFileChooserNative* dialog = gtk_file_chooser_native_new(
        "Add a folder to Folder Tracker", owner, GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER, "_Add", "_Cancel");
    std::optional<std::wstring> result;
    if (gtk_native_dialog_run(GTK_NATIVE_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT) {
        if (gchar* path = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog))) {
            result = widen(path);
            g_free(path);
        }
    }
    g_object_unref(dialog);
    return result;
}

void openUrl(const std::wstring& url) {
    if (url.starts_with(L"https://") || url.starts_with(L"http://")) {
        g_app_info_launch_default_for_uri(narrow(url).c_str(), nullptr, nullptr);
    }
}

bool startsAtLogin() { return g_file_test(autostartFile().c_str(), G_FILE_TEST_EXISTS); }

void setStartAtLogin(bool enabled) {
    const std::string file = autostartFile();
    if (!enabled) {
        g_remove(file.c_str());
        return;
    }

    gchar* folder = g_path_get_dirname(file.c_str());
    g_mkdir_with_parents(folder, 0700);
    g_free(folder);

    GKeyFile* entry = g_key_file_new();
    const char* group = "Desktop Entry";
    g_key_file_set_string(entry, group, "Type", "Application");
    g_key_file_set_string(entry, group, "Name", "Folder Tracker");
    g_key_file_set_string(entry, group, "Comment", "Shows which project folders are on GitHub, git only, or not using git");
    g_key_file_set_string(entry, group, "Exec", quoteForExec(executablePath()).c_str());
    g_key_file_set_string(entry, group, "Icon", appIconPath().c_str());
    g_key_file_set_boolean(entry, group, "Terminal", FALSE);
    g_key_file_set_boolean(entry, group, "X-GNOME-Autostart-enabled", TRUE);
    g_key_file_save_to_file(entry, file.c_str(), nullptr);
    g_key_file_free(entry);
}
