#include "ui/Layout.h"

#include "ui/Theme.h"

using namespace theme;

PanelLayout computeLayout(Corner corner) {
    PanelLayout layout;
    layout.corner = corner;
    layout.panel = {kWindowGap, kWindowGap, kWindowWidth - kWindowGap, kWindowHeight - kWindowGap};

    const float bubbleOffset = kWindowGap + kBubbleSize / 2;
    layout.bubbleCenter = {corner.right ? kWindowWidth - bubbleOffset : bubbleOffset,
                           corner.bottom ? kWindowHeight - bubbleOffset : bubbleOffset};

    // Stack the rows starting from the bubble's edge: header, filter tabs, search box.
    // The folder list fills whatever is left.
    const RectF& p = layout.panel;
    const float left = p.left + kPanelPadding;
    const float right = p.right - kPanelPadding;
    const float listInset = 6;

    if (!corner.bottom) {
        float y = p.top;
        layout.header = {p.left, y, p.right, y + kHeaderHeight};
        y += kHeaderHeight;
        layout.tabs = {left, y, right, y + kControlHeight};
        y += kControlHeight + kControlGap;
        layout.search = {left, y, right, y + kControlHeight};
        y += kControlHeight + kControlGap;
        layout.list = {p.left + listInset, y, p.right - listInset, p.bottom - listInset};
    } else {
        float y = p.bottom;
        layout.header = {p.left, y - kHeaderHeight, p.right, y};
        y -= kHeaderHeight;
        layout.tabs = {left, y - kControlHeight, right, y};
        y -= kControlHeight + kControlGap;
        layout.search = {left, y - kControlHeight, right, y};
        y -= kControlHeight + kControlGap;
        layout.list = {p.left + listInset, p.top + listInset, p.right - listInset, y};
    }
    return layout;
}
