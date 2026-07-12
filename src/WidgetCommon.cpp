#include "WidgetCommon.h"

namespace WidgetCommon {

bool LoadBitmapResource(IWICImagingFactory* wicFactory, ID2D1DeviceContext* d2dContext, const std::wstring& filename, int resourceId, ID2D1Bitmap** ppBitmap) {
    HRESULT hr = S_OK;
    Microsoft::WRL::ComPtr<IWICBitmapDecoder> decoder;

    hr = wicFactory->CreateDecoderFromFilename(
        filename.c_str(), nullptr, GENERIC_READ, WICDecodeMetadataCacheOnLoad,
        &decoder);

    if (FAILED(hr)) {
        HMODULE hModule = GetModuleHandle(nullptr);
        HRSRC imageResHandle =
            FindResource(hModule, MAKEINTRESOURCE(resourceId), RT_RCDATA);
        if (!imageResHandle) {
            return false;
        }

        HGLOBAL imageResDataHandle = LoadResource(hModule, imageResHandle);
        if (!imageResDataHandle) {
            return false;
        }

        void* pImageFile = LockResource(imageResDataHandle);
        DWORD imageFileSize = SizeofResource(hModule, imageResHandle);
        if (!pImageFile || imageFileSize == 0) {
            return false;
        }

        Microsoft::WRL::ComPtr<IWICStream> stream;
        hr = wicFactory->CreateStream(&stream);
        if (FAILED(hr)) {
            return false;
        }

        hr = stream->InitializeFromMemory(reinterpret_cast<BYTE*>(pImageFile),
                                          imageFileSize);
        if (FAILED(hr)) {
            return false;
        }

        hr = wicFactory->CreateDecoderFromStream(
            stream.Get(), nullptr, WICDecodeMetadataCacheOnLoad, &decoder);
        if (FAILED(hr)) {
            return false;
        }
    }

    Microsoft::WRL::ComPtr<IWICBitmapFrameDecode> frame;
    hr = decoder->GetFrame(0, &frame);
    if (FAILED(hr)) {
        return false;
    }

    Microsoft::WRL::ComPtr<IWICFormatConverter> converter;
    hr = wicFactory->CreateFormatConverter(&converter);
    if (FAILED(hr)) {
        return false;
    }

    hr = converter->Initialize(frame.Get(), GUID_WICPixelFormat32bppPBGRA,
                               WICBitmapDitherTypeNone, nullptr, 0.0f,
                               WICBitmapPaletteTypeMedianCut);
    if (FAILED(hr)) {
        return false;
    }

    hr = d2dContext->CreateBitmapFromWicBitmap(converter.Get(), nullptr, ppBitmap);
    return SUCCEEDED(hr);
}

void ApplyTextTrimming(IDWriteFactory* dwriteFactory, IDWriteTextFormat* format) {
    if (!format || !dwriteFactory)
        return;
    DWRITE_TRIMMING trimmingOptions = {DWRITE_TRIMMING_GRANULARITY_CHARACTER, 0, 0};
    Microsoft::WRL::ComPtr<IDWriteInlineObject> ellipsis;
    if (SUCCEEDED(dwriteFactory->CreateEllipsisTrimmingSign(format, &ellipsis))) {
        format->SetTrimming(&trimmingOptions, ellipsis.Get());
        format->SetWordWrapping(DWRITE_WORD_WRAPPING_NO_WRAP);
    }
}

D2D1::ColorF HexToColorF(const std::wstring& hex, float alpha) {
    if (hex.length() >= 7 && hex[0] == L'#') {
        int r, g, b;
        if (swscanf_s(hex.c_str() + 1, L"%02x%02x%02x", &r, &g, &b) == 3) {
            return D2D1::ColorF(r / 255.0f, g / 255.0f, b / 255.0f, alpha);
        }
    }
    return D2D1::ColorF(1.0f, 1.0f, 1.0f, alpha);
}

void DrawShadowedTextLayout(
    ID2D1DeviceContext* context,
    IDWriteTextLayout* textLayout,
    ID2D1SolidColorBrush* textBrush,
    ID2D1SolidColorBrush* shadowBrush,
    D2D1_POINT_2F textOrigin,
    D2D1_POINT_2F shadowOrigin,
    float shadowOpacity) {
    
    if (shadowBrush && shadowOpacity > 0.0f) {
        float oldOpacity = shadowBrush->GetOpacity();
        shadowBrush->SetOpacity(shadowOpacity);
        context->DrawTextLayout(shadowOrigin, textLayout, shadowBrush);
        shadowBrush->SetOpacity(oldOpacity);
    }

    if (textBrush) {
        context->DrawTextLayout(textOrigin, textLayout, textBrush);
    }
}

void DrawShadowedText(
    ID2D1DeviceContext* context,
    const std::wstring& text,
    IDWriteTextFormat* textFormat,
    ID2D1SolidColorBrush* textBrush,
    ID2D1SolidColorBrush* shadowBrush,
    const D2D1_RECT_F& textRect,
    const D2D1_RECT_F& shadowRect,
    float shadowOpacity) {
    
    if (shadowBrush && shadowOpacity > 0.0f) {
        float oldOpacity = shadowBrush->GetOpacity();
        shadowBrush->SetOpacity(shadowOpacity);
        context->DrawText(text.c_str(), static_cast<UINT32>(text.length()), textFormat, &shadowRect, shadowBrush);
        shadowBrush->SetOpacity(oldOpacity);
    }

    if (textBrush) {
        context->DrawText(text.c_str(), static_cast<UINT32>(text.length()), textFormat, &textRect, textBrush);
    }
}

} // namespace WidgetCommon
