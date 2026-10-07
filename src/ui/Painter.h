#pragma once
#include <string_view>
#include <vector>

#include "ui/Geometry.h"
#include "ui/Icons.h"

enum class Font { Title, Body, BodyBold, Small, SmallBold, Icon, IconSmall, IconTiny };
enum class Align { Left, Center, Right };

// A color that fades from `from` at `start` to `to` at `end`.
struct LinearGradient {
    PointF start;
    PointF end;
    Color from;
    Color to;
};

// An open outline made of straight lines and circular arcs (each arc is the short way round).
struct Path {
    struct Segment {
        PointF to;
        float arcRadius = 0;  // 0 for a straight line
        bool clockwise = false;
    };
    PointF start;
    std::vector<Segment> segments;

    Path& lineTo(PointF to) { segments.push_back({to}); return *this; }
    Path& arcTo(PointF to, float radius, bool clockwise) { segments.push_back({to, radius, clockwise}); return *this; }
};

// Simple drawing commands used by everything that draws. Each system has its own:
// windows/D2DPainter (Direct2D) and linux/CairoPainter (Cairo + Pango).
class Painter {
public:
    virtual ~Painter() = default;

    virtual void fillRect(const RectF& rect, const Color& color) = 0;
    virtual void fillRoundRect(const RectF& rect, float radius, const Color& color) = 0;
    virtual void strokeRoundRect(const RectF& rect, float radius, const Color& color, float strokeWidth = 1) = 0;
    virtual void fillCircle(PointF center, float radius, const Color& color) = 0;
    virtual void fillCircle(PointF center, float radius, const LinearGradient& gradient) = 0;
    virtual void strokeCircle(PointF center, float radius, const Color& color, float strokeWidth = 1) = 0;
    virtual void line(PointF from, PointF to, const Color& color, float strokeWidth = 1) = 0;
    virtual void line(PointF from, PointF to, const LinearGradient& gradient, float strokeWidth = 1) = 0;
    virtual void strokePath(const Path& path, const LinearGradient& gradient, float strokeWidth = 1) = 0;

    virtual void text(std::wstring_view text, const RectF& rect, Font font, const Color& color,
                      Align align = Align::Left) = 0;
    virtual void icon(Icon icon, PointF center, Font font, const Color& color) = 0;
    virtual float textWidth(std::wstring_view text, Font font) const = 0;

    // Everything drawn until popLayer() is clipped to a circle and faded to `opacity`.
    virtual void pushCircleLayer(PointF center, float radius, float opacity) = 0;
    virtual void popLayer() = 0;
    virtual void pushClip(const RectF& rect) = 0;
    virtual void popClip() = 0;

    // Scales by `scale` and rotates by `degrees` (clockwise), both around `center`.
    virtual void setTransform(PointF center, float scale, float degrees = 0) = 0;
    virtual void resetTransform() = 0;
};
