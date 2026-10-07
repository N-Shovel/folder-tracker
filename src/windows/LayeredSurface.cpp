#include "windows/LayeredSurface.h"

#include "windows/Graphics.h"

LayeredSurface::~LayeredSurface() {
    releaseBitmap();
    if (dc_) DeleteDC(dc_);
}

void LayeredSurface::releaseBitmap() {
    if (!bitmap_) return;
    SelectObject(dc_, previousBitmap_);
    DeleteObject(bitmap_);
    bitmap_ = nullptr;
}

bool LayeredSurface::resize(SIZE pixels, float dpi) {
    releaseBitmap();
    size_ = pixels;
    if (!dc_) dc_ = CreateCompatibleDC(nullptr);

    BITMAPINFO info{};
    info.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    info.bmiHeader.biWidth = pixels.cx;
    info.bmiHeader.biHeight = -pixels.cy;  // top-down
    info.bmiHeader.biPlanes = 1;
    info.bmiHeader.biBitCount = 32;
    info.bmiHeader.biCompression = BI_RGB;
    void* bits = nullptr;
    bitmap_ = CreateDIBSection(dc_, &info, DIB_RGB_COLORS, &bits, nullptr, 0);
    if (!bitmap_) return false;
    previousBitmap_ = SelectObject(dc_, bitmap_);

    if (!target_) {
        // Software rendering keeps memory use low; this window is small enough not to need the GPU.
        const auto properties = D2D1::RenderTargetProperties(
            D2D1_RENDER_TARGET_TYPE_SOFTWARE,
            D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED));
        if (FAILED(Graphics::instance().d2d()->CreateDCRenderTarget(&properties, &target_))) return false;
        target_->SetTextAntialiasMode(D2D1_TEXT_ANTIALIAS_MODE_GRAYSCALE);  // ClearType needs an opaque background
    }
    target_->SetDpi(dpi, dpi);  // lets the rest of the code draw in DIPs

    const RECT bounds{0, 0, pixels.cx, pixels.cy};
    return SUCCEEDED(target_->BindDC(dc_, &bounds));
}

void LayeredSurface::present(HWND window, POINT topLeft) const {
    POINT source{0, 0};
    SIZE size = size_;
    BLENDFUNCTION blend{AC_SRC_OVER, 0, 255, AC_SRC_ALPHA};
    UpdateLayeredWindow(window, nullptr, &topLeft, &size, dc_, &source, 0, &blend, ULW_ALPHA);
}
