#include "ui/BubbleArt.h"

#include "ui/Painter.h"
#include "ui/Theme.h"

using namespace theme;

namespace {

constexpr Color kWhite{1, 1, 1, 1};

// A git branch: a straight line with a node at each end, and a curved line up to a green "on GitHub" dot.
// Same drawing as resources/icon.svg.
void drawBranchIcon(Painter& painter, PointF c, float alpha) {
    const auto at = [&](float x, float y) { return PointF{c.x + x, c.y + y}; };
    const LinearGradient line{at(-9, 12), at(11, -12), withAlpha(kBranchFrom, alpha), withAlpha(kBranchTo, alpha)};

    Path curve{at(8.8f, -7)};
    curve.lineTo(at(8.8f, -4.4f))
        .arcTo(at(2.8f, 1.6f), 6, true)
        .lineTo(at(-1.2f, 1.6f))
        .arcTo(at(-7.2f, 7.6f), 6, false);

    painter.line(at(-7.2f, -11.2f), at(-7.2f, 11.2f), line, 2.4f);
    painter.strokePath(curve, line, 2.4f);

    const auto blue = withAlpha(kBranchFrom, alpha);
    for (const float y : {-11.2f, 11.2f}) {
        painter.fillCircle(at(-7.2f, y), 3.4f, withAlpha(kBubbleTo, alpha));
        painter.strokeCircle(at(-7.2f, y), 3.4f, blue, 2);
    }
    painter.fillCircle(at(8.8f, -9.6f), 6, withAlpha(kBranchTo, alpha * 0.25f));  // glow
    painter.fillCircle(at(8.8f, -9.6f), 3.6f, withAlpha(kBranchTo, alpha));
}

void drawCloseIcon(Painter& painter, PointF c, float alpha) {
    const auto white = withAlpha(kWhite, alpha);
    painter.line({c.x - 7, c.y - 7}, {c.x + 7, c.y + 7}, white, 2.2f);
    painter.line({c.x - 7, c.y + 7}, {c.x + 7, c.y - 7}, white, 2.2f);
}

}  // namespace

void drawBubble(Painter& painter, PointF center, float openness, bool hovered, bool pressed) {
    const float radius = kBubbleSize / 2;
    const float scale = pressed ? kBubblePressScale : hovered ? kBubbleHoverScale : 1.0f;
    painter.setTransform(center, scale);

    // Soft shadow: a few faint circles, slightly lower than the bubble.
    for (int i = 6; i >= 1; --i) {
        painter.fillCircle({center.x, center.y + 3}, radius + i * 1.2f, withAlpha(palette().shadow, 0.12f));
    }
    painter.fillCircle(center, radius,
                       LinearGradient{{center.x - radius, center.y - radius}, {center.x + radius, center.y + radius},
                                      kBubbleFrom, kBubbleTo});
    painter.strokeCircle(center, radius - 0.5f, kBubbleBorder, 1);

    // The branch spins away and shrinks while the close icon spins in.
    painter.setTransform(center, scale * (1 - 0.5f * openness), 90 * openness);
    drawBranchIcon(painter, center, 1 - openness);
    painter.setTransform(center, scale * (0.5f + 0.5f * openness), -90 * (1 - openness));
    drawCloseIcon(painter, center, openness);

    painter.resetTransform();
}
