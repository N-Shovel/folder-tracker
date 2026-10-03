#pragma once
#include <d2d1.h>
#include <wrl/client.h>

#include <string_view>

#include "ui/Graphics.h"

// Simple drawing commands on top of Direct2D, used by everything that draws.
class Painter {
public:
    explicit Painter(ID2D1RenderTarget* target);

    ID2D1RenderTarget* target() const { return target_; }

    void fillRect(const D2D1_RECT_F& rect, const D2D1_COLOR_F& color);
    void fillRoundRect(const D2D1_RECT_F& rect, float radius, const D2D1_COLOR_F& color);
    void strokeRoundRect(const D2D1_RECT_F& rect, float radius, const D2D1_COLOR_F& color, float strokeWidth = 1);
    void fillCircle(D2D1_POINT_2F center, float radius, const D2D1_COLOR_F& color);
    void strokeCircle(D2D1_POINT_2F center, float radius, const D2D1_COLOR_F& color, float strokeWidth = 1);
    void line(D2D1_POINT_2F from, D2D1_POINT_2F to, const D2D1_COLOR_F& color, float strokeWidth = 1);

    void text(std::wstring_view text, const D2D1_RECT_F& rect, Font font, const D2D1_COLOR_F& color,
              Align align = Align::Left);
    void icon(wchar_t glyph, D2D1_POINT_2F center, Font font, const D2D1_COLOR_F& color);
    float textWidth(std::wstring_view text, Font font) const;

private:
    ID2D1SolidColorBrush* brush(const D2D1_COLOR_F& color);

    ID2D1RenderTarget* target_;
    Microsoft::WRL::ComPtr<ID2D1SolidColorBrush> brush_;
};
