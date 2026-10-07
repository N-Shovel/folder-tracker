#pragma once
#include <cairo.h>
#include <pango/pango.h>

#include <vector>

#include "ui/Painter.h"

// ui/Painter.h done with Cairo for shapes, Pango for text, and the desktop's symbolic icon theme for icons.
class CairoPainter final : public Painter {
public:
    explicit CairoPainter(cairo_t* target);
    ~CairoPainter() override;

    CairoPainter(const CairoPainter&) = delete;
    CairoPainter& operator=(const CairoPainter&) = delete;

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
    void roundRectPath(const RectF& rect, float radius);
    float deviceScale() const;

    cairo_t* cr_;
    PangoLayout* layout_;
    cairo_matrix_t baseMatrix_;  // the transform the caller set up; setTransform() works on top of it
    std::vector<float> layerOpacity_;
};
