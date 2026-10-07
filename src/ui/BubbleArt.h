#pragma once
#include "ui/Geometry.h"

class Painter;

// Draws the round bubble. `openness` goes from 0 (branch icon) to 1 (close icon).
void drawBubble(Painter& painter, PointF center, float openness, bool hovered, bool pressed);
