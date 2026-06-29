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

    int GetLogoX() const { return m_logoX; }
    int GetLogoY() const { return m_logoY; }
    int GetLogoWidth() const { return m_logoWidth; }
    int GetLogoHeight() const { return m_logoHeight; }

private:
    std::wstring m_iniFilePath;

    bool m_showTitleBar;
    bool m_showWindowFrame;
    bool m_showTaskbar;

    int m_logoX;
    int m_logoY;
    int m_logoWidth;
    int m_logoHeight;

    std::wstring GetExecutablePath() const;
    void LoadSettings();
    void SaveDefaultSettings();
};
