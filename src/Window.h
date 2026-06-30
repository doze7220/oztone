#pragma once
#include <windows.h>
#include <shellapi.h>
#include <ole2.h>
#include <functional>
#include <string>
#include <vector>

class ConfigManager;
class Window;

class DropTarget : public IDropTarget {
public:
    DropTarget(Window* pWindow);
    // IUnknown
    HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void** ppvObject) override;
    ULONG STDMETHODCALLTYPE AddRef() override;
    ULONG STDMETHODCALLTYPE Release() override;

    // IDropTarget
    HRESULT STDMETHODCALLTYPE DragEnter(IDataObject* pDataObj, DWORD grfKeyState, POINTL pt, DWORD* pdwEffect) override;
    HRESULT STDMETHODCALLTYPE DragOver(DWORD grfKeyState, POINTL pt, DWORD* pdwEffect) override;
    HRESULT STDMETHODCALLTYPE DragLeave() override;
    HRESULT STDMETHODCALLTYPE Drop(IDataObject* pDataObj, DWORD grfKeyState, POINTL pt, DWORD* pdwEffect) override;
private:
    LONG m_refCount;
    Window* m_pWindow;
};

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
    bool Initialize(HINSTANCE hInstance, int nCmdShow, ConfigManager& config);

    /**
     * @brief ウィンドウのメッセージ処理を行う（1フレーム分）
     * @return アプリケーションを終了すべき場合はfalse、継続する場合はtrue
     */
    bool ProcessMessages();

    /**
     * @brief ウィンドウハンドルを取得する
     */
    HWND GetHandle() const { return m_hwnd; }

    /**
     * @brief マウスがロゴのホバー領域にあるかどうかを返す
     */
    bool IsHovered() const { return m_isHovered; }
    void SetHovered(bool hovered) { m_isHovered = hovered; }

    /**
     * @brief 座標がロゴ領域内にあるかを判定する
     */
    bool IsInLogoRegion(int x, int y) const;

    /**
     * @brief ファイルドロップを通知する
     */
    void NotifyFilesDropped(const std::vector<std::wstring>& files) {
        if (m_onFilesDropped) m_onFilesDropped(files);
    }

    /**
     * @brief ファイルがドロップされた時のコールバックを設定する
     */
    void SetOnFilesDroppedCallback(std::function<void(const std::vector<std::wstring>&)> cb) {
        m_onFilesDropped = cb;
    }

    /**
     * @brief メディアキーのコマンドが入力された時のコールバックを設定する
     */
    void SetMediaCommandCallback(std::function<void(int)> cb) {
        m_onMediaCommand = cb;
    }

    /**
     * @brief 外部プロセスからファイルパス文字列（WM_COPYDATA）を受信した際のコールバックを設定する
     */
    void SetCopyDataCallback(std::function<void(const std::wstring&)> cb) {
        m_onCopyDataCallback = cb;
    }

private:
    static LRESULT CALLBACK WindowProcStatic(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
    LRESULT WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

    HWND m_hwnd;
    HINSTANCE m_hInstance;
    const wchar_t* m_className = L"OZtoneWindowClass";

    ConfigManager* m_config;
    bool m_isHovered;
    bool m_isTrackingMouse;
    std::function<void(const std::vector<std::wstring>&)> m_onFilesDropped;
    std::function<void(int)> m_onMediaCommand;
    std::function<void(const std::wstring&)> m_onCopyDataCallback;
    DropTarget* m_pDropTarget;

    static constexpr UINT WM_TRAYICON = WM_APP + 1;
    static constexpr UINT WM_APP_MEDIAKEY = WM_APP + 2;
    NOTIFYICONDATAW m_nid;

    HHOOK m_keyboardHook;
    static HWND s_hwnd;
    static LRESULT CALLBACK LowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam);
};
