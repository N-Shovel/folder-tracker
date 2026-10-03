#pragma once
#include <windows.h>
#include <d2d1.h>
#include <wrl/client.h>

// The see-through image the window shows. We draw into it with Direct2D, then hand it to
// Windows with UpdateLayeredWindow. Fully transparent pixels let clicks pass through to
// whatever is behind the window, which is what makes the empty area around the bubble "not there".
class LayeredSurface {
public:
    LayeredSurface() = default;
    ~LayeredSurface();

    LayeredSurface(const LayeredSurface&) = delete;
    LayeredSurface& operator=(const LayeredSurface&) = delete;

    bool resize(SIZE pixels, float dpi);
    ID2D1DCRenderTarget* target() const { return target_.Get(); }
    void present(HWND window, POINT topLeft) const;

private:
    void releaseBitmap();

    HDC dc_ = nullptr;
    HBITMAP bitmap_ = nullptr;
    HGDIOBJ previousBitmap_ = nullptr;
    SIZE size_{};
    Microsoft::WRL::ComPtr<ID2D1DCRenderTarget> target_;
};
