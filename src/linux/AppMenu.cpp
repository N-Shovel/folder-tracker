#include "linux/AppMenu.h"

#include <memory>

namespace {

using Handler = std::function<void(MenuCommand)>;

struct ItemAction {
    std::shared_ptr<Handler> handler;
    MenuCommand command;
};

void onActivate(GtkMenuItem*, gpointer data) {
    const auto* action = static_cast<ItemAction*>(data);
    (*action->handler)(action->command);
}

// Connected after the item's checked state is set, so building the menu doesn't run any commands.
GtkWidget* addItem(GtkWidget* menu, GtkWidget* item, const std::shared_ptr<Handler>& handler, MenuCommand command) {
    g_signal_connect_data(item, "activate", G_CALLBACK(onActivate), new ItemAction{handler, command},
                          [](gpointer data, GClosure*) { delete static_cast<ItemAction*>(data); }, GConnectFlags(0));
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), item);
    return item;
}

}  // namespace

GtkWidget* buildAppMenu(const MenuState& state, std::function<void(MenuCommand)> onCommand,
                        GtkWidget** toggleBubbleItem) {
    const auto handler = std::make_shared<Handler>(std::move(onCommand));

    GtkWidget* themeMenu = gtk_menu_new();
    GSList* group = nullptr;
    const struct {
        const char* label;
        ThemeMode mode;
        MenuCommand command;
    } themes[] = {
        {"Same as system", ThemeMode::System, MenuCommand::ThemeSystem},
        {"Dark", ThemeMode::Dark, MenuCommand::ThemeDark},
        {"Light", ThemeMode::Light, MenuCommand::ThemeLight},
    };
    for (const auto& theme : themes) {
        GtkWidget* item = gtk_radio_menu_item_new_with_label(group, theme.label);
        group = gtk_radio_menu_item_get_group(GTK_RADIO_MENU_ITEM(item));
        gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(item), state.theme == theme.mode);
        addItem(themeMenu, item, handler, theme.command);
    }

    GtkWidget* menu = gtk_menu_new();
    if (state.forTray) {
        addItem(menu, gtk_menu_item_new_with_label("Open Folder Tracker"), handler, MenuCommand::ShowPanel);
    }
    GtkWidget* toggle = addItem(menu, gtk_menu_item_new_with_label(state.bubbleVisible ? "Hide bubble" : "Show bubble"),
                                handler, MenuCommand::ToggleBubble);
    if (toggleBubbleItem) *toggleBubbleItem = toggle;

    GtkWidget* login = gtk_check_menu_item_new_with_label("Start at login");
    gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(login), state.startsAtLogin);
    addItem(menu, login, handler, MenuCommand::StartAtLogin);

    GtkWidget* themeItem = gtk_menu_item_new_with_label("Theme");
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(themeItem), themeMenu);
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), themeItem);

    gtk_menu_shell_append(GTK_MENU_SHELL(menu), gtk_separator_menu_item_new());
    addItem(menu, gtk_menu_item_new_with_label("Quit Folder Tracker"), handler, MenuCommand::Quit);

    gtk_widget_show_all(menu);
    return menu;
}
