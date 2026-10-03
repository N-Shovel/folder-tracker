#include "ui/BubbleWindow.h"

#include <shellscalingapi.h>
#include <windowsx.h>

#include <algorithm>
#include <cmath>

#include "Messages.h"
#include "core/Workspace.h"
#include "platform/AppMenu.h"
#include "platform/Settings.h"
#include "platform/Shell.h"
#include "platform/Tray.h"
#include "resource.h"
#include "ui/BubbleArt.h"
#include "ui/Painter.h"
#include "ui/Theme.h"

using namespace theme;

namespace {
constexpr UINT_PTR kAnimationTimer = 1;
constexpr UINT kFrameMs = 15;
}  // namespace

BubbleWindow::BubbleWindow(Settings& settings, Workspace& workspace) : settings_(settings), workspace_(workspace) {}

BubbleWindow::~BubbleWindow() = default;

bool BubbleWindow::create(HINSTANCE instance) {
    HICON bigIcon = LoadIconW(instance, MAKEINTRESOURCEW(IDI_APP_ICON));
    HICON smallIcon = static_cast<HICON>(LoadImageW(instance, MAKEINTRESOURCEW(IDI_APP_ICON), IMAGE_ICON,
                                                    GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON), 0));

    WNDCLASSEXW windowClass{sizeof(windowClass)};
    windowClass.lpfnWndProc = windowProc;
    windowClass.hInstance = instance;
    windowClass.hCursor = LoadCursorW(nullptr, IDC_ARROW);
    windowClass.hIcon = bigIcon;
    windowClass.hIconSm = smallIcon;
    windowClass.lpszClassName = kWindowClass;
    RegisterClassExW(&windowClass);

    // Layered: per-pixel transparency. Topmost: floats above other windows. Tool window: no taskbar button.
    window_ = CreateWindowExW(WS_EX_LAYERED | WS_EX_TOPMOST | WS_EX_TOOLWINDOW, kWindowClass, L"Folder Tracker",
                              WS_POPUP, 0, 0, 0, 0, nullptr, nullptr, instance, this);
    if (!window_) return false;

    workspace_.attach(window_);
    tray_ = std::make_unique<Tray>(window_, smallIcon);
    taskbarCreatedMessage_ = RegisterWindowMessageW(L"TaskbarCreated");

    const auto saved = settings_.bubblePosition();
    if (saved) {
        placeBubble(*saved);
    } else {
        MONITORINFO info{sizeof(info)};
        GetMonitorInfoW(MonitorFromPoint({0, 0}, MONITOR_DEFAULTTOPRIMARY), &info);
        placeBubble({info.rcWork.right - 70, info.rcWork.bottom - 70});
    }
    ShowWindow(window_, SW_SHOWNOACTIVATE);
    return true;
}

// ---------------------------------------------------------------------------------------------
// Position

SIZE BubbleWindow::windowSize() const {
    return {LONG(std::lround(kWindowWidth * scale())), LONG(std::lround(kWindowHeight * scale()))};
}

POINT BubbleWindow::bubbleCenterOnScreen() const {
    const SIZE size = windowSize();
    const LONG offset = std::lround((kWindowGap + kBubbleSize / 2) * scale());
    return {corner_.right ? position_.x + size.cx - offset : position_.x + offset,
            corner_.bottom ? position_.y + size.cy - offset : position_.y + offset};
}

// Puts the bubble's center at `center`, choosing the corner so the panel opens toward
// the middle of the screen, and keeping the whole window on screen.
void BubbleWindow::placeBubble(POINT center) {
    HMONITOR monitor = MonitorFromPoint(center, MONITOR_DEFAULTTONEAREST);
    UINT dpiX = 96, dpiY = 96;
    GetDpiForMonitor(monitor, MDT_EFFECTIVE_DPI, &dpiX, &dpiY);
    if (float(dpiX) != dpi_) {
        dpi_ = float(dpiX);
        surface_.resize(windowSize(), dpi_);
    }

    MONITORINFO info{sizeof(info)};
    GetMonitorInfoW(monitor, &info);
    const RECT& area = info.rcWork;
    corner_.right = center.x > (area.left + area.right) / 2;
    corner_.bottom = center.y > (area.top + area.bottom) / 2;
    layout_ = computeLayout(corner_);

    const SIZE size = windowSize();
    const LONG offset = std::lround((kWindowGap + kBubbleSize / 2) * scale());
    const LONG x = corner_.right ? center.x - (size.cx - offset) : center.x - offset;
    const LONG y = corner_.bottom ? center.y - (size.cy - offset) : center.y - offset;
    position_ = {std::clamp(x, area.left, area.right - size.cx), std::clamp(y, area.top, area.bottom - size.cy)};

    settings_.setBubblePosition(bubbleCenterOnScreen());
    render();
}

// ---------------------------------------------------------------------------------------------
// Panel

void BubbleWindow::setOpen(bool open) {
    if (open == open_) return;
    open_ = open;
    if (open) {
        SetForegroundWindow(window_);  // so typing goes to the search box
        panel_.invalidate();
    }
    reveal_.animateTo(open ? 1.0f : 0.0f, open ? kOpenMs : kCloseMs);
    SetTimer(window_, kAnimationTimer, kFrameMs, nullptr);
    render();
}

void BubbleWindow::toggleVisible() {
    if (IsWindowVisible(window_)) {
        setOpen(false);
        ShowWindow(window_, SW_HIDE);
    } else {
        ShowWindow(window_, SW_SHOWNOACTIVATE);
    }
}

void BubbleWindow::onAnimationTick() {
    if (!reveal_.update()) KillTimer(window_, kAnimationTimer);
    render();
}

void BubbleWindow::render() {
    ID2D1DCRenderTarget* target = surface_.target();
    if (!target) return;

    const float t = reveal_.value();
    const float eased = open_ ? easeOutCubic(t) : 1 - easeOutCubic(1 - t);
    const float opacity = open_ ? 1.0f : std::clamp(t / kCloseFadePortion, 0.0f, 1.0f);
    const bool interactive = open_ && !reveal_.running();

    target->BeginDraw();
    target->Clear(D2D1::ColorF(0, 0, 0, 0));
    Painter painter(target);
    if (t > 0) panel_.draw(painter, layout_, workspace_, eased, opacity, interactive, hovered_);
    drawBubble(painter, layout_.bubbleCenter, eased, hovered_.action == Action::Bubble, drag_ && !drag_->moved);
    target->EndDraw();

    surface_.present(window_, position_);
}

// ---------------------------------------------------------------------------------------------
// Input

D2D1_POINT_2F BubbleWindow::toDips(LPARAM lParam) const {
    return {GET_X_LPARAM(lParam) / scale(), GET_Y_LPARAM(lParam) / scale()};
}

bool BubbleWindow::isOverBubble(D2D1_POINT_2F point) const {
    return std::hypot(point.x - layout_.bubbleCenter.x, point.y - layout_.bubbleCenter.y) <= kBubbleSize / 2;
}

void BubbleWindow::onLeftDown(D2D1_POINT_2F point) {
    if (isOverBubble(point)) {
        POINT cursor;
        GetCursorPos(&cursor);
        drag_ = Drag{cursor, position_};
        SetCapture(window_);
        render();  // pressed look
    } else if (open_) {
        runAction(panel_.hitTest(point));
    }
}

void BubbleWindow::onMouseMove(D2D1_POINT_2F point) {
    if (!trackingMouse_) {
        TRACKMOUSEEVENT track{sizeof(track), TME_LEAVE, window_, 0};
        trackingMouse_ = TrackMouseEvent(&track);
    }

    if (drag_) {
        POINT cursor;
        GetCursorPos(&cursor);
        const LONG dx = cursor.x - drag_->cursorStart.x;
        const LONG dy = cursor.y - drag_->cursorStart.y;
        if (!drag_->moved && std::hypot(float(dx), float(dy)) < kDragThreshold * scale()) return;
        drag_->moved = true;
        position_ = {drag_->windowStart.x + dx, drag_->windowStart.y + dy};
        SetWindowPos(window_, nullptr, position_.x, position_.y, 0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);
        return;
    }

    setHovered(isOverBubble(point) ? Hit{Action::Bubble} : open_ ? panel_.hitTest(point) : Hit{});
}

void BubbleWindow::onLeftUp() {
    if (!drag_) return;
    const Drag drag = *drag_;
    drag_.reset();
    ReleaseCapture();

    if (!drag.moved) {
        setOpen(!open_);
    } else if (open_) {
        settings_.setBubblePosition(bubbleCenterOnScreen());
        render();
    } else {
        placeBubble(bubbleCenterOnScreen());  // may switch corners so the panel opens toward the screen's middle
    }
}

void BubbleWindow::onKeyDown(WPARAM key) {
    if (key != VK_ESCAPE || !open_) return;
    if (!panel_.clearSearch()) setOpen(false);
    render();
}

void BubbleWindow::onChar(wchar_t ch) {
    if (!open_) return;
    if (ch == L'\b') panel_.backspace();
    else if (ch >= L' ' && ch != 127) panel_.typeChar(ch);
    else return;
    render();
}

void BubbleWindow::setHovered(Hit hit) {
    if (hit == hovered_) return;
    hovered_ = hit;
    render();
}

void BubbleWindow::runAction(Hit hit) {
    switch (hit.action) {
        case Action::AddFolder: addFolder(); break;
        case Action::RescanAll: workspace_.rescanAll(); break;
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
    const auto folder = pickFolder(window_);
    ignoreDeactivate_ = false;
    SetForegroundWindow(window_);
    if (folder) workspace_.addFolder(*folder);
}

void BubbleWindow::showMenu() {
    switch (showAppMenu(window_, IsWindowVisible(window_))) {
        case MenuCommand::ToggleBubble: toggleVisible(); break;
        case MenuCommand::StartWithWindows: setStartWithWindows(!startsWithWindows()); break;
        case MenuCommand::Quit: DestroyWindow(window_); break;
        case MenuCommand::None: break;
    }
}

// ---------------------------------------------------------------------------------------------
// Messages

LRESULT CALLBACK BubbleWindow::windowProc(HWND window, UINT message, WPARAM wParam, LPARAM lParam) {
    if (message == WM_NCCREATE) {
        auto* self = static_cast<BubbleWindow*>(reinterpret_cast<CREATESTRUCTW*>(lParam)->lpCreateParams);
        self->window_ = window;
        SetWindowLongPtrW(window, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(self));
    }
    auto* self = reinterpret_cast<BubbleWindow*>(GetWindowLongPtrW(window, GWLP_USERDATA));
    return self ? self->handleMessage(message, wParam, lParam) : DefWindowProcW(window, message, wParam, lParam);
}

LRESULT BubbleWindow::handleMessage(UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) {
        case WM_TIMER: onAnimationTick(); return 0;
        case WM_LBUTTONDOWN: onLeftDown(toDips(lParam)); return 0;
        case WM_MOUSEMOVE: onMouseMove(toDips(lParam)); return 0;
        case WM_LBUTTONUP: onLeftUp(); return 0;
        case WM_RBUTTONUP:
            if (isOverBubble(toDips(lParam))) showMenu();
            return 0;
        case WM_MOUSELEAVE:
            trackingMouse_ = false;
            setHovered({});
            return 0;
        case WM_MOUSEWHEEL:
            if (open_) {
                panel_.scrollBy(-GET_WHEEL_DELTA_WPARAM(wParam) / float(WHEEL_DELTA) * 3 * kRowHeight);
                render();
            }
            return 0;
        case WM_SETCURSOR:
            if (LOWORD(lParam) == HTCLIENT) {
                const auto cursor = hovered_.action == Action::Search ? IDC_IBEAM
                                    : hovered_.action == Action::None ? IDC_ARROW
                                                                      : IDC_HAND;
                SetCursor(LoadCursorW(nullptr, cursor));
                return TRUE;
            }
            break;
        case WM_KEYDOWN: onKeyDown(wParam); return 0;
        case WM_CHAR: onChar(wchar_t(wParam)); return 0;
        case WM_ACTIVATE:
            if (LOWORD(wParam) == WA_INACTIVE && !ignoreDeactivate_) setOpen(false);  // clicked somewhere else
            return 0;
        case WM_SCAN_FINISHED:
            workspace_.finishScan(std::unique_ptr<ScanResult>(reinterpret_cast<ScanResult*>(lParam)));
            panel_.invalidate();
            render();
            return 0;
        case WM_TRAY_ICON:
            if (lParam == WM_LBUTTONUP) toggleVisible();
            if (lParam == WM_RBUTTONUP) showMenu();
            return 0;
        case WM_SHOW_PANEL:
            ShowWindow(window_, SW_SHOWNOACTIVATE);
            setOpen(true);
            return 0;
        case WM_SETTINGCHANGE:
            refreshPalette();  // light/dark mode may have changed
            render();
            break;
        case WM_DPICHANGED: return 0;  // handled in placeBubble when the bubble is dropped
        case WM_DESTROY:
            tray_.reset();
            PostQuitMessage(0);
            return 0;
        default:
            if (message == taskbarCreatedMessage_ && tray_) tray_->restore();
            break;
    }
    return DefWindowProcW(window_, message, wParam, lParam);
}
