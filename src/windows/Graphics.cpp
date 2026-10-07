#include "windows/Graphics.h"

#include "ui/Theme.h"

using Microsoft::WRL::ComPtr;

namespace {

constexpr wchar_t kFontFamily[] = L"Segoe UI";

// Windows 11 has "Segoe Fluent Icons"; Windows 10 has the older "Segoe MDL2 Assets".
const wchar_t* findIconFamily(IDWriteFactory* factory) {
    ComPtr<IDWriteFontCollection> fonts;
    factory->GetSystemFontCollection(&fonts);
    UINT32 index = 0;
    BOOL exists = FALSE;
    fonts->FindFamilyName(L"Segoe Fluent Icons", &index, &exists);
    return exists ? L"Segoe Fluent Icons" : L"Segoe MDL2 Assets";
}

}  // namespace

Graphics& Graphics::instance() {
    static Graphics graphics;
    return graphics;
}

Graphics::Graphics() {
    D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, d2d_.GetAddressOf());
    DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory),
                        reinterpret_cast<IUnknown**>(dwrite_.GetAddressOf()));
    iconFamily_ = findIconFamily(dwrite_.Get());
}

IDWriteTextFormat* Graphics::format(Font font) {
    auto& slot = formats_[static_cast<size_t>(font)];
    if (!slot) slot = createFormat(font);
    return slot.Get();
}

ComPtr<IDWriteTextFormat> Graphics::createFormat(Font font) {
    struct Spec {
        const wchar_t* family;
        float size;
        DWRITE_FONT_WEIGHT weight;
    };
    const Spec spec = [&]() -> Spec {
        switch (font) {
            case Font::Title: return {kFontFamily, theme::kTitleSize, DWRITE_FONT_WEIGHT_SEMI_BOLD};
            case Font::Body: return {kFontFamily, theme::kBodySize, DWRITE_FONT_WEIGHT_NORMAL};
            case Font::BodyBold: return {kFontFamily, theme::kBodySize, DWRITE_FONT_WEIGHT_SEMI_BOLD};
            case Font::Small: return {kFontFamily, theme::kSmallSize, DWRITE_FONT_WEIGHT_NORMAL};
            case Font::SmallBold: return {kFontFamily, theme::kSmallSize, DWRITE_FONT_WEIGHT_SEMI_BOLD};
            case Font::Icon: return {iconFamily_, theme::kIconSize, DWRITE_FONT_WEIGHT_NORMAL};
            case Font::IconSmall: return {iconFamily_, theme::kIconSmallSize, DWRITE_FONT_WEIGHT_NORMAL};
            case Font::IconTiny: return {iconFamily_, theme::kIconTinySize, DWRITE_FONT_WEIGHT_NORMAL};
        }
        return {kFontFamily, theme::kBodySize, DWRITE_FONT_WEIGHT_NORMAL};
    }();

    ComPtr<IDWriteTextFormat> format;
    dwrite_->CreateTextFormat(spec.family, nullptr, spec.weight, DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL,
                              spec.size, L"en-us", &format);

    // Single line, vertically centered, "..." when too long.
    format->SetWordWrapping(DWRITE_WORD_WRAPPING_NO_WRAP);
    format->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);
    ComPtr<IDWriteInlineObject> ellipsis;
    dwrite_->CreateEllipsisTrimmingSign(format.Get(), &ellipsis);
    const DWRITE_TRIMMING trimming{DWRITE_TRIMMING_GRANULARITY_CHARACTER, 0, 0};
    format->SetTrimming(&trimming, ellipsis.Get());
    return format;
}
