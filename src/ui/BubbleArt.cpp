#include "ui/BubbleArt.h"

#include <wrl/client.h>


#include "ui/Painter.h"
#include "ui/Theme.h"

using Microsoft::WRL::ComPtr;
using namespace theme;

namespace {

constexpr D2D1_COLOR_F kWhite{1, 1, 1, 1};

ComPtr<ID2D1LinearGradientBrush> gradientBrush(ID2D1RenderTarget* target, D2D1_COLOR_F from, D2D1_COLOR_F to,
                                               D2D1_POINT_2F start, D2D1_POINT_2F end) {
    const D2D1_GRADIENT_STOP stops[] = {{0, from}, {1, to}};
    ComPtr<ID2D1GradientStopCollection> collection;
    target->CreateGradientStopCollection(stops, 2, &collection);
    ComPtr<ID2D1LinearGradientBrush> brush;
    target->CreateLinearGradientBrush(D2D1::LinearGradientBrushProperties(start, end), collection.Get(), &brush);
    return brush;
}

void fillGradientCircle(ID2D1RenderTarget* target, D2D1_POINT_2F center, float radius) {
    const auto brush = gradientBrush(target, kBubbleFrom, kBubbleTo, {center.x - radius, center.y - radius},
                                     {center.x + radius, center.y + radius});
    target->FillEllipse(D2D1::Ellipse(center, radius, radius), brush.Get());
}

// A git branch: a straight line with a node at each end, and a curved line up to a green "on GitHub" dot.
// Same drawing as resources/icon.svg.
void drawBranchIcon(Painter& painter, D2D1_POINT_2F c, float alpha) {
    ID2D1RenderTarget* target = painter.target();
    const auto at = [&](float x, float y) { return D2D1_POINT_2F{c.x + x, c.y + y}; };
    const auto line = gradientBrush(target, kBranchFrom, kBranchTo, at(-9, 12), at(11, -12));
    line->SetOpacity(alpha);

    ComPtr<ID2D1Factory> factory;
    target->GetFactory(&factory);
    ComPtr<ID2D1PathGeometry> curve;
    factory->CreatePathGeometry(&curve);
    ComPtr<ID2D1GeometrySink> sink;
    curve->Open(&sink);
    sink->BeginFigure(at(8.8f, -7), D2D1_FIGURE_BEGIN_HOLLOW);
    sink->AddLine(at(8.8f, -4.4f));
    sink->AddArc(D2D1::ArcSegment(at(2.8f, 1.6f), {6, 6}, 0, D2D1_SWEEP_DIRECTION_CLOCKWISE, D2D1_ARC_SIZE_SMALL));
    sink->AddLine(at(-1.2f, 1.6f));
    sink->AddArc(D2D1::ArcSegment(at(-7.2f, 7.6f), {6, 6}, 0, D2D1_SWEEP_DIRECTION_COUNTER_CLOCKWISE,
                                  D2D1_ARC_SIZE_SMALL));
    sink->EndFigure(D2D1_FIGURE_END_OPEN);
    sink->Close();

    target->DrawLine(at(-7.2f, -11.2f), at(-7.2f, 11.2f), line.Get(), 2.4f);
    target->DrawGeometry(curve.Get(), line.Get(), 2.4f);

    const auto blue = withAlpha(kBranchFrom, alpha);
    for (const float y : {-11.2f, 11.2f}) {
        painter.fillCircle(at(-7.2f, y), 3.4f, withAlpha(kBubbleTo, alpha));
        painter.strokeCircle(at(-7.2f, y), 3.4f, blue, 2);
    }
    painter.fillCircle(at(8.8f, -9.6f), 6, withAlpha(kBranchTo, alpha * 0.25f));  // glow
    painter.fillCircle(at(8.8f, -9.6f), 3.6f, withAlpha(kBranchTo, alpha));
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
    painter.strokeCircle(center, radius - 0.5f, kBubbleBorder, 1);

    // The branch spins away and shrinks while the close icon spins in.
    const auto spin = [&](float degrees, float size) {
        target->SetTransform(D2D1::Matrix3x2F::Rotation(degrees, center) * D2D1::Matrix3x2F::Scale(size, size, center) *
                             D2D1::Matrix3x2F::Scale(scale, scale, center));
    };
    spin(90 * openness, 1 - 0.5f * openness);
    drawBranchIcon(painter, center, 1 - openness);
    spin(-90 * (1 - openness), 0.5f + 0.5f * openness);
    drawCloseIcon(painter, center, openness);

    target->SetTransform(D2D1::Matrix3x2F::Identity());
}
