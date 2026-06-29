#pragma once
#include <windows.h>

class ConfigManager;

/**
 * @brief メインウィンドウを管理するクラス
 * 
 * Win32 APIのウィンドウ生成、メッセージループのハンドリング（WndProcの委譲）を行う。
 * 現在は開発初期段階のため、通常の枠ありウィンドウとして生成される。
 */
class Window {
public:
    Window();
    ~Window();

    /**
     * @brief ウィンドウを初期化して作成する
     * @param hInstance アプリケーションのインスタンスハンドル
     * @param nCmdShow ウィンドウの表示状態
     * @param config 設定マネージャへの参照
     * @return 成功した場合はtrue、失敗した場合はfalse
     */
    bool Initialize(HINSTANCE hInstance, int nCmdShow, const ConfigManager& config);

    /**
     * @brief ウィンドウのメッセージ処理を行う（1フレーム分）
     * @return アプリケーションを終了すべき場合はfalse、継続する場合はtrue
     */
    bool ProcessMessages();

    /**
     * @brief ウィンドウハンドルを取得する
     */
    HWND GetHandle() const { return m_hwnd; }

private:
    static LRESULT CALLBACK WindowProcStatic(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
    LRESULT WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

    HWND m_hwnd;
    HINSTANCE m_hInstance;
    const wchar_t* m_className = L"OZtoneWindowClass";
};
