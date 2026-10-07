#pragma once
#include <gtk/gtk.h>

#include <memory>
#include <optional>

#include "linux/AppMenu.h"
#include "core/Settings.h"
#include "ui/Animation.h"
#include "ui/Layout.h"
#include "ui/PanelView.h"

class Tray;
class Workspace;
struct ScanResult;

// The floating window: the bubble plus the panel that grows out of it (the Linux twin of
// ui/BubbleWindow.cpp). It is always the size of the open panel; while closed, only the bubble
// is drawn, and the window's input shape lets clicks on the transparent rest fall through.
class BubbleWindow {
public:
    BubbleWindow(Settings& settings, Workspace& workspace);
    ~BubbleWindow();

    BubbleWindow(const BubbleWindow&) = delete;
    BubbleWindow& operator=(const BubbleWindow&) = delete;

    bool create(GtkApplication* app);
    void showPanel();  // when Folder Tracker is opened a second time

private:
    // Position (screen coordinates; GTK already applies the desktop's scaling, so these match DIPs)
    ScreenPoint bubbleCenterOnScreen() const;
    void placeBubble(ScreenPoint center);
    void saveBubblePosition();
    void moveWindow();

    // Panel
    void setOpen(bool open);
    void toggleVisible();
    void render();
    void updateInputShape();

    // Input
    bool isOverBubble(PointF point) const;
    void onLeftDown(PointF point, double rootX, double rootY);
    void onMouseMove(PointF point, double rootX, double rootY);
    void onLeftUp();
    bool onKeyPress(const GdkEventKey& event);
    void onScroll(const GdkEventScroll& event);
    void setHovered(Hit hit);
    void updateCursor();
    void runAction(Hit hit);
    void addFolder();

    // Menus
    MenuState menuState(bool forTray) const;
    void showMenu();
    void runCommand(MenuCommand command);
    void refreshTray();
    void onSystemThemeChanged();
    void quit();

    // GTK callbacks
    static gboolean onDraw(GtkWidget*, cairo_t* cr, gpointer self);
    static gboolean onButtonPress(GtkWidget*, GdkEventButton* event, gpointer self);
    static gboolean onButtonRelease(GtkWidget*, GdkEventButton* event, gpointer self);
    static gboolean onMotion(GtkWidget*, GdkEventMotion* event, gpointer self);
    static gboolean onLeave(GtkWidget*, GdkEventCrossing* event, gpointer self);
    static gboolean onScrollEvent(GtkWidget*, GdkEventScroll* event, gpointer self);
    static gboolean onKeyPressEvent(GtkWidget*, GdkEventKey* event, gpointer self);
    static gboolean onFocusOut(GtkWidget*, GdkEventFocus*, gpointer self);
    static gboolean onDelete(GtkWidget*, GdkEvent*, gpointer self);
    static void onMenuClosed(GtkMenuShell* menu, gpointer self);
    static gboolean onAnimationFrame(gpointer self);
    static gboolean onRefreshTray(gpointer self);
    static gboolean onScanFinished(gpointer pending);

    Settings& settings_;
    Workspace& workspace_;
    GtkApplication* app_ = nullptr;
    GtkWidget* window_ = nullptr;
    bool composited_ = false;  // false: no transparency, so the window's shape is cut to the drawing instead
    std::unique_ptr<Tray> tray_;
    guint trayRefresh_ = 0;
    GSettings* interfaceSettings_ = nullptr;  // GNOME's light/dark setting, when it exists

    cairo_surface_t* surface_ = nullptr;  // the window's picture, drawn in render()
    int scale_ = 0;
    PanelView panel_;
    PanelLayout layout_;
    Corner corner_;
    ScreenPoint position_;  // window's top-left corner on screen

    Animation reveal_;
    guint animationTimer_ = 0;
    bool open_ = false;
    bool ignoreDeactivate_ = false;  // while a dialog or menu is open
    Hit hovered_;

    struct Drag {
        double cursorX;
        double cursorY;
        ScreenPoint windowStart;
        bool moved = false;
    };
    std::optional<Drag> drag_;
};
