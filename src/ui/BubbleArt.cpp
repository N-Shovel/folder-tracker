#include "ui/BubbleArt.h"

#include <wrl/client.h>

#include <cmath>
#include <numbers>

#include "ui/Painter.h"
#include "ui/Theme.h"

using Microsoft::WRL::ComPtr;
using namespace theme;

namespace {

constexpr D2D1_COLOR_F kWhite{1, 1, 1, 1};

D2D1_POINT_2F pointAt(D2D1_POINT_2F center, float radius, float degrees) {
    const float radians = degrees * std::numbers::pi_v<float> / 180;
    return {center.x + radius * std::cos(radians), center.y + radius * std::sin(radians)};
}

void fillGradientCircle(ID2D1RenderTarget* target, D2D1_POINT_2F center, float radius) {
    const D2D1_GRADIENT_STOP stops[] = {{0, kBubbleFrom}, {1, kBubbleTo}};
    ComPtr<ID2D1GradientStopCollection> collection;
    target->CreateGradientStopCollection(stops, 2, &collection);
    ComPtr<ID2D1LinearGradientBrush> brush;
    target->CreateLinearGradientBrush(
        D2D1::LinearGradientBrushProperties({center.x - radius, center.y - radius}, {center.x + radius, center.y + radius}),
        collection.Get(), &brush);
    target->FillEllipse(D2D1::Ellipse(center, radius, radius), brush.Get());
}

// Rings, a sweep line and two blips.
void drawRadarIcon(Painter& painter, D2D1_POINT_2F c, float alpha) {
    const auto white = withAlpha(kWhite, alpha);
    painter.strokeCircle(c, 11, white, 2);
    painter.strokeCircle(c, 5.5f, withAlpha(kWhite, alpha * 0.6f), 1.6f);
    painter.line(c, pointAt(c, 11, -35), white, 2);
    painter.fillCircle(c, 2, white);
    painter.fillCircle({c.x + 5, c.y + 6}, 1.7f, white);
}

void drawCloseIcon(Painter& painter, D2D1_POINT_2F c, float alpha) {
    const auto white = withAlpha(kWhite, alpha);
    painter.line({c.x - 7, c.y - 7}, {c.x + 7, c.y + 7}, white, 2.2f);
    painter.line({c.x - 7, c.y + 7}, {c.x + 7, c.y - 7}, white, 2.2f);
}

}  // namespace

void drawBubble(Painter& painter, D2D1_POINT_2F center, float openness, bool hovered, bool pressed) {
    ID2D1RenderTarget* target = painter.target();
    const float radius = kBubbleSize / 2;
    const float scale = pressed ? kBubblePressScale : hovered ? kBubbleHoverScale : 1.0f;
    target->SetTransform(D2D1::Matrix3x2F::Scale(scale, scale, center));

    // Soft shadow: a few faint circles, slightly lower than the bubble.
    for (int i = 6; i >= 1; --i) {
        painter.fillCircle({center.x, center.y + 3}, radius + i * 1.2f, withAlpha(palette().shadow, 0.12f));
    }
    fillGradientCircle(target, center, radius);

    // The radar spins away and shrinks while the close icon spins in.
    const auto spin = [&](float degrees, float size) {
        target->SetTransform(D2D1::Matrix3x2F::Rotation(degrees, center) * D2D1::Matrix3x2F::Scale(size, size, center) *
                             D2D1::Matrix3x2F::Scale(scale, scale, center));
    };
    spin(90 * openness, 1 - 0.5f * openness);
    drawRadarIcon(painter, center, 1 - openness);
    spin(-90 * (1 - openness), 0.5f + 0.5f * openness);
    drawCloseIcon(painter, center, openness);

    target->SetTransform(D2D1::Matrix3x2F::Identity());
}
