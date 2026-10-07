#pragma once
#include <windows.h>

#include <memory>
#include <optional>

#include "ui/Animation.h"
#include "windows/LayeredSurface.h"
#include "ui/Layout.h"
#include "ui/PanelView.h"

class Settings;
class Tray;
class Workspace;

// The floating window: the bubble plus the panel that grows out of it.
// It is always the size of the open panel; while closed, only the bubble is drawn
// and the transparent rest lets clicks through.
class BubbleWindow {
public:
    BubbleWindow(Settings& settings, Workspace& workspace);
    ~BubbleWindow();

    bool create(HINSTANCE instance);

private:
    static LRESULT CALLBACK windowProc(HWND window, UINT message, WPARAM wParam, LPARAM lParam);
    LRESULT handleMessage(UINT message, WPARAM wParam, LPARAM lParam);

    // Position (screen pixels)
    float scale() const { return dpi_ / 96.0f; }
    SIZE windowSize() const;
    POINT bubbleCenterOnScreen() const;
    void placeBubble(POINT center);
    void saveBubblePosition();

    // Panel
    void setOpen(bool open);
    void toggleVisible();
    void onAnimationTick();
    void render();

    // Input
    PointF toDips(LPARAM lParam) const;
    bool isOverBubble(PointF point) const;
    void onLeftDown(PointF point);
    void onMouseMove(PointF point);
    void onLeftUp();
    void onKeyDown(WPARAM key);
    void onChar(wchar_t ch);
    void setHovered(Hit hit);
    void runAction(Hit hit);
    void addFolder();
    void showMenu();

    Settings& settings_;
    Workspace& workspace_;
    HWND window_ = nullptr;
    std::unique_ptr<Tray> tray_;
    UINT taskbarCreatedMessage_ = 0;

    LayeredSurface surface_;
    PanelView panel_;
    PanelLayout layout_;
    Corner corner_;
    float dpi_ = 0;
    POINT position_{};  // window's top-left corner on screen

    Animation reveal_;
    bool open_ = false;
    bool ignoreDeactivate_ = false;  // while a dialog is open
    Hit hovered_;
    bool trackingMouse_ = false;

    struct Drag {
        POINT cursorStart;
        POINT windowStart;
        bool moved = false;
    };
    std::optional<Drag> drag_;
};
