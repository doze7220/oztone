#pragma once
#include <windows.h>
#include <atomic>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <condition_variable>
#include <queue>
#include <optional>
#include "Window.h"
#include "ConfigManager.h"
#include "Renderer.h"
#include "AudioPlayer.h"
#include "TagManager.h"
#include "PlaylistManager.h"
#include "TrackAnalyzer.h"
#include "TrackDatabase.h"
#include "ArtFramingDatabase.h"
#include "ThumbnailDatabase.h"
#include "ThumbCacher.h"

enum class WatchdogState {
    Normal,
    Warning
};

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

    /**
     * @brief メディアコマンドの共通処理を実行する
     * @param cmd APPCOMMAND_MEDIA_... の定数
     */
    void HandleMediaCommand(int cmd);

    /**
     * @brief すべての設定を初期化する
     */
    void ResetAllSettings();

    /**
     * @brief スリープ移行時の処理
     */
    void OnPowerSuspend();

    /**
     * @brief スリープ復帰時の処理
     */
    void OnPowerResume();
private:
    /**
     * @brief Windowからの各種コールバック登録を行う
     */
    void SetupCallbacks();

    /**
     * @brief 現在のトラックのメタデータを自己修復し、再生を開始し、次曲を先読みする
     * @return 再生に成功した場合はtrue、失敗した場合はfalse
     */
    bool PlayCurrentTrack(int relativeDistance = -1);

    /**
     * @brief 特定のファイルパスに対するメタデータの比較・更新・保存処理を行う
     * @param filepath 対象ファイルのパス
     */
    void UpdateTrackMetadataIfNeeded(const std::wstring& filepath);

    /**
     * @brief プレイリスト一覧のメタデータを解析・キャッシュする
     */
    void UpdatePlaylistSummaries();



    /**
     * @brief ファイルがドロップされた時の処理
     * @param paths ドロップされたファイルのパスのリスト
     */
    void OnFilesDropped(const std::vector<std::wstring>& paths);

    void OnPlaylistClicked(int x, int y);
    void OnPlaylistDoubleClicked(int x, int y);
    void OnPlaylistToolbarClicked(int btnIndex);

    ConfigManager m_config;
    Window m_window;
    Renderer m_renderer;
    AudioPlayer m_audioPlayer;
    TagManager m_tagManager;
    PlaylistManager m_playlistManager;

    // バックグラウンド解析データ
    TrackAnalyzer m_trackAnalyzer;
    TrackDatabase m_trackDatabase;
    ArtFramingDatabase m_framingDb;
    std::wstring m_framingDbPath;

    ThumbnailDatabase m_thumbnailDatabase;
    ThumbCacher m_thumbCacher;

    bool m_isPlaylistListViewMode = false;
    ULONGLONG m_lastConfigCheckTime = 0;
    ULONGLONG m_lastPlaylistSnapshotTime = 0;
    std::optional<size_t> m_focusedPlaylistIndex;

    std::vector<PlaylistSummary> m_playlistSummaries;

    bool m_suspendIsPlaying = false;
    float m_suspendPosition = 0.0f;
    bool m_isWaitingForDevice = false;

    WatchdogState m_watchdogState = WatchdogState::Normal;
    ULONGLONG m_lastWatchdogPollTime = 0;
    float m_lastWatchdogPosition = -1.0f;
    ULONGLONG m_watchdogWarningStartTime = 0;
};
