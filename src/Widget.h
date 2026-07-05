#pragma once
#include <windows.h>
#include <d2d1.h>
#include <d2d1_1.h>
#include <dwrite.h>
#include <dwrite_1.h>
#include <wincodec.h>
#include <string>
#include <vector>
#include <optional>
#include "PlaylistManager.h"

#include "Window.h"

class ConfigManager;

#include "WidgetContext.h"

/**
 * @brief 各UI要素（Widget）の基底インターフェース
 */
class IWidget {
public:
    virtual ~IWidget() = default;

    /**
     * @brief デバイス依存リソースの作成を行う
     */
    virtual void CreateResources(ID2D1DeviceContext* context, IWICImagingFactory* wicFactory, IDWriteFactory* dwriteFactory, const ConfigManager* config) = 0;

    /**
     * @brief デバイス依存リソースを解放する
     */
    virtual void ReleaseResources() = 0;

    /**
     * @brief アニメーションや状態の更新を行う（毎フレーム呼ばれる）
     */
    virtual void UpdateAnimation(const WidgetContext& ctx) = 0;

    /**
     * @brief レイアウトの更新を行う
     */
    virtual void UpdateLayout(const WidgetContext& ctx, const ConfigManager* config) = 0;
    
    // 描画
    virtual void Draw(ID2D1DeviceContext* context, const WidgetContext& ctx, const ConfigManager* config) = 0;

    // スクロール操作の受け取り（デフォルトは何もしない）
    virtual void AddScroll(float delta) {}
    virtual float GetScrollY() const { return 0.0f; }
};
