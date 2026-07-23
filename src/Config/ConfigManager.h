#pragma once
#include <windows.h>
#include <string>
#include <vector>
#include <filesystem>

#include "Config_System.h"
#include "Config_Window.h"
#include "Config_Visibility.h"
#include "Config_Playlist.h"
#include "Config_Audio.h"
#include "Config_TrackDrum.h"
#include "Config_UICommonParm.h"
#include "Config_Background.h"
#include "Config_LayoutAppLogo.h"
#include "Config_LayoutLogoMenu.h"
#include "Config_LayoutNowPlaying.h"
#include "Config_LayoutSeekBar.h"
#include "Config_LayoutPlaybackControls.h"
#include "Config_LayoutVolumeControl.h"
#include "Config_LayoutTooltip.h"
#include "Config_LayoutPlaylist.h"
#include "Config_LayoutGlobalHotkeys.h"
#include "Config_LayoutOSD.h"
#include "Config_Visualizer.h"
#include "Config_VisualizerPrismBeat.h"
#include "Config_VisualizerHaloDust.h"
#include "Config_GlobalHotkeys.h"
/**
 * @brief アプリケーションの設定（iniファイル）を管理するクラス
 */
class ConfigManager {
public:
    ConfigManager();
    ~ConfigManager();

    /**
     * @brief 設定の初期化（ファイルの読み込み・作成）
     * @return 成功ならtrue
     */
    bool Initialize();

    const Config_System& GetSystem() const { return m_configSystem; }
    const Config_Window& GetWindow() const { return m_configWindow; }
    const Config_Visibility& GetVisibility() const { return m_configVisibility; }
    const Config_Playlist& GetPlaylist() const { return m_configPlaylist; }
    const Config_Audio& GetAudio() const { return m_configAudio; }
    const Config_TrackDrum& GetTrackDrum() const { return m_configTrackDrum; }
    const Config_UICommonParm& GetUICommonParm() const { return m_configUICommonParm; }
    const Config_Background& GetBackground() const { return m_configBackground; }
    const Config_LayoutAppLogo& GetLayoutAppLogo() const { return m_configLayoutAppLogo; }
    const Config_LayoutLogoMenu& GetLayoutLogoMenu() const { return m_configLayoutLogoMenu; }
    const Config_LayoutNowPlaying& GetLayoutNowPlaying() const { return m_configLayoutNowPlaying; }
    const Config_LayoutSeekBar& GetLayoutSeekBar() const { return m_configLayoutSeekBar; }
    const Config_LayoutPlaybackControls& GetLayoutPlaybackControls() const { return m_configLayoutPlaybackControls; }
    const Config_LayoutVolumeControl& GetLayoutVolumeControl() const { return m_configLayoutVolumeControl; }
    const Config_LayoutTooltip& GetLayoutTooltip() const { return m_configLayoutTooltip; }
    const Config_LayoutPlaylist& GetLayoutPlaylist() const { return m_configLayoutPlaylist; }
    const Config_LayoutGlobalHotkeys& GetLayoutGlobalHotkeys() const { return m_configLayoutGlobalHotkeys; }
    const Config_LayoutOSD& GetLayoutOSD() const { return m_configLayoutOSD; }
    const Config_Visualizer& GetVisualizer() const { return m_configVisualizer; }
    const Config_VisualizerPrismBeat& GetVisualizerPrismBeat() const { return m_configVisualizerPrismBeat; }
    const Config_VisualizerHaloDust& GetVisualizerHaloDust() const { return m_configVisualizerHaloDust; }
    const Config_GlobalHotkeys& GetGlobalHotkeys() const { return m_configGlobalHotkeys; }

    /**
     * @brief 更新がないか確認する
     */

    void SaveWindowPosition(int x, int y, int width, int height);
    void SetZOrder(int zOrder);
    void SetSavePositionOnExit(bool save);
    void SetEnableResize(bool enable);
    void SetLockWindowPosition(bool lock);
    void SetIsPlaylistPinned(bool pinned);
    void SetShuffleMode(bool mode);
    void SetBackgroundArtMode(int mode);
    void SetVisualizerMode(int mode);
    void SetShowHotkeys(bool show);
    void SetEnableOSD(bool enable);
    void SetPlaylistPosition(int position);
    void SetDefaultVolume(float volume);

    bool CheckForUpdates();

    /**
     * @brief ウィンドウの表示位置とサイズを保存する
     */
    void SaveWindowPosition(int x, int y, int width, int height);

    /**
     * @brief デフォルト設定で上書き保存する
     */
    void SaveDefaultSettings();

    /**
     * @brief 初期値にリセットする
     */
    void ResetToDefaults();

    void LoadSettings();

    std::wstring GetExecutablePath() const;

private:
    int LoadOrWriteInt(const std::wstring& section, const std::wstring& key);
    float LoadOrWriteFloat(const std::wstring& section, const std::wstring& key);
    std::wstring LoadOrWriteString(const std::wstring& section, const std::wstring& key);
    std::wstring GetDefaultIniValue(const std::wstring& section, const std::wstring& key) const;

    void LoadSection_System(Config_System& outConfig);
    void LoadSection_Window(Config_Window& outConfig);
    void LoadSection_Visibility(Config_Visibility& outConfig);
    void LoadSection_Playlist(Config_Playlist& outConfig);
    void LoadSection_Audio(Config_Audio& outConfig);
    void LoadSection_TrackDrum(Config_TrackDrum& outConfig);
    void LoadSection_UICommonParm(Config_UICommonParm& outConfig);
    void LoadSection_Background(Config_Background& outConfig);
    void LoadSection_LayoutAppLogo(Config_LayoutAppLogo& outConfig);
    void LoadSection_LayoutLogoMenu(Config_LayoutLogoMenu& outConfig);
    void LoadSection_LayoutNowPlaying(Config_LayoutNowPlaying& outConfig);
    void LoadSection_LayoutSeekBar(Config_LayoutSeekBar& outConfig);
    void LoadSection_LayoutPlaybackControls(Config_LayoutPlaybackControls& outConfig);
    void LoadSection_LayoutVolumeControl(Config_LayoutVolumeControl& outConfig);
    void LoadSection_LayoutTooltip(Config_LayoutTooltip& outConfig);
    void LoadSection_LayoutPlaylist(Config_LayoutPlaylist& outConfig);
    void LoadSection_LayoutGlobalHotkeys(Config_LayoutGlobalHotkeys& outConfig);
    void LoadSection_LayoutOSD(Config_LayoutOSD& outConfig);
    void LoadSection_Visualizer(Config_Visualizer& outConfig);
    void LoadSection_VisualizerPrismBeat(Config_VisualizerPrismBeat& outConfig);
    void LoadSection_VisualizerHaloDust(Config_VisualizerHaloDust& outConfig);
    void LoadSection_GlobalHotkeys(Config_GlobalHotkeys& outConfig);

    std::wstring m_iniFilePath;
    std::filesystem::file_time_type m_lastIniWriteTime;

    Config_System m_configSystem;
    Config_Window m_configWindow;
    Config_Visibility m_configVisibility;
    Config_Playlist m_configPlaylist;
    Config_Audio m_configAudio;
    Config_TrackDrum m_configTrackDrum;
    Config_UICommonParm m_configUICommonParm;
    Config_Background m_configBackground;
    Config_LayoutAppLogo m_configLayoutAppLogo;
    Config_LayoutLogoMenu m_configLayoutLogoMenu;
    Config_LayoutNowPlaying m_configLayoutNowPlaying;
    Config_LayoutSeekBar m_configLayoutSeekBar;
    Config_LayoutPlaybackControls m_configLayoutPlaybackControls;
    Config_LayoutVolumeControl m_configLayoutVolumeControl;
    Config_LayoutTooltip m_configLayoutTooltip;
    Config_LayoutPlaylist m_configLayoutPlaylist;
    Config_LayoutGlobalHotkeys m_configLayoutGlobalHotkeys;
    Config_LayoutOSD m_configLayoutOSD;
    Config_Visualizer m_configVisualizer;
    Config_VisualizerPrismBeat m_configVisualizerPrismBeat;
    Config_VisualizerHaloDust m_configVisualizerHaloDust;
    Config_GlobalHotkeys m_configGlobalHotkeys;
};
