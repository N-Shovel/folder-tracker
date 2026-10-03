#pragma once
#include <d2d1.h>

// Which corner of the window the bubble sits in. It is picked so the panel
// opens toward the middle of the screen (bubble bottom-right -> panel opens up and left).
struct Corner {
    bool bottom = true;
    bool right = true;
};

// Where everything goes inside the window, in DIPs.
struct PanelLayout {
    Corner corner;
    D2D1_POINT_2F bubbleCenter;
    D2D1_RECT_F panel;
    D2D1_RECT_F header;
    D2D1_RECT_F tabs;
    D2D1_RECT_F search;
    D2D1_RECT_F list;
};

PanelLayout computeLayout(Corner corner);
