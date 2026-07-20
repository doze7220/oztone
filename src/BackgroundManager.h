#pragma once

#include <Windows.h>
#include <string>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <vector>
#include <wincodec.h>
#include <wrl/client.h>
#include <d2d1.h>

class ConfigManager;

/**
 * @brief 背景レイヤーの描画種別
 */
enum class BackgroundLayerType {
    SolidColor, ///< 単色塗りつぶし
    Image,      ///< 画像描画
    LayerGroupBegin, ///< レイヤーグループ開始
    LayerGroupEnd,   ///< レイヤーグループ終了
};

/**
 * @brief Rendererに渡すための1層分の描画指示データ
 */
struct BackgroundLayer {
    BackgroundLayerType type = BackgroundLayerType::Image;
    Microsoft::WRL::ComPtr<IWICFormatConverter> image;
    float x = 0.0f;
    float y = 0.0f;
    float scale = 1.0f;
    float opacity = 1.0f;
    D2D1_COLOR_F color = { 0.0f, 0.0f, 0.0f, 1.0f };
};

/**
 * @brief 背景画像のロードおよびクロスフェードアニメーションを統括する司令塔
 */
class BackgroundManager {
public:
    BackgroundManager();
    ~BackgroundManager();

    BackgroundManager(const BackgroundManager&) = delete;
    BackgroundManager& operator=(const BackgroundManager&) = delete;

    /**
     * @brief 初期化処理。ワーカー・スレッドを起動する
     */
    void Initialize(const ConfigManager* config);

    /**
     * @brief 終了処理。ワーカー・スレッドを安全に停止・破棄する
     */
    void Uninitialize();

    /**
     * @brief 背景画像のロードを非同期で要求する
     * @param filePath 画像を抽出するファイルパス
     */
    void RequestLoad(const std::wstring& filePath);

    /**
     * @brief アニメーションの状態を更新する
     * @param deltaTime 前回フレームからの経過時間（秒）
     */
    void UpdateAnimation(float deltaTime);

    /**
     * @brief 現在表示中（またはフェード先）の背景アートのスケールを更新する
     */
    void SetArtFramingScale(float scale);

    /**
     * @brief 現在表示中（またはフェード先）の背景アートのスクロールオフセットを更新する
     */
    void SetArtFramingScroll(float offsetX, float offsetY);

    /**
     * @brief 曲ロード時にDBから絶対値を受け取り、NEWレイヤーのみを更新する
     */
    void SetBackgroundFraming(float scale, float offsetX, float offsetY);

    /**
     * @brief 現在のフレーミング状態を取得する（セーブ用など）
     */
    void GetFraming(float& x, float& y, float& scale) const;

    Microsoft::WRL::ComPtr<IWICFormatConverter> GetCurrentWicImage() const { return m_currentWicImage; }
    Microsoft::WRL::ComPtr<IWICFormatConverter> GetOldWicImage() const { return m_oldWicImage; }
    float GetFadeProgress() const { return m_fadeProgress; }

    /**
     * @brief 現在の描画状態に応じたレイヤーリストを取得する
     */
    std::vector<BackgroundLayer> GetLayers() const;

private:
    /**
     * @brief 非同期画像ロードを行うワーカー・スレッドのメインループ
     */
    void WorkerLoop();

private:
    const ConfigManager* m_config = nullptr;

    std::thread m_workerThread;
    std::mutex m_mutex;
    std::condition_variable m_cv;
    bool m_isRunning;

    std::queue<std::wstring> m_requestQueue;
    
    // ワーカーからの受け渡し用
    Microsoft::WRL::ComPtr<IWICFormatConverter> m_nextWicImage;
    bool m_hasNewImage;

    // メインスレッド（描画・状態管理）用
    Microsoft::WRL::ComPtr<IWICFormatConverter> m_currentWicImage;
    Microsoft::WRL::ComPtr<IWICFormatConverter> m_oldWicImage;
    float m_fadeProgress;

    // フレーミング状態（OLD）
    float m_oldScale;
    float m_oldOffsetX;
    float m_oldOffsetY;

    // フレーミング状態（NEW）
    float m_currentScale;
    float m_currentOffsetX;
    float m_currentOffsetY;

    // プレースホルダ画像
    Microsoft::WRL::ComPtr<IWICBitmapSource> m_placeholderWic;
};
