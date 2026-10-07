#include "linux/AppIcon.h"

#include <cairo.h>
#include <glib.h>
#include <glib/gstdio.h>

#include "ui/BubbleArt.h"
#include "linux/CairoPainter.h"
#include "ui/Theme.h"

namespace {

constexpr int kPixels = 128;

void drawIcon(const std::string& path) {
    cairo_surface_t* surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, kPixels, kPixels);
    cairo_t* cr = cairo_create(surface);

    // Fit the bubble and its shadow (about 10 DIPs past the edge) into the square.
    const float extent = theme::kBubbleSize / 2 + 10;
    const float scale = (kPixels / 2.0f) / extent;
    cairo_scale(cr, scale, scale);
    {
        CairoPainter painter(cr);
        drawBubble(painter, {extent, extent}, 0, false, false);
    }
    cairo_destroy(cr);
    cairo_surface_write_to_png(surface, path.c_str());
    cairo_surface_destroy(surface);
}

}  // namespace

const std::string& appIconFolder() {
    static const std::string folder = [] {
        gchar* path = g_build_filename(g_get_user_data_dir(), "folder-tracker", nullptr);
        g_mkdir_with_parents(path, 0700);
        std::string result = path;
        g_free(path);
        return result;
    }();
    return folder;
}

const std::string& appIconPath() {
    static const std::string path = [] {
        std::string file = appIconFolder() + "/" + kAppIconName + ".png";
        drawIcon(file);  // redrawn each launch, so it always matches this version of the app
        return file;
    }();
    return path;
}
