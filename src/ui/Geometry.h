#pragma once
#include <d2d1.h>

#include <algorithm>

// Small helpers for rectangles and points (all in DIPs).

inline float width(const D2D1_RECT_F& r) { return r.right - r.left; }
inline float height(const D2D1_RECT_F& r) { return r.bottom - r.top; }
inline float centerY(const D2D1_RECT_F& r) { return (r.top + r.bottom) / 2; }
inline D2D1_POINT_2F center(const D2D1_RECT_F& r) { return {(r.left + r.right) / 2, centerY(r)}; }

inline D2D1_RECT_F inset(const D2D1_RECT_F& r, float dx, float dy) {
    return {r.left + dx, r.top + dy, r.right - dx, r.bottom - dy};
}

inline D2D1_RECT_F squareAround(D2D1_POINT_2F c, float size) {
    return {c.x - size / 2, c.y - size / 2, c.x + size / 2, c.y + size / 2};
}

inline bool contains(const D2D1_RECT_F& r, D2D1_POINT_2F p) {
    return p.x >= r.left && p.x < r.right && p.y >= r.top && p.y < r.bottom;
}

inline D2D1_RECT_F intersect(const D2D1_RECT_F& a, const D2D1_RECT_F& b) {
    return {std::max(a.left, b.left), std::max(a.top, b.top), std::min(a.right, b.right), std::min(a.bottom, b.bottom)};
}

inline bool isEmpty(const D2D1_RECT_F& r) { return r.right <= r.left || r.bottom <= r.top; }
