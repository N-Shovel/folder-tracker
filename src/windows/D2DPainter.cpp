#include "windows/D2DPainter.h"

#include "windows/Graphics.h"

using Microsoft::WRL::ComPtr;

namespace {

D2D1_POINT_2F toD2D(PointF p) { return {p.x, p.y}; }
D2D1_RECT_F toD2D(const RectF& r) { return {r.left, r.top, r.right, r.bottom}; }
D2D1_COLOR_F toD2D(const Color& c) { return {c.r, c.g, c.b, c.a}; }

// Characters from Windows' built-in icon font (Segoe Fluent Icons / Segoe MDL2 Assets).
// Browse them with the "Character Map" app, or at learn.microsoft.com (search "Segoe Fluent Icons").
wchar_t glyphFor(Icon icon) {
    switch (icon) {
        case Icon::Folder: return 0xE8B7;
        case Icon::FolderOpen: return 0xE838;
        case Icon::ChevronRight: return 0xE76C;
        case Icon::ChevronDown: return 0xE70D;
        case Icon::Refresh: return 0xE72C;
        case Icon::Delete: return 0xE74D;
        case Icon::OpenLink: return 0xE8A7;
        case Icon::AddFolder: return 0xE8F4;
        case Icon::Search: return 0xE721;
        case Icon::Settings: return 0xE713;
        case Icon::GitHub: return 0xE753;
        case Icon::Local: return 0xEDA2;
        case Icon::Untracked: return 0xE7BA;
    }
    return L' ';
}

}  // namespace

D2DPainter::D2DPainter(ID2D1RenderTarget* target) : target_(target) {
    target_->CreateSolidColorBrush(D2D1::ColorF(0, 0, 0), &brush_);
}

ID2D1Brush* D2DPainter::brush(const Color& color) {
    brush_->SetColor(toD2D(color));
    return brush_.Get();
}

ComPtr<ID2D1Brush> D2DPainter::gradientBrush(const LinearGradient& gradient) {
    const D2D1_GRADIENT_STOP stops[] = {{0, toD2D(gradient.from)}, {1, toD2D(gradient.to)}};
    ComPtr<ID2D1GradientStopCollection> collection;
    target_->CreateGradientStopCollection(stops, 2, &collection);
    ComPtr<ID2D1LinearGradientBrush> brush;
    target_->CreateLinearGradientBrush(
        D2D1::LinearGradientBrushProperties(toD2D(gradient.start), toD2D(gradient.end)), collection.Get(), &brush);
    return brush;
}

void D2DPainter::fillRect(const RectF& rect, const Color& color) { target_->FillRectangle(toD2D(rect), brush(color)); }

void D2DPainter::fillRoundRect(const RectF& rect, float radius, const Color& color) {
    target_->FillRoundedRectangle(D2D1::RoundedRect(toD2D(rect), radius, radius), brush(color));
}

void D2DPainter::strokeRoundRect(const RectF& rect, float radius, const Color& color, float strokeWidth) {
    target_->DrawRoundedRectangle(D2D1::RoundedRect(toD2D(rect), radius, radius), brush(color), strokeWidth);
}

void D2DPainter::fillCircle(PointF center, float radius, const Color& color) {
    target_->FillEllipse(D2D1::Ellipse(toD2D(center), radius, radius), brush(color));
}

void D2DPainter::fillCircle(PointF center, float radius, const LinearGradient& gradient) {
    target_->FillEllipse(D2D1::Ellipse(toD2D(center), radius, radius), gradientBrush(gradient).Get());
}

void D2DPainter::strokeCircle(PointF center, float radius, const Color& color, float strokeWidth) {
    target_->DrawEllipse(D2D1::Ellipse(toD2D(center), radius, radius), brush(color), strokeWidth);
}

void D2DPainter::line(PointF from, PointF to, const Color& color, float strokeWidth) {
    target_->DrawLine(toD2D(from), toD2D(to), brush(color), strokeWidth);
}

void D2DPainter::line(PointF from, PointF to, const LinearGradient& gradient, float strokeWidth) {
    target_->DrawLine(toD2D(from), toD2D(to), gradientBrush(gradient).Get(), strokeWidth);
}

void D2DPainter::strokePath(const Path& path, const LinearGradient& gradient, float strokeWidth) {
    ComPtr<ID2D1PathGeometry> geometry;
    Graphics::instance().d2d()->CreatePathGeometry(&geometry);
    ComPtr<ID2D1GeometrySink> sink;
    geometry->Open(&sink);
    sink->BeginFigure(toD2D(path.start), D2D1_FIGURE_BEGIN_HOLLOW);
    for (const Path::Segment& segment : path.segments) {
        if (segment.arcRadius > 0) {
            sink->AddArc(D2D1::ArcSegment(
                toD2D(segment.to), {segment.arcRadius, segment.arcRadius}, 0,
                segment.clockwise ? D2D1_SWEEP_DIRECTION_CLOCKWISE : D2D1_SWEEP_DIRECTION_COUNTER_CLOCKWISE,
                D2D1_ARC_SIZE_SMALL));
        } else {
            sink->AddLine(toD2D(segment.to));
        }
    }
    sink->EndFigure(D2D1_FIGURE_END_OPEN);
    sink->Close();
    target_->DrawGeometry(geometry.Get(), gradientBrush(gradient).Get(), strokeWidth);
}

void D2DPainter::text(std::wstring_view text, const RectF& rect, Font font, const Color& color, Align align) {
    IDWriteTextFormat* format = Graphics::instance().format(font);
    format->SetTextAlignment(align == Align::Center  ? DWRITE_TEXT_ALIGNMENT_CENTER
                             : align == Align::Right ? DWRITE_TEXT_ALIGNMENT_TRAILING
                                                     : DWRITE_TEXT_ALIGNMENT_LEADING);
    target_->DrawText(text.data(), UINT32(text.size()), format, toD2D(rect), brush(color), D2D1_DRAW_TEXT_OPTIONS_CLIP);
}

void D2DPainter::icon(Icon icon, PointF center, Font font, const Color& color) {
    const wchar_t glyph = glyphFor(icon);
    text(std::wstring_view(&glyph, 1), squareAround(center, 24), font, color, Align::Center);
}

float D2DPainter::textWidth(std::wstring_view text, Font font) const {
    ComPtr<IDWriteTextLayout> layout;
    Graphics::instance().dwrite()->CreateTextLayout(text.data(), UINT32(text.size()), Graphics::instance().format(font),
                                                    10000, 100, &layout);
    DWRITE_TEXT_METRICS metrics{};
    layout->GetMetrics(&metrics);
    return metrics.widthIncludingTrailingWhitespace;
}

void D2DPainter::pushCircleLayer(PointF center, float radius, float opacity) {
    ComPtr<ID2D1EllipseGeometry> circle;
    Graphics::instance().d2d()->CreateEllipseGeometry(D2D1::Ellipse(toD2D(center), radius, radius), &circle);
    ComPtr<ID2D1Layer> layer;
    target_->CreateLayer(&layer);
    target_->PushLayer(D2D1::LayerParameters(D2D1::InfiniteRect(), circle.Get(), D2D1_ANTIALIAS_MODE_PER_PRIMITIVE,
                                             D2D1::IdentityMatrix(), opacity),
                       layer.Get());
    layers_.push_back(layer);
}

void D2DPainter::popLayer() {
    target_->PopLayer();
    layers_.pop_back();
}

void D2DPainter::pushClip(const RectF& rect) { target_->PushAxisAlignedClip(toD2D(rect), D2D1_ANTIALIAS_MODE_ALIASED); }

void D2DPainter::popClip() { target_->PopAxisAlignedClip(); }

void D2DPainter::setTransform(PointF center, float scale, float degrees) {
    target_->SetTransform(D2D1::Matrix3x2F::Rotation(degrees, toD2D(center)) *
                          D2D1::Matrix3x2F::Scale(scale, scale, toD2D(center)));
}

void D2DPainter::resetTransform() { target_->SetTransform(D2D1::Matrix3x2F::Identity()); }
