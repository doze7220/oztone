#pragma once
#include <windows.h>
#include <string>
#include <vector>

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
    std::wstring GetLogoMenuFontFamily() const { return m_logoMenuFontFamily; }
    std::wstring GetLogoMenuTextColor() const { return m_logoMenuTextColor; }
    std::wstring GetLogoMenuTypingFontFamily() const { return m_logoMenuTypingFontFamily; }
    float GetLogoMenuTypingFontSize() const { return m_logoMenuTypingFontSize; }
    int GetLogoMenuTextOffsetX() const { return m_logoMenuTextOffsetX; }
    int GetLogoMenuTextOffsetY() const { return m_logoMenuTextOffsetY; }
    float GetLogoMenuTypingLetterSpacing() const { return m_logoMenuTypingLetterSpacing; }
    float GetLogoMenuStrikeLength() const { return m_logoMenuStrikeLength; }
    float GetLogoMenuStrikeThickness() const { return m_logoMenuStrikeThickness; }
    float GetLogoMenuIconHoverBgAlpha() const { return m_logoMenuIconHoverBgAlpha; }
    float GetLogoMenuVisualizerFontSize() const { return m_logoMenuVisualizerFontSize; }
    int GetLogoMenuVisualizerIconOffsetX() const { return m_logoMenuVisualizerIconOffsetX; }
    int GetLogoMenuVisualizerIconOffsetY() const { return m_logoMenuVisualizerIconOffsetY; }
    float GetLogoMenuDescShadowOffsetX() const { return m_logoMenuDescShadowOffsetX; }
    float GetLogoMenuDescShadowOffsetY() const { return m_logoMenuDescShadowOffsetY; }
    float GetLogoMenuDescShadowOpacity() const { return m_logoMenuDescShadowOpacity; }

    /**
     * @brief デフォルト設定で上書き保存する
     */
    void SaveDefaultSettings();




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

    int GetPlaylistHoverWidth() const { return m_playlistHoverWidth; }

    float GetPlaylistBgOpacity() const { return m_playlistBgOpacity; }

    int GetPlaylistPosition() const { return m_playlistPosition; }
    float GetPlaylistGripOffset() const { return m_playlistGripOffset; }
    float GetPlaylistGripLineWidth() const { return m_playlistGripLineWidth; }
    std::wstring GetPlaylistGripLineColor() const { return m_playlistGripLineColor; }
    float GetPlaylistGripArrowHeight() const { return m_playlistGripArrowHeight; }
    float GetPlaylistGripArrowWidth() const { return m_playlistGripArrowWidth; }
    std::wstring GetPlaylistGripArrowColor() const { return m_playlistGripArrowColor; }
    float GetPlaylistGripShadowOffsetX() const { return m_playlistGripShadowOffsetX; }
    float GetPlaylistGripShadowOffsetY() const { return m_playlistGripShadowOffsetY; }
    float GetPlaylistGripShadowOpacity() const { return m_playlistGripShadowOpacity; }

private:
    std::wstring m_iniFilePath;

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


    int m_logoX;
    int m_logoY;
    int m_logoWidth;
    int m_logoHeight;

    float m_logoMenuIconSize;
    int m_logoMenuIconSpacing;
    int m_logoMenuIconOffsetX;
    int m_logoMenuIconOffsetY;
    float m_logoMenuScrollDuration;
    std::wstring m_logoMenuFontFamily;
    std::wstring m_logoMenuTextColor;
    std::wstring m_logoMenuTypingFontFamily;
    float m_logoMenuTypingFontSize;
    int m_logoMenuTextOffsetX;
    int m_logoMenuTextOffsetY;
    float m_logoMenuTypingLetterSpacing;
    float m_logoMenuStrikeLength;
    float m_logoMenuStrikeThickness;
    float m_logoMenuIconHoverBgAlpha;
    float m_logoMenuVisualizerFontSize;
    int m_logoMenuVisualizerIconOffsetX;
    int m_logoMenuVisualizerIconOffsetY;
    float m_logoMenuDescShadowOffsetX;
    float m_logoMenuDescShadowOffsetY;
    float m_logoMenuDescShadowOpacity;

    int m_baseX;
    int m_baseBottomOffset;
    int m_artOffsetX;
    int m_artOffsetY;
    int m_artSize;
    float m_fallbackArtOpacity;




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
    std::wstring GetExecutablePath() const;

    float m_defaultVolume;
    float m_controlHoverHeight;
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

    float m_playlistBgOpacity;

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

    void LoadSettings();
};
