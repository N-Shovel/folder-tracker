#pragma once
#include <d2d1.h>
#include <wrl/client.h>

#include <vector>

#include "ui/Painter.h"

// ui/Painter.h done with Direct2D and DirectWrite.
class D2DPainter final : public Painter {
public:
    explicit D2DPainter(ID2D1RenderTarget* target);

    void fillRect(const RectF& rect, const Color& color) override;
    void fillRoundRect(const RectF& rect, float radius, const Color& color) override;
    void strokeRoundRect(const RectF& rect, float radius, const Color& color, float strokeWidth = 1) override;
    void fillCircle(PointF center, float radius, const Color& color) override;
    void fillCircle(PointF center, float radius, const LinearGradient& gradient) override;
    void strokeCircle(PointF center, float radius, const Color& color, float strokeWidth = 1) override;
    void line(PointF from, PointF to, const Color& color, float strokeWidth = 1) override;
    void line(PointF from, PointF to, const LinearGradient& gradient, float strokeWidth = 1) override;
    void strokePath(const Path& path, const LinearGradient& gradient, float strokeWidth = 1) override;

    void text(std::wstring_view text, const RectF& rect, Font font, const Color& color,
              Align align = Align::Left) override;
    void icon(Icon icon, PointF center, Font font, const Color& color) override;
    float textWidth(std::wstring_view text, Font font) const override;

    void pushCircleLayer(PointF center, float radius, float opacity) override;
    void popLayer() override;
    void pushClip(const RectF& rect) override;
    void popClip() override;

    void setTransform(PointF center, float scale, float degrees = 0) override;
    void resetTransform() override;

private:
    ID2D1Brush* brush(const Color& color);
    Microsoft::WRL::ComPtr<ID2D1Brush> gradientBrush(const LinearGradient& gradient);

    ID2D1RenderTarget* target_;
    Microsoft::WRL::ComPtr<ID2D1SolidColorBrush> brush_;
    std::vector<Microsoft::WRL::ComPtr<ID2D1Layer>> layers_;
};
