#include "linux/BubbleWindow.h"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <utility>
#include <vector>

#include "core/Workspace.h"
#include "linux/AppIcon.h"
#include "linux/Shell.h"
#include "linux/Tray.h"
#include "ui/BubbleArt.h"
#include "linux/CairoPainter.h"
#include "ui/Theme.h"

using namespace theme;

namespace {

constexpr guint kFrameMs = 15;
constexpr int kWidth = int(kWindowWidth);
constexpr int kHeight = int(kWindowHeight);

// How opaque (0-255) a pixel must be to catch clicks. Fainter pixels, like the soft shadows,
// let clicks through to the window behind.
constexpr std::uint32_t kClickableAlpha = 64;

// A finished scan on its way from a scan thread to the main thread.
struct PendingScan {
    BubbleWindow* window;
    ScanResult* result;
};

GSettings* watchInterfaceSettings() {
    GSettingsSchemaSource* source = g_settings_schema_source_get_default();
    GSettingsSchema* schema = source ? g_settings_schema_source_lookup(source, "org.gnome.desktop.interface", TRUE)
                                     : nullptr;
    if (!schema) return nullptr;
    const bool hasColorScheme = g_settings_schema_has_key(schema, "color-scheme");
    g_settings_schema_unref(schema);
    return hasColorScheme ? g_settings_new("org.gnome.desktop.interface") : nullptr;
}

}  // namespace

BubbleWindow::BubbleWindow(Settings& settings, Workspace& workspace) : settings_(settings), workspace_(workspace) {}

BubbleWindow::~BubbleWindow() {
    if (animationTimer_) g_source_remove(animationTimer_);
    if (trayRefresh_) g_source_remove(trayRefresh_);
    if (GtkSettings* gtk = gtk_settings_get_default()) g_signal_handlers_disconnect_by_data(gtk, this);
    if (interfaceSettings_) {
        g_signal_handlers_disconnect_by_data(interfaceSettings_, this);
        g_object_unref(interfaceSettings_);
    }
    tray_.reset();
    if (window_) gtk_widget_destroy(window_);
    if (surface_) cairo_surface_destroy(surface_);
}

bool BubbleWindow::create(GtkApplication* app) {
    app_ = app;
    gtk_window_set_default_icon_from_file(appIconPath().c_str(), nullptr);

    window_ = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    if (!window_) return false;
    GtkWindow* window = GTK_WINDOW(window_);
    gtk_window_set_application(window, app);
    gtk_window_set_title(window, "Folder Tracker");

    // No frame, no taskbar button, floats above other windows, on every workspace.
    gtk_window_set_decorated(window, FALSE);
    gtk_window_set_resizable(window, FALSE);
    gtk_window_set_skip_taskbar_hint(window, TRUE);
    gtk_window_set_skip_pager_hint(window, TRUE);
    gtk_window_set_keep_above(window, TRUE);
    gtk_window_set_type_hint(window, GDK_WINDOW_TYPE_HINT_UTILITY);
    gtk_window_set_focus_on_map(window, FALSE);
    gtk_window_stick(window);
    gtk_window_set_default_size(window, kWidth, kHeight);
    gtk_widget_set_size_request(window_, kWidth, kHeight);

    // Per-pixel transparency, when the desktop has a compositor (almost all do).
    gtk_widget_set_app_paintable(window_, TRUE);
    GdkScreen* screen = gtk_widget_get_screen(window_);
    if (GdkVisual* visual = gdk_screen_get_rgba_visual(screen); visual && gdk_screen_is_composited(screen)) {
        gtk_widget_set_visual(window_, visual);
        composited_ = true;
    }

    gtk_widget_add_events(window_, GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK | GDK_POINTER_MOTION_MASK |
                                       GDK_LEAVE_NOTIFY_MASK | GDK_SCROLL_MASK | GDK_SMOOTH_SCROLL_MASK |
                                       GDK_KEY_PRESS_MASK | GDK_FOCUS_CHANGE_MASK);
    g_signal_connect(window_, "draw", G_CALLBACK(onDraw), this);
    g_signal_connect(window_, "button-press-event", G_CALLBACK(onButtonPress), this);
    g_signal_connect(window_, "button-release-event", G_CALLBACK(onButtonRelease), this);
    g_signal_connect(window_, "motion-notify-event", G_CALLBACK(onMotion), this);
    g_signal_connect(window_, "leave-notify-event", G_CALLBACK(onLeave), this);
    g_signal_connect(window_, "scroll-event", G_CALLBACK(onScrollEvent), this);
    g_signal_connect(window_, "key-press-event", G_CALLBACK(onKeyPressEvent), this);
    g_signal_connect(window_, "focus-out-event", G_CALLBACK(onFocusOut), this);
    g_signal_connect(window_, "delete-event", G_CALLBACK(onDelete), this);

    // Follow the desktop's light/dark setting.
    const auto themeChanged = +[](GObject*, GParamSpec*, gpointer self) {
        static_cast<BubbleWindow*>(self)->onSystemThemeChanged();
    };
    GtkSettings* gtk = gtk_settings_get_default();
    g_signal_connect(gtk, "notify::gtk-theme-name", G_CALLBACK(themeChanged), this);
    g_signal_connect(gtk, "notify::gtk-application-prefer-dark-theme", G_CALLBACK(themeChanged), this);
    if ((interfaceSettings_ = watchInterfaceSettings())) {
        g_signal_connect(interfaceSettings_, "changed::color-scheme",
                         G_CALLBACK(+[](GSettings*, gchar*, gpointer self) {
                             static_cast<BubbleWindow*>(self)->onSystemThemeChanged();
                         }),
                         this);
    }
    setThemeMode(settings_.themeMode());

    workspace_.attach([this](ScanResult* result) {
        g_idle_add(onScanFinished, new PendingScan{this, result});  // g_idle_add is safe from any thread
        return true;
    });

    gtk_widget_realize(window_);
    if (const auto saved = settings_.bubblePosition()) {
        placeBubble(*saved);
    } else {
        GdkDisplay* display = gdk_display_get_default();
        GdkMonitor* monitor = gdk_display_get_primary_monitor(display);
        if (!monitor) monitor = gdk_display_get_monitor(display, 0);
        GdkRectangle area{0, 0, 1280, 720};
        if (monitor) gdk_monitor_get_workarea(monitor, &area);
        placeBubble({area.x + area.width - 70, area.y + area.height - 70});
    }
    gtk_widget_show(window_);

    tray_ = std::make_unique<Tray>();
    onRefreshTray(this);
    return true;
}

void BubbleWindow::showPanel() {
    if (!gtk_widget_get_visible(window_)) toggleVisible();
    setOpen(true);
}

// ---------------------------------------------------------------------------------------------
// Position

ScreenPoint BubbleWindow::bubbleCenterOnScreen() const {
    const int offset = int(std::lround(kWindowGap + kBubbleSize / 2));
    return {corner_.right ? position_.x + kWidth - offset : position_.x + offset,
            corner_.bottom ? position_.y + kHeight - offset : position_.y + offset};
}

// Puts the bubble's center at `center`, choosing the corner so the panel opens toward
// the middle of the screen, and keeping the whole window on screen.
void BubbleWindow::placeBubble(ScreenPoint center) {
    GdkRectangle area{0, 0, 1280, 720};
    if (GdkMonitor* monitor = gdk_display_get_monitor_at_point(gdk_display_get_default(), center.x, center.y)) {
        gdk_monitor_get_workarea(monitor, &area);
    }
    corner_.right = center.x > area.x + area.width / 2;
    corner_.bottom = center.y > area.y + area.height / 2;
    layout_ = computeLayout(corner_);

    const int offset = int(std::lround(kWindowGap + kBubbleSize / 2));
    const int x = corner_.right ? center.x - (kWidth - offset) : center.x - offset;
    const int y = corner_.bottom ? center.y - (kHeight - offset) : center.y - offset;
    position_ = {std::max(area.x, std::min(x, area.x + area.width - kWidth)),
                 std::max(area.y, std::min(y, area.y + area.height - kHeight))};

    moveWindow();
    saveBubblePosition();
    render();
}

void BubbleWindow::saveBubblePosition() { settings_.setBubblePosition(bubbleCenterOnScreen()); }

void BubbleWindow::moveWindow() { gtk_window_move(GTK_WINDOW(window_), position_.x, position_.y); }

// ---------------------------------------------------------------------------------------------
// Panel

void BubbleWindow::setOpen(bool open) {
    if (open == open_) return;
    open_ = open;
    if (open) {
        gtk_window_present_with_time(GTK_WINDOW(window_), gtk_get_current_event_time());  // so typing goes to the search box
        panel_.invalidate();
    }
    reveal_.animateTo(open ? 1.0f : 0.0f, open ? kOpenMs : kCloseMs);
    if (!animationTimer_) animationTimer_ = g_timeout_add(kFrameMs, onAnimationFrame, this);
    render();
}

void BubbleWindow::toggleVisible() {
    if (gtk_widget_get_visible(window_)) {
        setOpen(false);
        gtk_widget_hide(window_);
    } else {
        moveWindow();  // some window managers forget the position of a hidden window
        gtk_widget_show(window_);
    }
    refreshTray();
}

void BubbleWindow::render() {
    if (!window_) return;
    const int scale = std::max(1, gtk_widget_get_scale_factor(window_));
    if (!surface_ || scale != scale_) {
        if (surface_) cairo_surface_destroy(surface_);
        surface_ = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, kWidth * scale, kHeight * scale);
        cairo_surface_set_device_scale(surface_, scale, scale);  // lets the rest of the code draw in DIPs
        scale_ = scale;
    }

    const float t = reveal_.value();
    const float eased = open_ ? easeOutCubic(t) : 1 - easeOutCubic(1 - t);
    const float opacity = open_ ? 1.0f : std::clamp(t / kCloseFadePortion, 0.0f, 1.0f);
    const bool interactive = open_ && !reveal_.running();

    cairo_t* cr = cairo_create(surface_);
    cairo_set_operator(cr, CAIRO_OPERATOR_CLEAR);
    cairo_paint(cr);
    cairo_set_operator(cr, CAIRO_OPERATOR_OVER);
    {
        CairoPainter painter(cr);
        if (t > 0) panel_.draw(painter, layout_, workspace_, eased, opacity, interactive, hovered_);
        drawBubble(painter, layout_.bubbleCenter, eased, hovered_.action == Action::Bubble, drag_ && !drag_->moved);
    }
    cairo_destroy(cr);

    updateInputShape();
    gtk_widget_queue_draw(window_);
}

// Only the parts of the window that are drawn catch the mouse, like a Windows layered window.
void BubbleWindow::updateInputShape() {
    cairo_surface_flush(surface_);
    const unsigned char* pixels = cairo_image_surface_get_data(surface_);
    const int stride = cairo_image_surface_get_stride(surface_);

    // One rectangle per run of solid pixels in a row; identical rows next to each other share rectangles.
    std::vector<cairo_rectangle_int_t> rects;
    std::vector<std::pair<int, int>> runs, previousRuns;
    size_t previousStart = 0;
    for (int y = 0; y < kHeight; ++y) {
        const auto* row = reinterpret_cast<const std::uint32_t*>(pixels + (y * scale_ + scale_ / 2) * stride);
        runs.clear();
        int runStart = -1;
        for (int x = 0; x <= kWidth; ++x) {
            const bool solid = x < kWidth && (row[x * scale_ + scale_ / 2] >> 24) >= kClickableAlpha;
            if (solid && runStart < 0) runStart = x;
            if (!solid && runStart >= 0) {
                runs.emplace_back(runStart, x);
                runStart = -1;
            }
        }
        if (!runs.empty() && runs == previousRuns) {
            for (size_t i = previousStart; i < rects.size(); ++i) ++rects[i].height;
        } else {
            previousStart = rects.size();
            for (const auto& [start, end] : runs) rects.push_back({start, y, end - start, 1});
        }
        std::swap(runs, previousRuns);
    }

    cairo_region_t* region = cairo_region_create_rectangles(rects.data(), int(rects.size()));
    if (composited_) gtk_widget_input_shape_combine_region(window_, region);
    else gtk_widget_shape_combine_region(window_, region);
    cairo_region_destroy(region);
}

// ---------------------------------------------------------------------------------------------
// Input

bool BubbleWindow::isOverBubble(PointF point) const {
    return std::hypot(point.x - layout_.bubbleCenter.x, point.y - layout_.bubbleCenter.y) <= kBubbleSize / 2;
}

void BubbleWindow::onLeftDown(PointF point, double rootX, double rootY) {
    if (isOverBubble(point)) {
        drag_ = Drag{rootX, rootY, position_};
        render();  // pressed look
    } else if (open_) {
        runAction(panel_.hitTest(point));
    }
}

void BubbleWindow::onMouseMove(PointF point, double rootX, double rootY) {
    if (drag_) {
        const double dx = rootX - drag_->cursorX;
        const double dy = rootY - drag_->cursorY;
        if (!drag_->moved && std::hypot(dx, dy) < kDragThreshold) return;
        drag_->moved = true;
        position_ = {drag_->windowStart.x + int(std::lround(dx)), drag_->windowStart.y + int(std::lround(dy))};
        moveWindow();
        return;
    }
    setHovered(isOverBubble(point) ? Hit{Action::Bubble} : open_ ? panel_.hitTest(point) : Hit{});
}

void BubbleWindow::onLeftUp() {
    if (!drag_) return;
    const Drag drag = *drag_;
    drag_.reset();

    if (!drag.moved) {
        setOpen(!open_);
    } else if (open_) {
        saveBubblePosition();
        render();
    } else {
        placeBubble(bubbleCenterOnScreen());  // may switch corners so the panel opens toward the screen's middle
    }
}

bool BubbleWindow::onKeyPress(const GdkEventKey& event) {
    if (!open_) return false;
    if (event.keyval == GDK_KEY_Escape) {
        if (!panel_.clearSearch()) setOpen(false);
    } else if (event.keyval == GDK_KEY_BackSpace) {
        panel_.backspace();
    } else {
        if (event.state & (GDK_CONTROL_MASK | GDK_MOD1_MASK | GDK_SUPER_MASK)) return false;
        const gunichar ch = gdk_keyval_to_unicode(event.keyval);
        if (ch < 0x20 || ch == 0x7F) return false;
        panel_.typeChar(wchar_t(ch));
    }
    render();
    return true;
}

void BubbleWindow::onScroll(const GdkEventScroll& event) {
    if (!open_) return;
    double notches = 0;
    switch (event.direction) {
        case GDK_SCROLL_UP: notches = -1; break;
        case GDK_SCROLL_DOWN: notches = 1; break;
        case GDK_SCROLL_SMOOTH: notches = event.delta_y; break;
        default: return;
    }
    panel_.scrollBy(float(notches) * 3 * kRowHeight);
    render();
}

void BubbleWindow::setHovered(Hit hit) {
    if (hit == hovered_) return;
    hovered_ = hit;
    updateCursor();
    render();
}

void BubbleWindow::updateCursor() {
    GdkWindow* window = gtk_widget_get_window(window_);
    if (!window) return;
    const char* name = hovered_.action == Action::Search ? "text"
                       : hovered_.action == Action::None ? "default"
                                                         : "pointer";
    GdkCursor* cursor = gdk_cursor_new_from_name(gtk_widget_get_display(window_), name);
    gdk_window_set_cursor(window, cursor);
    if (cursor) g_object_unref(cursor);
}

void BubbleWindow::runAction(Hit hit) {
    switch (hit.action) {
        case Action::AddFolder: addFolder(); break;
        case Action::RescanAll: workspace_.rescanAll(); break;
        case Action::Settings: showMenu(); break;
        case Action::FilterTab: panel_.setFilter(static_cast<Filter>(hit.index)); break;
        case Action::ToggleRow: panel_.toggleRow(hit.index); break;
        case Action::RescanRoot: workspace_.rescan(hit.index); break;
        case Action::RemoveRoot: workspace_.removeFolder(hit.index); break;
        case Action::OpenLink:
            if (const FolderNode* node = panel_.rowNode(hit.index)) openUrl(node->webUrl);
            break;
        default: return;
    }
    panel_.invalidate();
    render();
}

void BubbleWindow::addFolder() {
    ignoreDeactivate_ = true;  // the dialog takes focus; keep the panel open
    const auto folder = pickFolder(GTK_WINDOW(window_));
    ignoreDeactivate_ = false;
    gtk_window_present(GTK_WINDOW(window_));
    if (folder) workspace_.addFolder(*folder);
}

// ---------------------------------------------------------------------------------------------
// Menus

MenuState BubbleWindow::menuState(bool forTray) const {
    return {gtk_widget_get_visible(window_) != FALSE, startsAtLogin(), settings_.themeMode(), forTray};
}

void BubbleWindow::showMenu() {
    GtkWidget* menu = buildAppMenu(menuState(false), [this](MenuCommand command) { runCommand(command); });
    gtk_menu_attach_to_widget(GTK_MENU(menu), window_, nullptr);
    g_signal_connect(menu, "deactivate", G_CALLBACK(onMenuClosed), this);
    ignoreDeactivate_ = true;  // keep the panel open while the menu is up

    GdkEvent* trigger = gtk_get_current_event();
    gtk_menu_popup_at_pointer(GTK_MENU(menu), trigger);
    if (trigger) gdk_event_free(trigger);
}

void BubbleWindow::runCommand(MenuCommand command) {
    switch (command) {
        case MenuCommand::ShowPanel: showPanel(); break;
        case MenuCommand::ToggleBubble: toggleVisible(); break;
        case MenuCommand::StartAtLogin: setStartAtLogin(!startsAtLogin()); break;
        case MenuCommand::ThemeSystem:
        case MenuCommand::ThemeDark:
        case MenuCommand::ThemeLight: {
            const auto mode = static_cast<ThemeMode>(int(command) - int(MenuCommand::ThemeSystem));
            settings_.setThemeMode(mode);
            setThemeMode(mode);
            panel_.invalidate();
            render();
            break;
        }
        case MenuCommand::Quit: quit(); return;
        case MenuCommand::None: return;
    }
    refreshTray();
}

// The tray menu shows the current state, so it is rebuilt when something changes. Later, not
// right away: this often runs from inside one of that menu's own items.
void BubbleWindow::refreshTray() {
    if (tray_ && tray_->available() && !trayRefresh_) trayRefresh_ = g_idle_add(onRefreshTray, this);
}

void BubbleWindow::onSystemThemeChanged() {
    refreshPalette();
    panel_.invalidate();
    render();
}

void BubbleWindow::quit() { g_application_quit(G_APPLICATION(app_)); }

// ---------------------------------------------------------------------------------------------
// GTK callbacks

gboolean BubbleWindow::onDraw(GtkWidget*, cairo_t* cr, gpointer data) {
    auto* self = static_cast<BubbleWindow*>(data);
    if (!self->surface_) return TRUE;
    cairo_set_operator(cr, CAIRO_OPERATOR_SOURCE);
    cairo_set_source_surface(cr, self->surface_, 0, 0);
    cairo_paint(cr);
    return TRUE;
}

gboolean BubbleWindow::onButtonPress(GtkWidget*, GdkEventButton* event, gpointer data) {
    auto* self = static_cast<BubbleWindow*>(data);
    if (event->type == GDK_BUTTON_PRESS && event->button == GDK_BUTTON_PRIMARY) {
        self->onLeftDown({float(event->x), float(event->y)}, event->x_root, event->y_root);
    }
    return TRUE;
}

gboolean BubbleWindow::onButtonRelease(GtkWidget*, GdkEventButton* event, gpointer data) {
    auto* self = static_cast<BubbleWindow*>(data);
    if (event->button == GDK_BUTTON_PRIMARY) {
        self->onLeftUp();
    } else if (event->button == GDK_BUTTON_SECONDARY && self->isOverBubble({float(event->x), float(event->y)})) {
        self->showMenu();
    }
    return TRUE;
}

gboolean BubbleWindow::onMotion(GtkWidget*, GdkEventMotion* event, gpointer data) {
    static_cast<BubbleWindow*>(data)->onMouseMove({float(event->x), float(event->y)}, event->x_root, event->y_root);
    return TRUE;
}

gboolean BubbleWindow::onLeave(GtkWidget*, GdkEventCrossing*, gpointer data) {
    auto* self = static_cast<BubbleWindow*>(data);
    if (!self->drag_) self->setHovered({});
    return FALSE;
}

gboolean BubbleWindow::onScrollEvent(GtkWidget*, GdkEventScroll* event, gpointer data) {
    static_cast<BubbleWindow*>(data)->onScroll(*event);
    return TRUE;
}

gboolean BubbleWindow::onKeyPressEvent(GtkWidget*, GdkEventKey* event, gpointer data) {
    return static_cast<BubbleWindow*>(data)->onKeyPress(*event);
}

gboolean BubbleWindow::onFocusOut(GtkWidget*, GdkEventFocus*, gpointer data) {
    auto* self = static_cast<BubbleWindow*>(data);
    if (!self->ignoreDeactivate_) self->setOpen(false);  // clicked somewhere else
    return FALSE;
}

gboolean BubbleWindow::onDelete(GtkWidget*, GdkEvent*, gpointer data) {
    static_cast<BubbleWindow*>(data)->quit();  // Alt+F4 on the bubble quits, like on Windows
    return TRUE;
}

void BubbleWindow::onMenuClosed(GtkMenuShell* menu, gpointer data) {
    static_cast<BubbleWindow*>(data)->ignoreDeactivate_ = false;
    // The chosen item runs after this signal, so the menu can't be destroyed yet.
    g_idle_add(
        [](gpointer widget) {
            gtk_widget_destroy(GTK_WIDGET(widget));
            return G_SOURCE_REMOVE;
        },
        menu);
}

gboolean BubbleWindow::onAnimationFrame(gpointer data) {
    auto* self = static_cast<BubbleWindow*>(data);
    const bool running = self->reveal_.update();
    self->render();
    if (running) return G_SOURCE_CONTINUE;
    self->animationTimer_ = 0;
    return G_SOURCE_REMOVE;
}

gboolean BubbleWindow::onRefreshTray(gpointer data) {
    auto* self = static_cast<BubbleWindow*>(data);
    self->trayRefresh_ = 0;
    if (!self->tray_ || !self->tray_->available()) return G_SOURCE_REMOVE;
    GtkWidget* toggleItem = nullptr;
    GtkWidget* menu = buildAppMenu(self->menuState(true), [self](MenuCommand command) { self->runCommand(command); },
                                   &toggleItem);
    self->tray_->setMenu(menu, toggleItem);
    return G_SOURCE_REMOVE;
}

gboolean BubbleWindow::onScanFinished(gpointer data) {
    const std::unique_ptr<PendingScan> pending(static_cast<PendingScan*>(data));
    BubbleWindow* self = pending->window;
    self->workspace_.finishScan(std::unique_ptr<ScanResult>(pending->result));
    self->panel_.invalidate();
    self->render();
    return G_SOURCE_REMOVE;
}
