#pragma once
#include "ui/Geometry.h"

// Which corner of the window the bubble sits in. It is picked so the panel
// opens toward the middle of the screen (bubble bottom-right -> panel opens up and left).
struct Corner {
    bool bottom = true;
    bool right = true;
};

// Where everything goes inside the window, in DIPs.
struct PanelLayout {
    Corner corner;
    PointF bubbleCenter;
    RectF panel;
    RectF header;
    RectF tabs;
    RectF search;
    RectF list;
};

PanelLayout computeLayout(Corner corner);
