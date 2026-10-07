#include "linux/CairoPainter.h"

#include <gtk/gtk.h>
#include <pango/pangocairo.h>

#include <array>
#include <cmath>
#include <map>
#include <string>
#include <utility>

#include "core/Text.h"
#include "ui/Theme.h"

namespace {

constexpr double kPi = 3.14159265358979323846;

void setColor(cairo_t* cr, const Color& c) { cairo_set_source_rgba(cr, c.r, c.g, c.b, c.a); }

cairo_pattern_t* createGradient(const LinearGradient& g) {
    cairo_pattern_t* pattern = cairo_pattern_create_linear(g.start.x, g.start.y, g.end.x, g.end.y);
    cairo_pattern_add_color_stop_rgba(pattern, 0, g.from.r, g.from.g, g.from.b, g.from.a);
    cairo_pattern_add_color_stop_rgba(pattern, 1, g.to.r, g.to.g, g.to.b, g.to.a);
    return pattern;
}

void strokeWithGradient(cairo_t* cr, const LinearGradient& gradient, float strokeWidth) {
    cairo_pattern_t* pattern = createGradient(gradient);
    cairo_set_source(cr, pattern);
    cairo_set_line_width(cr, strokeWidth);
    cairo_stroke(cr);
    cairo_pattern_destroy(pattern);
}

// ---------------------------------------------------------------------------------------------
// Fonts

// The desktop's own interface font (Ubuntu, Cantarell, Noto Sans, ...), like Segoe UI on Windows.
std::string fontFamily() {
    std::string family = "Sans";
    if (GtkSettings* settings = gtk_settings_get_default()) {
        gchar* name = nullptr;
        g_object_get(settings, "gtk-font-name", &name, nullptr);
        if (name) {
            PangoFontDescription* description = pango_font_description_from_string(name);
            if (const char* f = pango_font_description_get_family(description)) family = f;
            pango_font_description_free(description);
            g_free(name);
        }
    }
    return family;
}

const PangoFontDescription* fontFor(Font font) {
    static std::array<PangoFontDescription*, 8> fonts{};
    auto& slot = fonts[static_cast<size_t>(font)];
    if (slot) return slot;

    const auto [size, bold] = [&]() -> std::pair<float, bool> {
        switch (font) {
            case Font::Title: return {theme::kTitleSize, true};
            case Font::Body: return {theme::kBodySize, false};
            case Font::BodyBold: return {theme::kBodySize, true};
            case Font::Small: return {theme::kSmallSize, false};
            case Font::SmallBold: return {theme::kSmallSize, true};
            case Font::Icon: return {theme::kIconSize, false};
            case Font::IconSmall: return {theme::kIconSmallSize, false};
            case Font::IconTiny: return {theme::kIconTinySize, false};
        }
        return {theme::kBodySize, false};
    }();

    slot = pango_font_description_new();
    pango_font_description_set_family(slot, fontFamily().c_str());
    pango_font_description_set_absolute_size(slot, size * PANGO_SCALE);  // in DIPs, like the Windows version
    pango_font_description_set_weight(slot, bold ? PANGO_WEIGHT_SEMIBOLD : PANGO_WEIGHT_NORMAL);
    return slot;
}

float iconSize(Font font) {
    // Symbolic icons are drawn on a 16px grid with some padding, so they need a little more room
    // than the Windows icon font glyphs of the same size.
    switch (font) {
        case Font::IconSmall: return theme::kIconSmallSize + 2;
        case Font::IconTiny: return theme::kIconTinySize + 2;
        default: return theme::kIconSize + 2;
    }
}

// ---------------------------------------------------------------------------------------------
// Icons

// Names from the freedesktop / Adwaita symbolic icon set. The first one the theme has wins.
std::vector<const char*> iconNames(Icon icon) {
    switch (icon) {
        case Icon::Folder: return {"folder-symbolic"};
        case Icon::FolderOpen: return {"folder-open-symbolic", "folder-symbolic"};
        case Icon::ChevronRight: return {"pan-end-symbolic", "go-next-symbolic"};
        case Icon::ChevronDown: return {"pan-down-symbolic", "go-down-symbolic"};
        case Icon::Refresh: return {"view-refresh-symbolic"};
        case Icon::Delete: return {"user-trash-symbolic", "edit-delete-symbolic"};
        case Icon::OpenLink: return {"adw-external-link-symbolic", "external-link-symbolic", "web-browser-symbolic"};
        case Icon::AddFolder: return {"folder-new-symbolic", "list-add-symbolic"};
        case Icon::Search: return {"edit-find-symbolic", "system-search-symbolic"};
        case Icon::Settings: return {"emblem-system-symbolic", "preferences-system-symbolic"};
        case Icon::GitHub: return {"weather-overcast-symbolic", "network-server-symbolic"};
        case Icon::Local: return {"drive-harddisk-symbolic"};
        case Icon::Untracked: return {"dialog-warning-symbolic"};
    }
    return {};
}

// The icon's shape as an alpha mask, `pixels` wide. Cached; null if the theme has no such icon.
cairo_surface_t* iconMask(Icon icon, int pixels) {
    static std::map<std::pair<Icon, int>, cairo_surface_t*> cache;
    const auto key = std::make_pair(icon, pixels);
    if (const auto found = cache.find(key); found != cache.end()) return found->second;

    cairo_surface_t* mask = nullptr;
    std::vector<const char*> names = iconNames(icon);
    names.push_back(nullptr);
    if (GtkIconInfo* info = gtk_icon_theme_choose_icon(gtk_icon_theme_get_default(), names.data(), pixels,
                                                       GTK_ICON_LOOKUP_FORCE_SIZE)) {
        const GdkRGBA black{0, 0, 0, 1};
        if (GdkPixbuf* pixbuf = gtk_icon_info_load_symbolic(info, &black, nullptr, nullptr, nullptr, nullptr, nullptr)) {
            mask = gdk_cairo_surface_create_from_pixbuf(pixbuf, 1, nullptr);
            g_object_unref(pixbuf);
        }
        g_object_unref(info);
    }
    cache[key] = mask;
    return mask;
}

}  // namespace

// ---------------------------------------------------------------------------------------------
// Painter

CairoPainter::CairoPainter(cairo_t* target) : cr_(target), layout_(pango_cairo_create_layout(target)) {
    cairo_get_matrix(cr_, &baseMatrix_);

    // Grayscale antialiasing: subpixel (colored) text needs an opaque background.
    cairo_font_options_t* options = cairo_font_options_create();
    cairo_font_options_set_antialias(options, CAIRO_ANTIALIAS_GRAY);
    cairo_font_options_set_hint_style(options, CAIRO_HINT_STYLE_SLIGHT);
    pango_cairo_context_set_font_options(pango_layout_get_context(layout_), options);
    cairo_font_options_destroy(options);
    pango_layout_context_changed(layout_);

    pango_layout_set_single_paragraph_mode(layout_, TRUE);
}

CairoPainter::~CairoPainter() { g_object_unref(layout_); }

float CairoPainter::deviceScale() const {
    double x = 1, y = 1;
    cairo_surface_get_device_scale(cairo_get_target(cr_), &x, &y);
    return float(x);
}

void CairoPainter::roundRectPath(const RectF& rect, float radius) {
    const float r = std::min(radius, std::min(width(rect), height(rect)) / 2);
    cairo_new_sub_path(cr_);
    cairo_arc(cr_, rect.right - r, rect.top + r, r, -kPi / 2, 0);
    cairo_arc(cr_, rect.right - r, rect.bottom - r, r, 0, kPi / 2);
    cairo_arc(cr_, rect.left + r, rect.bottom - r, r, kPi / 2, kPi);
    cairo_arc(cr_, rect.left + r, rect.top + r, r, kPi, 3 * kPi / 2);
    cairo_close_path(cr_);
}

void CairoPainter::fillRect(const RectF& rect, const Color& color) {
    cairo_rectangle(cr_, rect.left, rect.top, width(rect), height(rect));
    setColor(cr_, color);
    cairo_fill(cr_);
}

void CairoPainter::fillRoundRect(const RectF& rect, float radius, const Color& color) {
    roundRectPath(rect, radius);
    setColor(cr_, color);
    cairo_fill(cr_);
}

void CairoPainter::strokeRoundRect(const RectF& rect, float radius, const Color& color, float strokeWidth) {
    roundRectPath(rect, radius);
    setColor(cr_, color);
    cairo_set_line_width(cr_, strokeWidth);
    cairo_stroke(cr_);
}

void CairoPainter::fillCircle(PointF center, float radius, const Color& color) {
    cairo_new_sub_path(cr_);
    cairo_arc(cr_, center.x, center.y, radius, 0, 2 * kPi);
    setColor(cr_, color);
    cairo_fill(cr_);
}

void CairoPainter::fillCircle(PointF center, float radius, const LinearGradient& gradient) {
    cairo_new_sub_path(cr_);
    cairo_arc(cr_, center.x, center.y, radius, 0, 2 * kPi);
    cairo_pattern_t* pattern = createGradient(gradient);
    cairo_set_source(cr_, pattern);
    cairo_fill(cr_);
    cairo_pattern_destroy(pattern);
}

void CairoPainter::strokeCircle(PointF center, float radius, const Color& color, float strokeWidth) {
    cairo_new_sub_path(cr_);
    cairo_arc(cr_, center.x, center.y, radius, 0, 2 * kPi);
    setColor(cr_, color);
    cairo_set_line_width(cr_, strokeWidth);
    cairo_stroke(cr_);
}

void CairoPainter::line(PointF from, PointF to, const Color& color, float strokeWidth) {
    cairo_move_to(cr_, from.x, from.y);
    cairo_line_to(cr_, to.x, to.y);
    setColor(cr_, color);
    cairo_set_line_width(cr_, strokeWidth);
    cairo_stroke(cr_);
}

void CairoPainter::line(PointF from, PointF to, const LinearGradient& gradient, float strokeWidth) {
    cairo_move_to(cr_, from.x, from.y);
    cairo_line_to(cr_, to.x, to.y);
    strokeWithGradient(cr_, gradient, strokeWidth);
}

void CairoPainter::strokePath(const Path& path, const LinearGradient& gradient, float strokeWidth) {
    cairo_move_to(cr_, path.start.x, path.start.y);
    PointF from = path.start;
    for (const Path::Segment& segment : path.segments) {
        const PointF to = segment.to;
        if (segment.arcRadius <= 0) {
            cairo_line_to(cr_, to.x, to.y);
        } else {
            // Cairo draws arcs around a center; find the center of the short arc from `from` to `to`.
            const float dx = to.x - from.x, dy = to.y - from.y;
            const float distance = std::hypot(dx, dy);
            const float r = std::max(segment.arcRadius, distance / 2);
            const float h = std::sqrt(std::max(0.0f, r * r - distance * distance / 4));
            const float side = segment.clockwise ? 1.0f : -1.0f;  // clockwise: center is to the right
            const float cx = (from.x + to.x) / 2 - side * h * dy / distance;
            const float cy = (from.y + to.y) / 2 + side * h * dx / distance;
            const double start = std::atan2(from.y - cy, from.x - cx);
            const double end = std::atan2(to.y - cy, to.x - cx);
            if (segment.clockwise) cairo_arc(cr_, cx, cy, r, start, end);
            else cairo_arc_negative(cr_, cx, cy, r, start, end);
        }
        from = to;
    }
    strokeWithGradient(cr_, gradient, strokeWidth);
}

void CairoPainter::text(std::wstring_view text, const RectF& rect, Font font, const Color& color, Align align) {
    if (width(rect) <= 0 || text.empty()) return;
    const std::string utf8 = narrow(text);
    pango_layout_set_text(layout_, utf8.c_str(), int(utf8.size()));
    pango_layout_set_font_description(layout_, fontFor(font));
    pango_layout_set_width(layout_, int(width(rect) * PANGO_SCALE));
    pango_layout_set_ellipsize(layout_, PANGO_ELLIPSIZE_END);  // "..." when too long
    pango_layout_set_alignment(layout_, align == Align::Center  ? PANGO_ALIGN_CENTER
                                        : align == Align::Right ? PANGO_ALIGN_RIGHT
                                                                : PANGO_ALIGN_LEFT);
    pango_cairo_update_layout(cr_, layout_);

    int textWidth = 0, textHeight = 0;
    pango_layout_get_pixel_size(layout_, &textWidth, &textHeight);

    cairo_save(cr_);
    cairo_rectangle(cr_, rect.left, rect.top, width(rect), height(rect));
    cairo_clip(cr_);
    cairo_move_to(cr_, rect.left, centerY(rect) - textHeight / 2.0);  // single line, vertically centered
    setColor(cr_, color);
    pango_cairo_show_layout(cr_, layout_);
    cairo_restore(cr_);
}

void CairoPainter::icon(Icon icon, PointF center, Font font, const Color& color) {
    const float size = iconSize(font);
    const int pixels = std::max(1, int(std::lround(size * deviceScale())));
    cairo_surface_t* mask = iconMask(icon, pixels);
    if (!mask) return;

    cairo_save(cr_);
    cairo_translate(cr_, std::round(center.x - size / 2), std::round(center.y - size / 2));
    cairo_scale(cr_, size / pixels, size / pixels);
    setColor(cr_, color);
    cairo_mask_surface(cr_, mask, 0, 0);
    cairo_restore(cr_);
}

float CairoPainter::textWidth(std::wstring_view text, Font font) const {
    const std::string utf8 = narrow(text);
    pango_layout_set_text(layout_, utf8.c_str(), int(utf8.size()));
    pango_layout_set_font_description(layout_, fontFor(font));
    pango_layout_set_width(layout_, -1);
    pango_layout_set_ellipsize(layout_, PANGO_ELLIPSIZE_NONE);
    PangoRectangle logical;
    pango_layout_get_extents(layout_, nullptr, &logical);
    return float(logical.width) / PANGO_SCALE;
}

void CairoPainter::pushCircleLayer(PointF center, float radius, float opacity) {
    cairo_save(cr_);
    cairo_new_sub_path(cr_);
    cairo_arc(cr_, center.x, center.y, radius, 0, 2 * kPi);
    cairo_clip(cr_);
    cairo_push_group(cr_);
    layerOpacity_.push_back(opacity);
}

void CairoPainter::popLayer() {
    cairo_pop_group_to_source(cr_);
    cairo_paint_with_alpha(cr_, layerOpacity_.back());
    layerOpacity_.pop_back();
    cairo_restore(cr_);
}

void CairoPainter::pushClip(const RectF& rect) {
    cairo_save(cr_);
    cairo_rectangle(cr_, rect.left, rect.top, width(rect), height(rect));
    cairo_clip(cr_);
}

void CairoPainter::popClip() { cairo_restore(cr_); }

void CairoPainter::setTransform(PointF center, float scale, float degrees) {
    resetTransform();
    cairo_translate(cr_, center.x, center.y);
    cairo_rotate(cr_, degrees * kPi / 180);
    cairo_scale(cr_, scale, scale);
    cairo_translate(cr_, -center.x, -center.y);
}

void CairoPainter::resetTransform() {
    cairo_set_matrix(cr_, &baseMatrix_);
}
