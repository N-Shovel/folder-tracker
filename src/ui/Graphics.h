#pragma once
#include <d2d1.h>
#include <dwrite.h>
#include <wrl/client.h>

#include <array>

enum class Font { Title, Body, BodyBold, Small, SmallBold, Icon, IconSmall, IconTiny };
enum class Align { Left, Center, Right };

// Shared Direct2D / DirectWrite objects (created once, used by every frame).
class Graphics {
public:
    static Graphics& instance();

    ID2D1Factory* d2d() const { return d2d_.Get(); }
    IDWriteFactory* dwrite() const { return dwrite_.Get(); }
    IDWriteTextFormat* format(Font font);

private:
    Graphics();
    Microsoft::WRL::ComPtr<IDWriteTextFormat> createFormat(Font font);

    Microsoft::WRL::ComPtr<ID2D1Factory> d2d_;
    Microsoft::WRL::ComPtr<IDWriteFactory> dwrite_;
    std::array<Microsoft::WRL::ComPtr<IDWriteTextFormat>, 8> formats_;
    const wchar_t* iconFamily_;
};
