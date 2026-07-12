#pragma once
#include <d2d1_1.h>
#include <dwrite.h>
#include <wincodec.h>
#include <wrl/client.h>
#include <string>

namespace WidgetCommon {
    // WICを利用したビットマップリソースの読み込み
    bool LoadBitmapResource(IWICImagingFactory* wicFactory, ID2D1DeviceContext* d2dContext, const std::wstring& filename, int resourceId, ID2D1Bitmap** ppBitmap);

    // テキストフォーマットに対するトリミング（...）と折り返し無効化の適用
    void ApplyTextTrimming(IDWriteFactory* dwriteFactory, IDWriteTextFormat* format);

    // Hexカラーコード文字列 (#RRGGBB) から D2D1::ColorF への変換
    D2D1::ColorF HexToColorF(const std::wstring& hex, float alpha = 1.0f);

    // 影付きテキストレイアウトの描画ユーティリティ
    void DrawShadowedTextLayout(
        ID2D1DeviceContext* context,
        IDWriteTextLayout* textLayout,
        ID2D1SolidColorBrush* textBrush,
        ID2D1SolidColorBrush* shadowBrush,
        D2D1_POINT_2F textOrigin,
        D2D1_POINT_2F shadowOrigin,
        float shadowOpacity);
        
    // 影付きテキスト（直接文字列指定）の描画ユーティリティ
    void DrawShadowedText(
        ID2D1DeviceContext* context,
        const std::wstring& text,
        IDWriteTextFormat* textFormat,
        ID2D1SolidColorBrush* textBrush,
        ID2D1SolidColorBrush* shadowBrush,
        const D2D1_RECT_F& textRect,
        const D2D1_RECT_F& shadowRect,
        float shadowOpacity);
}
