#pragma once
#include <windows.h>
#include <string>

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

    /**
     * @brief ウィンドウの表示位置とサイズを保存する
     */
    void SaveWindowPosition(int x, int y, int width, int height);

    int GetLogoX() const { return m_logoX; }
    int GetLogoY() const { return m_logoY; }
    int GetLogoWidth() const { return m_logoWidth; }
    int GetLogoHeight() const { return m_logoHeight; }

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

    float GetSeekBarWidthRatio() const { return m_seekBarWidthRatio; }
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

    int GetPlaybackBaseBottomOffset() const { return m_playbackBaseBottomOffset; }
    int GetPlaybackCenterOffsetX() const { return m_playbackCenterOffsetX; }
    int GetPlaybackButtonSpacing() const { return m_playbackButtonSpacing; }
    int GetPlaybackButtonSize() const { return m_playbackButtonSize; }

private:
    std::wstring m_iniFilePath;

    bool m_showTitleBar;
    bool m_showWindowFrame;
    bool m_showTaskbar;

    int m_zOrder;
    bool m_savePositionOnExit;

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

    float m_seekBarWidthRatio;
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
    void LoadSettings();
};
