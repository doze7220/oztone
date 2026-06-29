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

private:
    std::wstring m_iniFilePath;

    bool m_showTitleBar;
    bool m_showWindowFrame;
    bool m_showTaskbar;

    std::wstring GetExecutablePath() const;
    void LoadSettings();
    void SaveDefaultSettings();
};
