#pragma once
#include <d2d1.h>

class Painter;

// Draws the round bubble. `openness` goes from 0 (radar icon) to 1 (close icon).
void drawBubble(Painter& painter, D2D1_POINT_2F center, float openness, bool hovered, bool pressed);
