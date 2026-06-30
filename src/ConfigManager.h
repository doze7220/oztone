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

    int GetWindowX() const { return m_windowX; }
    int GetWindowY() const { return m_windowY; }
    int GetWindowWidth() const { return m_windowWidth; }
    int GetWindowHeight() const { return m_windowHeight; }

    /**
     * @brief ウィンドウの表示位置とサイズを保存する
     */
    void SaveWindowPosition(int x, int y, int width, int height);

    int GetLogoX() const { return m_logoX; }
    int GetLogoY() const { return m_logoY; }
    int GetLogoWidth() const { return m_logoWidth; }
    int GetLogoHeight() const { return m_logoHeight; }

    int GetBaseX() const { return m_baseX; }
    int GetBaseY() const { return m_baseY; }
    int GetArtOffsetX() const { return m_artOffsetX; }
    int GetArtOffsetY() const { return m_artOffsetY; }
    int GetArtSize() const { return m_artSize; }
    float GetBgOpacity() const { return m_bgOpacity; }
    float GetFallbackArtOpacity() const { return m_fallbackArtOpacity; }

    int GetTitleOffsetX() const { return m_titleOffsetX; }
    int GetTitleOffsetY() const { return m_titleOffsetY; }
    float GetTitleFontSize() const { return m_titleFontSize; }

    int GetArtistOffsetX() const { return m_artistOffsetX; }
    int GetArtistOffsetY() const { return m_artistOffsetY; }
    float GetArtistFontSize() const { return m_artistFontSize; }

    float GetSeekBarWidthRatio() const { return m_seekBarWidthRatio; }
    int GetSeekBarHeight() const { return m_seekBarHeight; }
    int GetSeekBarBottomOffset() const { return m_seekBarBottomOffset; }
    float GetSeekBarBgOpacity() const { return m_seekBarBgOpacity; }
    const std::wstring& GetSeekBarTimeFontFamily() const { return m_seekBarTimeFontFamily; }
    float GetSeekBarTimeFontSize() const { return m_seekBarTimeFontSize; }
    int GetSeekBarTimeAreaWidth() const { return m_seekBarTimeAreaWidth; }
    float GetSeekBarTimeLetterSpacing() const { return m_seekBarTimeLetterSpacing; }

    std::wstring GetDefaultPlaylistPath() const { return m_defaultPlaylistPath; }

private:
    std::wstring m_iniFilePath;

    bool m_showTitleBar;
    bool m_showWindowFrame;
    bool m_showTaskbar;

    int m_windowX;
    int m_windowY;
    int m_windowWidth;
    int m_windowHeight;

    int m_logoX;
    int m_logoY;
    int m_logoWidth;
    int m_logoHeight;

    int m_baseX;
    int m_baseY;
    int m_artOffsetX;
    int m_artOffsetY;
    int m_artSize;
    float m_bgOpacity;
    float m_fallbackArtOpacity;

    int m_titleOffsetX;
    int m_titleOffsetY;
    float m_titleFontSize;

    int m_artistOffsetX;
    int m_artistOffsetY;
    float m_artistFontSize;

    float m_seekBarWidthRatio;
    int m_seekBarHeight;
    int m_seekBarBottomOffset;
    float m_seekBarBgOpacity;
    std::wstring m_seekBarTimeFontFamily;
    float m_seekBarTimeFontSize;
    int m_seekBarTimeAreaWidth;
    float m_seekBarTimeLetterSpacing;

    std::wstring m_defaultPlaylistPath;

    std::wstring GetExecutablePath() const;
    void LoadSettings();
    void SaveDefaultSettings();
};
