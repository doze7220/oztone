#pragma once
#include <windows.h>
#include <atomic>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>
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

    /**
     * @brief 強制的に1フレーム描画を行う（リサイズ中の追従用）
     */
    void ForceRender();

    /**
     * @brief 起動直後のコマンドライン引数を処理する
     * @param argc 引数の数
     * @param argv 引数の文字列配列
     */
    void ProcessCommandLineArgs(int argc, LPWSTR* argv);

    /**
     * @brief プレイリストをクリアし、UIを初期状態に戻す
     */
    void ClearPlaylist();

    /**
     * @brief プレイリストを切り替える
     * @param filepath 新しいプレイリストのパス
     */
    void SwitchPlaylist(const std::wstring& filepath);

    /**
     * @brief 新規プレイリストを作成する
     */
    void CreateNewPlaylist();

private:
    /**
     * @brief 特定のファイルパスに対するメタデータの比較・更新・保存処理を行う
     * @param filepath 対象ファイルのパス
     */
    void UpdateTrackMetadataIfNeeded(const std::wstring& filepath);

    /**
     * @brief 次の曲のタグ情報と画像を先読みする
     */
    void PrefetchNextTrack();

    /**
     * @brief ファイルがドロップされた時の処理
     * @param paths ドロップされたファイルのパスのリスト
     */
    void OnFilesDropped(const std::vector<std::wstring>& paths);

    /**
     * @brief バックグラウンドでメタデータを解析するスレッドの関数
     */
    void ParseThreadFunc();

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

    // バックグラウンド解析データ
    std::thread m_parseThread;
    std::mutex m_parseMutex;
    std::condition_variable m_parseCV;
    std::queue<std::wstring> m_parseQueue;
    std::atomic<bool> m_parseThreadRunning{false};

    bool m_isPlaylistListViewMode = false;
    ULONGLONG m_lastConfigCheckTime = 0;
};
