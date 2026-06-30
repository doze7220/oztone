#pragma once
#include <windows.h>
#include <atomic>
#include <thread>
#include "Window.h"
#include "ConfigManager.h"
#include "Renderer.h"
#include "AudioPlayer.h"
#include "TagManager.h"
#include "PlaylistManager.h"

/**
 * @brief アプリケーション全体のライフサイクルとメインループを管理するクラス
 */
class Application {
public:
    Application();
    ~Application();

    /**
     * @brief アプリケーションの初期化
     * @param hInstance インスタンスハンドル
     * @param nCmdShow 表示状態
     * @return 成功ならtrue
     */
    bool Initialize(HINSTANCE hInstance, int nCmdShow);

    /**
     * @brief アプリケーションのメインループを実行
     */
    void Run();

private:
    /**
     * @brief 次の曲のタグ情報と画像を先読みする
     */
    void PrefetchNextTrack();

    ConfigManager m_config;
    Window m_window;
    Renderer m_renderer;
    AudioPlayer m_audioPlayer;
    TagManager m_tagManager;
    PlaylistManager m_playlistManager;

    // 先読みデータ
    std::atomic<bool> m_isPrefetchReady{false};
    std::thread m_prefetchThread;
    
    std::wstring m_prefetchedTitle;
    std::wstring m_prefetchedArtist;
    Microsoft::WRL::ComPtr<ID2D1Bitmap> m_prefetchedAlbumArt;
};
