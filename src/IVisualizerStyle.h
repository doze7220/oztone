#pragma once
#include <windows.h>
#include <d2d1.h>
#include <d2d1_1.h>
#include <vector>
#include <string>

/**
 * @brief ビジュアライザ描画スタイルの共通インターフェース
 */
class IVisualizerStyle {
public:
    virtual ~IVisualizerStyle() = default;

    /**
     * @brief 初期化処理。描画用ブラシ等のリソースを作成する
     * @param context D2D描画コンテキスト
     * @return 成功した場合はtrue
     */
    virtual bool Initialize(ID2D1DeviceContext* context) = 0;

    /**
     * @brief リソースの解放処理
     */
    virtual void ReleaseResources() = 0;

    /**
     * @brief ビジュアライザを描画する
     * @param context D2D描画コンテキスト
     * @param spectrum スペクトルデータ
     * @param drawRect 描画領域の矩形
     * @param trackTitle 曲名
     * @param trackArtist アーティスト名
     */
    virtual void Draw(ID2D1DeviceContext* context, const std::vector<float>& spectrum, D2D1_RECT_F drawRect, const std::wstring& trackTitle, const std::wstring& trackArtist) = 0;
};
