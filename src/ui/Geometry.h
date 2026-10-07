#pragma once
#include <algorithm>

// Points, rectangles and colors shared by every platform, plus small helpers (all in DIPs).

struct PointF {
    float x = 0;
    float y = 0;
};

struct RectF {
    float left = 0;
    float top = 0;
    float right = 0;
    float bottom = 0;
};

struct Color {
    float r = 0;
    float g = 0;
    float b = 0;
    float a = 1;
};

inline float width(const RectF& r) { return r.right - r.left; }
inline float height(const RectF& r) { return r.bottom - r.top; }
inline float centerY(const RectF& r) { return (r.top + r.bottom) / 2; }
inline PointF center(const RectF& r) { return {(r.left + r.right) / 2, centerY(r)}; }

inline RectF inset(const RectF& r, float dx, float dy) {
    return {r.left + dx, r.top + dy, r.right - dx, r.bottom - dy};
}

inline RectF squareAround(PointF c, float size) {
    return {c.x - size / 2, c.y - size / 2, c.x + size / 2, c.y + size / 2};
}

inline bool contains(const RectF& r, PointF p) {
    return p.x >= r.left && p.x < r.right && p.y >= r.top && p.y < r.bottom;
}

inline RectF intersect(const RectF& a, const RectF& b) {
    return {std::max(a.left, b.left), std::max(a.top, b.top), std::min(a.right, b.right), std::min(a.bottom, b.bottom)};
}

inline bool isEmpty(const RectF& r) { return r.right <= r.left || r.bottom <= r.top; }
