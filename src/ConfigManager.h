#pragma once
#include <windows.h>
#include <string>
#include <vector>
#include <filesystem>

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

    bool GetShowTitleBar() const { return m_showTitleBar; }
    bool GetShowWindowFrame() const { return m_showWindowFrame; }
    bool GetShowTaskbar() const { return m_showTaskbar; }

    bool GetShowAppLogo() const { return m_showAppLogo; }
    bool GetShowNowPlaying() const { return m_showNowPlaying; }
    bool GetShowNextTrack() const { return m_showNextTrack; }
    bool GetEnableNextTrack() const { return m_enableNextTrack; }
    bool GetShowSeekBar() const { return m_showSeekBar; }
    bool GetShowPlaybackControls() const { return m_showPlaybackControls; }
    bool GetShowVolumeControl() const { return m_showVolumeControl; }

    bool GetEnableShadow() const { return m_enableShadow; }
    float GetShadowOffsetX() const { return m_shadowOffsetX; }
    float GetShadowOffsetY() const { return m_shadowOffsetY; }
    float GetShadowOpacity() const { return m_shadowOpacity; }
    float GetBgDarkenOpacity() const { return m_bgDarkenOpacity; }
    float GetBgOpacity() const { return m_bgOpacity; }
    int GetBackgroundArtMode() const { return m_backgroundArtMode; }
    void SetBackgroundArtMode(int mode);

    int GetVisualizerMode() const { return m_visualizerMode; }
    void SetVisualizerMode(int mode);

    float GetVisualizerGamma() const { return m_visualizerGamma; }
    void SetVisualizerGamma(float gamma);

    bool GetEnablePreScan() const { return m_enablePreScan; }
    void SetEnablePreScan(bool enable) { m_enablePreScan = enable; }

    float GetHighFreqNoiseThreshold() const { return m_highFreqNoiseThreshold; }
    void SetHighFreqNoiseThreshold(float threshold);

    float GetBandGain0() const { return m_bandGain0; }
    float GetBandGain25() const { return m_bandGain25; }
    float GetBandGain50() const { return m_bandGain50; }
    float GetBandGain75() const { return m_bandGain75; }
    float GetBandGain100() const { return m_bandGain100; }
    void SetBandGains(float b0, float b25, float b50, float b75, float b100);

    float GetPrismBeatMaxHeightRatio() const { return m_prismBeatMaxHeightRatio; }
    void SetPrismBeatMaxHeightRatio(float ratio);

    float GetHaloDustBaseRadiusRatio() const { return m_haloDustBaseRadiusRatio; }
    void SetHaloDustBaseRadiusRatio(float ratio);

    float GetHaloDustGraphLengthRatio() const { return m_haloDustGraphLengthRatio; }
    void SetHaloDustGraphLengthRatio(float ratio);

    int GetWindowX() const { return m_windowX; }
    int GetWindowY() const { return m_windowY; }
    int GetWindowWidth() const { return m_windowWidth; }
    int GetWindowHeight() const { return m_windowHeight; }
    
    int GetZOrder() const { return m_zOrder; }
    void SetZOrder(int zOrder);

    bool GetSavePositionOnExit() const { return m_savePositionOnExit; }
    void SetSavePositionOnExit(bool save);

    bool GetEnableResize() const { return m_enableResize; }
    void SetEnableResize(bool enable);

    bool GetLockWindowPosition() const { return m_lockWindowPosition; }
    void SetLockWindowPosition(bool lock);

    bool GetIsPlaylistPinned() const { return m_isPlaylistPinned; }
    void SetIsPlaylistPinned(bool pinned);

    bool GetShuffleMode() const { return m_shuffleMode; }
    void SetShuffleMode(bool mode);

    float GetSkipSeconds() const { return m_skipSeconds; }
    void SetSkipSeconds(float seconds);

    std::wstring GetSkipIconPoints() const { return m_skipIconPoints; }
    float GetSkipTextFontSize() const { return m_skipTextFontSize; }
    float GetSkipTextOffsetX() const { return m_skipTextOffsetX; }
    float GetSkipTextOffsetY() const { return m_skipTextOffsetY; }
    std::wstring GetSkipTextShadowColor() const { return m_skipTextShadowColor; }
    float GetSkipTextShadowOpacity() const { return m_skipTextShadowOpacity; }
    float GetSkipTextShadowShift() const { return m_skipTextShadowShift; }

    // --- Layout_GlobalHotkeys ---
    std::wstring GetGlobalHotkeysFontFamily() const { return m_ghFontFamily; }
    float GetGlobalHotkeysFontSize() const { return m_ghFontSize; }
    float GetGlobalHotkeysLineSpacing() const { return m_ghLineSpacing; }
    std::wstring GetGlobalHotkeysCoreColor() const { return m_ghCoreColor; }
    std::wstring GetGlobalHotkeysGlowColor() const { return m_ghGlowColor; }
    float GetGlobalHotkeysGlowOpacity() const { return m_ghGlowOpacity; }
    std::wstring GetGlobalHotkeysShadowColor() const { return m_ghShadowColor; }
    float GetGlobalHotkeysShadowOpacity() const { return m_ghShadowOpacity; }
    float GetGlobalHotkeysKeyColumnOffset() const { return m_ghKeyColumnOffset; }
    float GetGlobalHotkeysActionColumnOffset() const { return m_ghActionColumnOffset; }

    // --- Layout_OSD ---
    std::wstring GetOsdFontFamily() const { return m_osdFontFamily; }
    float GetOsdFontSize() const { return m_osdFontSize; }
    std::wstring GetOsdTextColor() const { return m_osdTextColor; }
    std::wstring GetOsdShadowColor() const { return m_osdShadowColor; }
    float GetOsdShadowOffsetX() const { return m_osdShadowOffsetX; }
    float GetOsdShadowOffsetY() const { return m_osdShadowOffsetY; }
    float GetOsdShadowOpacity() const { return m_osdShadowOpacity; }
    float GetOsdFadeSpeed() const { return m_osdFadeSpeed; }
    float GetOsdFadeWait() const { return m_osdFadeWait; }

    bool GetShowHotkeys() const { return m_showHotkeys; }
    void SetShowHotkeys(bool show);

    bool GetEnableOSD() const { return m_enableOSD; }
    void SetEnableOSD(bool enable);

    int GetModifierNextTrack() const { return m_modNextTrack; }
    int GetVKNextTrack() const { return m_vkNextTrack; }
    void SetNextTrackHotkey(int mod, int vk);

    int GetModifierPrevTrack() const { return m_modPrevTrack; }
    int GetVKPrevTrack() const { return m_vkPrevTrack; }
    void SetPrevTrackHotkey(int mod, int vk);

    int GetModifierPlayPause() const { return m_modPlayPause; }
    int GetVKPlayPause() const { return m_vkPlayPause; }
    void SetPlayPauseHotkey(int mod, int vk);

    int GetModifierStop() const { return m_modStop; }
    int GetVKStop() const { return m_vkStop; }
    void SetStopHotkey(int mod, int vk);

    int GetModifierVolUp5() const { return m_modVolUp5; }
    int GetVKVolUp5() const { return m_vkVolUp5; }
    void SetVolUp5Hotkey(int mod, int vk);

    int GetModifierVolDown5() const { return m_modVolDown5; }
    int GetVKVolDown5() const { return m_vkVolDown5; }
    void SetVolDown5Hotkey(int mod, int vk);

    int GetModifierVolUp25() const { return m_modVolUp25; }
    int GetVKVolUp25() const { return m_vkVolUp25; }
    void SetVolUp25Hotkey(int mod, int vk);

    int GetModifierVolDown25() const { return m_modVolDown25; }
    int GetVKVolDown25() const { return m_vkVolDown25; }
    void SetVolDown25Hotkey(int mod, int vk);

    int GetModifierPrevPlaylist() const { return m_modPrevPlaylist; }
    int GetVKPrevPlaylist() const { return m_vkPrevPlaylist; }
    void SetPrevPlaylistHotkey(int mod, int vk);

    int GetModifierNextPlaylist() const { return m_modNextPlaylist; }
    int GetVKNextPlaylist() const { return m_vkNextPlaylist; }
    void SetNextPlaylistHotkey(int mod, int vk);

    int GetModifierActiveTopMost() const { return m_modActiveTopMost; }
    int GetVKActiveTopMost() const { return m_vkActiveTopMost; }
    void SetActiveTopMostHotkey(int mod, int vk);

    int GetModifierActiveBottom() const { return m_modActiveBottom; }
    int GetVKActiveBottom() const { return m_vkActiveBottom; }
    void SetActiveBottomHotkey(int mod, int vk);

    int GetModifierExitApp() const { return m_modExitApp; }
    int GetVKExitApp() const { return m_vkExitApp; }
    void SetExitAppHotkey(int mod, int vk);

    /**
     * @brief 更新がないか確認する
     */
    bool CheckForUpdates();

    /**
     * @brief ウィンドウの表示位置とサイズを保存する
     */
    void SaveWindowPosition(int x, int y, int width, int height);

    int GetLogoX() const { return m_logoX; }
    int GetLogoY() const { return m_logoY; }
    int GetLogoWidth() const { return m_logoWidth; }
    int GetLogoHeight() const { return m_logoHeight; }

    float GetLogoMenuIconSize() const { return m_logoMenuIconSize; }
    int GetLogoMenuIconSpacing() const { return m_logoMenuIconSpacing; }
    int GetLogoMenuIconOffsetX() const { return m_logoMenuIconOffsetX; }
    int GetLogoMenuIconOffsetY() const { return m_logoMenuIconOffsetY; }
    float GetLogoMenuScrollDuration() const { return m_logoMenuScrollDuration; }
    float GetMenuLeaveDelay() const { return m_menuLeaveDelay; }
    std::wstring GetLogoMenuFontFamily() const { return m_logoMenuFontFamily; }
    std::wstring GetLogoMenuTextColor() const { return m_logoMenuTextColor; }
    std::wstring GetLogoMenuTypingFontFamily() const { return m_logoMenuTypingFontFamily; }
    float GetLogoMenuTypingFontSize() const { return m_logoMenuTypingFontSize; }
    int GetLogoMenuTextOffsetX() const { return m_logoMenuTextOffsetX; }
    int GetLogoMenuTextOffsetY() const { return m_logoMenuTextOffsetY; }
    float GetLogoMenuTypingLetterSpacing() const { return m_logoMenuTypingLetterSpacing; }
    float GetLogoMenuVisualizerFontSize() const { return m_logoMenuVisualizerFontSize; }
    int GetLogoMenuVisualizerIconOffsetX() const { return m_logoMenuVisualizerIconOffsetX; }
    int GetLogoMenuVisualizerIconOffsetY() const { return m_logoMenuVisualizerIconOffsetY; }
    float GetLogoMenuLockIconFontSize() const { return m_logoMenuLockIconFontSize; }
    int GetLogoMenuLockIconOffsetX() const { return m_logoMenuLockIconOffsetX; }
    int GetLogoMenuLockIconOffsetY() const { return m_logoMenuLockIconOffsetY; }
    float GetLogoMenuDescShadowOffsetX() const { return m_logoMenuDescShadowOffsetX; }
    float GetLogoMenuDescShadowOffsetY() const { return m_logoMenuDescShadowOffsetY; }
    float GetLogoMenuDescShadowOpacity() const { return m_logoMenuDescShadowOpacity; }

    std::wstring GetHoverIconColor() const { return m_hoverIconColor; }
    float GetHoverFadeOutSpeed() const { return m_hoverFadeOutSpeed; }
    std::wstring GetPlayingItemColor() const { return m_playingItemColor; }
    std::wstring GetHoverItemColor() const { return m_hoverItemColor; }

    /**
     * @brief デフォルト設定で上書き保存する
     */
    void SaveDefaultSettings();

    /**
     * @brief 初期値にリセットする
     */
    void ResetToDefaults();




    int GetBaseX() const { return m_baseX; }
    int GetBaseBottomOffset() const { return m_baseBottomOffset; }
    int GetArtOffsetX() const { return m_artOffsetX; }
    int GetArtOffsetY() const { return m_artOffsetY; }
    int GetArtSize() const { return m_artSize; }
    float GetFallbackArtOpacity() const { return m_fallbackArtOpacity; }




    int GetTitleOffsetX() const { return m_titleOffsetX; }
    int GetTitleOffsetY() const { return m_titleOffsetY; }
    float GetTitleFontSize() const { return m_titleFontSize; }
    std::wstring GetTitleFontFamily() const { return m_titleFontFamily; }

    int GetArtistOffsetX() const { return m_artistOffsetX; }
    int GetArtistOffsetY() const { return m_artistOffsetY; }
    float GetArtistFontSize() const { return m_artistFontSize; }
    std::wstring GetArtistFontFamily() const { return m_artistFontFamily; }

    float GetSeekBarMargin() const { return m_seekBarMargin; }
    int GetSeekBarHeight() const { return m_seekBarHeight; }
    int GetSeekBarBottomOffset() const { return m_seekBarBottomOffset; }
    float GetSeekBarBgOpacity() const { return m_seekBarBgOpacity; }
    const std::wstring& GetSeekBarTimeFontFamily() const { return m_seekBarTimeFontFamily; }
    float GetSeekBarTimeFontSize() const { return m_seekBarTimeFontSize; }
    int GetSeekBarTimeAreaWidth() const { return m_seekBarTimeAreaWidth; }
    float GetSeekBarTimeLetterSpacing() const { return m_seekBarTimeLetterSpacing; }

    int GetNextBaseRightOffset() const { return m_nextBaseRightOffset; }
    int GetNextBaseBottomOffset() const { return m_nextBaseBottomOffset; }
    int GetNextArtOffsetX() const { return m_nextArtOffsetX; }
    int GetNextArtOffsetY() const { return m_nextArtOffsetY; }
    int GetNextArtSize() const { return m_nextArtSize; }
    float GetNextBgOpacity() const { return m_nextBgOpacity; }
    float GetNextFallbackArtOpacity() const { return m_nextFallbackArtOpacity; }



    int GetNextLabelOffsetX() const { return m_nextLabelOffsetX; }
    int GetNextLabelOffsetY() const { return m_nextLabelOffsetY; }
    float GetNextLabelFontSize() const { return m_nextLabelFontSize; }
    std::wstring GetNextLabelFontFamily() const { return m_nextLabelFontFamily; }

    int GetNextTitleOffsetX() const { return m_nextTitleOffsetX; }
    int GetNextTitleOffsetY() const { return m_nextTitleOffsetY; }
    float GetNextTitleFontSize() const { return m_nextTitleFontSize; }
    std::wstring GetNextTitleFontFamily() const { return m_nextTitleFontFamily; }

    int GetNextArtistOffsetX() const { return m_nextArtistOffsetX; }
    int GetNextArtistOffsetY() const { return m_nextArtistOffsetY; }
    float GetNextArtistFontSize() const { return m_nextArtistFontSize; }
    std::wstring GetNextArtistFontFamily() const { return m_nextArtistFontFamily; }
    std::wstring GetDefaultPlaylistPath() const { return m_defaultPlaylistPath; }
    void SetDefaultPlaylistPath(const std::wstring& path);
    std::vector<std::wstring> GetAvailablePlaylists() const;

    int GetPlaybackBaseBottomOffset() const { return m_playbackBaseBottomOffset; }
    int GetPlaybackCenterOffsetX() const { return m_playbackCenterOffsetX; }
    int GetPlaybackButtonSpacing() const { return m_playbackButtonSpacing; }
    int GetPlaybackButtonSize() const { return m_playbackButtonSize; }

    float GetDefaultVolume() const { return m_defaultVolume; }
    void SetDefaultVolume(float volume);

    float GetControlHoverHeight() const { return m_controlHoverHeight; }
    float GetControlLeaveDelay() const { return m_controlLeaveDelay; }

    int GetVolumeBaseLeftOffset() const { return m_volBaseLeftOffset; }
    int GetVolumeBaseBottomOffset() const { return m_volBaseBottomOffset; }
    int GetVolumeIconSize() const { return m_volIconSize; }
    int GetVolumeTextOffsetX() const { return m_volTextOffsetX; }
    int GetVolumeTextOffsetY() const { return m_volTextOffsetY; }
    float GetVolumeTextLetterSpacing() const { return m_volTextLetterSpacing; }
    float GetVolumeFontSize() const { return m_volFontSize; }
    std::wstring GetVolumeFontFamily() const { return m_volFontFamily; }

    bool GetVolumeEnableShadow() const { return m_volEnableShadow; }
    float GetVolumeShadowOffsetX() const { return m_volShadowOffsetX; }
    float GetVolumeShadowOffsetY() const { return m_volShadowOffsetY; }
    float GetVolumeShadowOpacity() const { return m_volShadowOpacity; }

    std::wstring GetVolumeTooltipText() const { return m_volTooltipText; }
    float GetVolumeTooltipFontSize() const { return m_volTooltipFontSize; }
    std::wstring GetVolumeTooltipFontFamily() const { return m_volTooltipFontFamily; }
    float GetVolumeTooltipOffsetY() const { return m_volTooltipOffsetY; }
    std::wstring GetVolumeTooltipBgColor() const { return m_volTooltipBgColor; }
    float GetVolumeTooltipBgOpacity() const { return m_volTooltipBgOpacity; }
    std::wstring GetVolumeTooltipTextColor() const { return m_volTooltipTextColor; }
    float GetVolumeTooltipWidth() const { return m_volTooltipWidth; }
    float GetVolumeTooltipHeight() const { return m_volTooltipHeight; }

    std::wstring GetTrackCountFontFamily() const { return m_trackCountFontFamily; }
    float GetTrackCountFontSize() const { return m_trackCountFontSize; }
    int GetTrackCountOffsetX() const { return m_trackCountOffsetX; }
    int GetTrackCountOffsetY() const { return m_trackCountOffsetY; }
    int GetTrackCountTextAlignment() const { return m_trackCountTextAlignment; }
    float GetTrackCountLetterSpacing() const { return m_trackCountLetterSpacing; }
    float GetTrackCountShadowOffsetX() const { return m_trackCountShadowOffsetX; }
    float GetTrackCountShadowOffsetY() const { return m_trackCountShadowOffsetY; }
    float GetTrackCountShadowOpacity() const { return m_trackCountShadowOpacity; }

    int GetPlaylistWidth() const { return m_playlistWidth; }
    int GetPlaylistItemOffsetY() const { return m_playlistItemOffsetY; }
    float GetPlaylistTitleFontSize() const { return m_playlistTitleFontSize; }
    std::wstring GetPlaylistTitleFontFamily() const { return m_playlistTitleFontFamily; }
    int GetPlaylistTitleOffsetX() const { return m_playlistTitleOffsetX; }
    int GetPlaylistTitleOffsetY() const { return m_playlistTitleOffsetY; }

    float GetPlaylistArtistFontSize() const { return m_playlistArtistFontSize; }
    std::wstring GetPlaylistArtistFontFamily() const { return m_playlistArtistFontFamily; }
    int GetPlaylistArtistOffsetX() const { return m_playlistArtistOffsetX; }
    int GetPlaylistArtistOffsetY() const { return m_playlistArtistOffsetY; }
    std::wstring GetPlaylistArtistColor() const { return m_playlistArtistColor; }

    float GetPlaylistTimeFontSize() const { return m_playlistTimeFontSize; }
    std::wstring GetPlaylistTimeFontFamily() const { return m_playlistTimeFontFamily; }
    int GetPlaylistTimeOffsetX() const { return m_playlistTimeOffsetX; }
    int GetPlaylistTimeOffsetY() const { return m_playlistTimeOffsetY; }
    float GetPlaylistTimeLetterSpacing() const { return m_playlistTimeLetterSpacing; }
    std::wstring GetPlaylistTimeColor() const { return m_playlistTimeColor; }

    float GetPlaylistToolbarHeight() const { return m_playlistToolbarHeight; }
    float GetPlaylistToolbarIconSize() const { return m_playlistToolbarIconSize; }
    float GetPlaylistToolbarIconSpacing() const { return m_playlistToolbarIconSpacing; }
    float GetPlaylistToolbarTextOffsetY() const { return m_playlistToolbarTextOffsetY; }
    float GetPlaylistToolbarTextFontSize() const { return m_playlistToolbarTextFontSize; }

    int GetPinSubIconOffsetX() const { return m_pinSubIconOffsetX; }
    int GetPinSubIconOffsetY() const { return m_pinSubIconOffsetY; }
    float GetPinSubIconFontSize() const { return m_pinSubIconFontSize; }

    int GetPlaylistHoverWidth() const { return m_playlistHoverWidth; }

    float GetPlaylistBgOpacity() const { return m_playlistBgOpacity; }
    float GetPlaylistLeaveDelay() const { return m_playlistLeaveDelay; }

    int GetPlaylistPosition() const { return m_playlistPosition; }
    void SetPlaylistPosition(int position);
    float GetPlaylistGripOffset() const { return m_playlistGripOffset; }
    float GetPlaylistGripLineWidth() const { return m_playlistGripLineWidth; }
    std::wstring GetPlaylistGripLineColor() const { return m_playlistGripLineColor; }
    float GetPlaylistGripArrowHeight() const { return m_playlistGripArrowHeight; }
    float GetPlaylistGripArrowWidth() const { return m_playlistGripArrowWidth; }
    std::wstring GetPlaylistGripArrowColor() const { return m_playlistGripArrowColor; }
    float GetPlaylistGripShadowOffsetX() const { return m_playlistGripShadowOffsetX; }
    float GetPlaylistGripShadowOffsetY() const { return m_playlistGripShadowOffsetY; }
    float GetPlaylistGripShadowOpacity() const { return m_playlistGripShadowOpacity; }

    void LoadSettings();

private:
    std::wstring m_iniFilePath;
    std::filesystem::file_time_type m_lastIniWriteTime;

    bool m_showTitleBar;
    bool m_showWindowFrame;
    bool m_showTaskbar;

    bool m_showAppLogo;
    bool m_showNowPlaying;
    bool m_showNextTrack;
    bool m_enableNextTrack;
    bool m_showSeekBar;
    bool m_showPlaybackControls;
    bool m_showVolumeControl;

    int m_zOrder;
    bool m_savePositionOnExit;
    bool m_enableResize;
    bool m_lockWindowPosition;
    bool m_shuffleMode;
    bool m_isPlaylistPinned;
    float m_skipSeconds;

    int m_windowX;
    int m_windowY;
    int m_windowWidth;
    int m_windowHeight;

    bool m_enableShadow;
    float m_shadowOffsetX;
    float m_shadowOffsetY;
    float m_shadowOpacity;
    float m_bgDarkenOpacity;
    float m_bgOpacity;
    int m_backgroundArtMode;
    int m_visualizerMode;
    float m_visualizerGamma;
    bool m_enablePreScan = true;
    float m_highFreqNoiseThreshold;
    float m_bandGain0;
    float m_bandGain25;
    float m_bandGain50;
    float m_bandGain75;
    float m_bandGain100;

    float m_prismBeatMaxHeightRatio;
    float m_haloDustBaseRadiusRatio;
    float m_haloDustGraphLengthRatio;

    int m_logoX;
    int m_logoY;
    int m_logoWidth;
    int m_logoHeight;

    float m_logoMenuIconSize;
    int m_logoMenuIconSpacing;
    int m_logoMenuIconOffsetX;
    int m_logoMenuIconOffsetY;
    float m_logoMenuScrollDuration;
    float m_menuLeaveDelay;
    std::wstring m_logoMenuFontFamily;
    std::wstring m_logoMenuTextColor;
    std::wstring m_logoMenuTypingFontFamily;
    float m_logoMenuTypingFontSize;
    int m_logoMenuTextOffsetX;
    int m_logoMenuTextOffsetY;
    float m_logoMenuTypingLetterSpacing;
    float m_logoMenuVisualizerFontSize;
    int m_logoMenuVisualizerIconOffsetX;
    int m_logoMenuVisualizerIconOffsetY;
    float m_logoMenuLockIconFontSize;
    int m_logoMenuLockIconOffsetX;
    int m_logoMenuLockIconOffsetY;
    float m_logoMenuDescShadowOffsetX;
    float m_logoMenuDescShadowOffsetY;
    float m_logoMenuDescShadowOpacity;

    int m_baseX;
    int m_baseBottomOffset;
    int m_artOffsetX;
    int m_artOffsetY;
    int m_artSize;
    float m_fallbackArtOpacity;

    std::wstring m_hoverIconColor;
    float m_hoverFadeOutSpeed;
    std::wstring m_playingItemColor;
    std::wstring m_hoverItemColor;




    int m_titleOffsetX;
    int m_titleOffsetY;
    float m_titleFontSize;
    std::wstring m_titleFontFamily;

    int m_artistOffsetX;
    int m_artistOffsetY;
    float m_artistFontSize;
    std::wstring m_artistFontFamily;

    float m_seekBarMargin;
    int m_seekBarHeight;
    int m_seekBarBottomOffset;
    float m_seekBarBgOpacity;
    std::wstring m_seekBarTimeFontFamily;
    float m_seekBarTimeFontSize;
    int m_seekBarTimeAreaWidth;
    float m_seekBarTimeLetterSpacing;

    int m_nextBaseRightOffset;
    int m_nextBaseBottomOffset;
    int m_nextArtOffsetX;
    int m_nextArtOffsetY;
    int m_nextArtSize;
    float m_nextBgOpacity;
    float m_nextFallbackArtOpacity;



    int m_nextLabelOffsetX;
    int m_nextLabelOffsetY;
    float m_nextLabelFontSize;
    std::wstring m_nextLabelFontFamily;

    int m_nextTitleOffsetX;
    int m_nextTitleOffsetY;
    float m_nextTitleFontSize;
    std::wstring m_nextTitleFontFamily;

    int m_nextArtistOffsetX;
    int m_nextArtistOffsetY;
    float m_nextArtistFontSize;
    std::wstring m_nextArtistFontFamily;

    std::wstring m_defaultPlaylistPath;

    int m_playbackBaseBottomOffset;
    int m_playbackCenterOffsetX;
    int m_playbackButtonSpacing;
    int m_playbackButtonSize;

    std::wstring m_skipIconPoints;
    float m_skipTextFontSize;
    float m_skipTextOffsetX;
    float m_skipTextOffsetY;
    std::wstring m_skipTextShadowColor;
    float m_skipTextShadowOpacity;
    float m_skipTextShadowShift;

    // --- Layout_GlobalHotkeys ---
    std::wstring m_ghFontFamily;
    float m_ghFontSize;
    float m_ghLineSpacing;
    std::wstring m_ghCoreColor;
    std::wstring m_ghGlowColor;
    float m_ghGlowOpacity;
    std::wstring m_ghShadowColor;
    float m_ghShadowOpacity;
    float m_ghKeyColumnOffset;
    float m_ghActionColumnOffset;

    // --- Layout_OSD ---
    std::wstring m_osdFontFamily;
    float m_osdFontSize;
    std::wstring m_osdTextColor;
    std::wstring m_osdShadowColor;
    float m_osdShadowOffsetX;
    float m_osdShadowOffsetY;
    float m_osdShadowOpacity;
    float m_osdFadeSpeed;
    float m_osdFadeWait;

    std::wstring GetExecutablePath() const;

    float m_defaultVolume;
    float m_controlHoverHeight;
    float m_controlLeaveDelay;
    int m_volBaseLeftOffset;
    int m_volBaseBottomOffset;
    int m_volIconSize;
    int m_volTextOffsetX;
    int m_volTextOffsetY;
    float m_volTextLetterSpacing;
    float m_volFontSize;
    std::wstring m_volFontFamily;

    bool m_volEnableShadow;
    float m_volShadowOffsetX;
    float m_volShadowOffsetY;
    float m_volShadowOpacity;

    std::wstring m_volTooltipText;
    float m_volTooltipFontSize;
    std::wstring m_volTooltipFontFamily;
    float m_volTooltipOffsetY;
    std::wstring m_volTooltipBgColor;
    float m_volTooltipBgOpacity;
    std::wstring m_volTooltipTextColor;
    float m_volTooltipWidth;
    float m_volTooltipHeight;

    std::wstring m_trackCountFontFamily;
    float m_trackCountFontSize;
    int m_trackCountOffsetX;
    int m_trackCountOffsetY;
    int m_trackCountTextAlignment;
    float m_trackCountLetterSpacing;
    float m_trackCountShadowOffsetX;
    float m_trackCountShadowOffsetY;
    float m_trackCountShadowOpacity;

    int m_playlistWidth;
    int m_playlistItemOffsetY;

    float m_playlistTitleFontSize;
    std::wstring m_playlistTitleFontFamily;
    int m_playlistTitleOffsetX;
    int m_playlistTitleOffsetY;

    float m_playlistArtistFontSize;
    std::wstring m_playlistArtistFontFamily;
    int m_playlistArtistOffsetX;
    int m_playlistArtistOffsetY;
    std::wstring m_playlistArtistColor;

    float m_playlistTimeFontSize;
    std::wstring m_playlistTimeFontFamily;
    int m_playlistTimeOffsetX;
    int m_playlistTimeOffsetY;
    float m_playlistTimeLetterSpacing;
    std::wstring m_playlistTimeColor;

    int m_playlistHoverWidth;

    float m_playlistToolbarHeight;
    float m_playlistToolbarIconSize;
    float m_playlistToolbarIconSpacing;
    float m_playlistToolbarTextOffsetY;
    float m_playlistToolbarTextFontSize;

    int m_pinSubIconOffsetX;
    int m_pinSubIconOffsetY;
    float m_pinSubIconFontSize;

    float m_playlistBgOpacity;
    float m_playlistLeaveDelay;

    int m_playlistPosition;
    float m_playlistGripOffset;
    float m_playlistGripLineWidth;
    std::wstring m_playlistGripLineColor;
    float m_playlistGripArrowHeight;
    float m_playlistGripArrowWidth;
    std::wstring m_playlistGripArrowColor;
    float m_playlistGripShadowOffsetX;
    float m_playlistGripShadowOffsetY;
    float m_playlistGripShadowOpacity;

    bool m_showHotkeys;
    bool m_enableOSD;
    int m_modNextTrack;
    int m_vkNextTrack;
    int m_modPrevTrack;
    int m_vkPrevTrack;
    int m_modPlayPause;
    int m_vkPlayPause;
    int m_modStop;
    int m_vkStop;
    int m_modVolUp5;
    int m_vkVolUp5;
    int m_modVolDown5;
    int m_vkVolDown5;
    int m_modVolUp25;
    int m_vkVolUp25;
    int m_modVolDown25;
    int m_vkVolDown25;
    int m_modPrevPlaylist;
    int m_vkPrevPlaylist;
    int m_modNextPlaylist;
    int m_vkNextPlaylist;
    int m_modActiveTopMost;
    int m_vkActiveTopMost;
    int m_modActiveBottom;
    int m_vkActiveBottom;
    int m_modExitApp;
    int m_vkExitApp;
};
