#include "ui/Painter.h"

#include "ui/Geometry.h"

using Microsoft::WRL::ComPtr;

Painter::Painter(ID2D1RenderTarget* target) : target_(target) {
    target_->CreateSolidColorBrush(D2D1::ColorF(0, 0, 0), &brush_);
}

ID2D1SolidColorBrush* Painter::brush(const D2D1_COLOR_F& color) {
    brush_->SetColor(color);
    return brush_.Get();
}

void Painter::fillRect(const D2D1_RECT_F& rect, const D2D1_COLOR_F& color) {
    target_->FillRectangle(rect, brush(color));
}

void Painter::fillRoundRect(const D2D1_RECT_F& rect, float radius, const D2D1_COLOR_F& color) {
    target_->FillRoundedRectangle(D2D1::RoundedRect(rect, radius, radius), brush(color));
}

void Painter::strokeRoundRect(const D2D1_RECT_F& rect, float radius, const D2D1_COLOR_F& color, float strokeWidth) {
    target_->DrawRoundedRectangle(D2D1::RoundedRect(rect, radius, radius), brush(color), strokeWidth);
}

void Painter::fillCircle(D2D1_POINT_2F center, float radius, const D2D1_COLOR_F& color) {
    target_->FillEllipse(D2D1::Ellipse(center, radius, radius), brush(color));
}

void Painter::strokeCircle(D2D1_POINT_2F center, float radius, const D2D1_COLOR_F& color, float strokeWidth) {
    target_->DrawEllipse(D2D1::Ellipse(center, radius, radius), brush(color), strokeWidth);
}

void Painter::line(D2D1_POINT_2F from, D2D1_POINT_2F to, const D2D1_COLOR_F& color, float strokeWidth) {
    target_->DrawLine(from, to, brush(color), strokeWidth);
}

void Painter::text(std::wstring_view text, const D2D1_RECT_F& rect, Font font, const D2D1_COLOR_F& color,
                   Align align) {
    IDWriteTextFormat* format = Graphics::instance().format(font);
    format->SetTextAlignment(align == Align::Center  ? DWRITE_TEXT_ALIGNMENT_CENTER
                             : align == Align::Right ? DWRITE_TEXT_ALIGNMENT_TRAILING
                                                     : DWRITE_TEXT_ALIGNMENT_LEADING);
    target_->DrawText(text.data(), UINT32(text.size()), format, rect, brush(color), D2D1_DRAW_TEXT_OPTIONS_CLIP);
}

void Painter::icon(wchar_t glyph, D2D1_POINT_2F center, Font font, const D2D1_COLOR_F& color) {
    text(std::wstring_view(&glyph, 1), squareAround(center, 24), font, color, Align::Center);
}

float Painter::textWidth(std::wstring_view text, Font font) const {
    ComPtr<IDWriteTextLayout> layout;
    Graphics::instance().dwrite()->CreateTextLayout(text.data(), UINT32(text.size()), Graphics::instance().format(font),
                                                    10000, 100, &layout);
    DWRITE_TEXT_METRICS metrics{};
    layout->GetMetrics(&metrics);
    return metrics.widthIncludingTrailingWhitespace;
}
