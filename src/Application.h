#pragma once
#include <windows.h>
#include "Window.h"
#include "ConfigManager.h"

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
    ConfigManager m_config;
    Window m_window;
};
