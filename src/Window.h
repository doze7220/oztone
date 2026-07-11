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

    static constexpr UINT ID_TRAY_ZORDER_NORMAL = 1010;
    static constexpr UINT ID_TRAY_ZORDER_TOPMOST = 1011;
    static constexpr UINT ID_TRAY_ZORDER_BOTTOM = 1012;
    static constexpr UINT ID_TRAY_SAVE_POS = 1013;
    static constexpr UINT ID_TRAY_RESET_POS = 1014;
    static constexpr UINT ID_TRAY_RESET_ALL = 1015;
    static constexpr UINT ID_TRAY_PLAYLIST_MENU = 1025;
    static constexpr UINT ID_TRAY_EXIT = 1001;

    static constexpr UINT ID_TRAY_PLAY_PAUSE = 1050;
    static constexpr UINT ID_TRAY_PREV_TRACK = 1051;
    static constexpr UINT ID_TRAY_NEXT_TRACK = 1052;
    static constexpr UINT ID_TRAY_VOL_100 = 1060;
    static constexpr UINT ID_TRAY_VOL_75 = 1061;
    static constexpr UINT ID_TRAY_VOL_50 = 1062;
    static constexpr UINT ID_TRAY_VOL_25 = 1063;
    static constexpr UINT ID_TRAY_VOL_MENU = 1064;
    static constexpr UINT ID_TRAY_ADVANCED_MENU = 1070;
    static constexpr UINT ID_TRAY_SHOW_HOTKEYS = 1071;
    static constexpr UINT ID_TRAY_SHOW_OSD = 1072;

    static constexpr UINT ID_LOGO_EXIT = 3001;
    static constexpr UINT ID_LOGO_BG_MODE = 3002;
    static constexpr UINT ID_LOGO_RESIZE_MODE = 3003;
    static constexpr UINT ID_LOGO_VISUALIZER = 3004;
    static constexpr UINT ID_LOGO_SHUFFLE = 3005;
    static constexpr UINT ID_LOGO_PLAYLIST_POS = 3006;
    static constexpr UINT ID_LOGO_LOCK_POS = 3007;

    struct LogoMenuItem {
        int commandId;
        std::wstring iconText;
        bool isToggle;
        bool toggleState;
        std::wstring labelText;
    };

    static constexpr UINT ID_TRAY_PLAYLIST_START = 2000;

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
     * @brief 再生コントロールのホバー状態
     */
    bool IsControlHovered() const { return m_isControlHovered; }
    void SetControlHovered(bool hovered) { m_isControlHovered = hovered; }

    /**
     * @brief 座標が再生コントロール領域内にあるかを判定する
     */
    bool IsInPlaybackControlRegion(int x, int y) const;

    /**
     * @brief プレイリストのホバー状態
     */
    bool IsPlaylistHovered() const { return m_isPlaylistHovered; }
    void SetPlaylistHovered(bool hovered) { m_isPlaylistHovered = hovered; }

    /**
     * @brief 座標がプレイリスト（TRACK表示）領域内にあるかを判定する
     */
    bool IsInPlaylistRegion(int x, int y) const;
    int GetPlaylistToolbarButtonAt(int x, int y) const;
    bool IsPlaylistPinnedButtonAt(int x, int y) const;
    int GetPlaylistToolbarHoveredIndex() const { return m_playlistToolbarHoveredIndex; }

    /**
     * @brief クリックされた再生コントロールボタンのIDを取得する (0:なし, 1:Prev, 2:Play/Pause, 3:Next)
     */
    int GetPlaybackButtonAt(int x, int y) const;

    /**
     * @brief 座標が音量コントロール領域内にあるかを判定する
     */
    bool IsInVolumeControlRegion(int x, int y) const;
    bool IsVolumeHovered() const { return m_isVolumeHovered; }
    void SetVolumeHovered(bool hovered) { m_isVolumeHovered = hovered; }


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
     * @brief シークコマンドが入力された時のコールバックを設定する
     */
    void SetSkipCommandCallback(std::function<void(float)> cb) {
        m_onSkipCommand = cb;
    }

    /**
     * @brief 外部プロセスからファイルパス文字列（WM_COPYDATA）を受信した際のコールバックを設定する
     */
    void SetCopyDataCallback(std::function<void(const std::wstring&)> cb) {
        m_onCopyDataCallback = cb;
    }

    /**
     * @brief 音量設定コマンドが入力された時のコールバックを設定する
     */
    void SetVolumeSetCallback(std::function<void(float)> cb) {
        m_onVolumeSetCommand = cb;
    }

    /**
     * @brief プレイリスト切り替えコマンドが入力された時のコールバックを設定する
     */
    void SetPlaylistSwitchCallback(std::function<void(const std::wstring&)> cb) {
        m_onPlaylistSwitchCommand = cb;
    }

    /**
     * @brief 音量コントロール上でマウスホイールが回転されたときのコールバックを設定する
     */
    void SetVolumeScrollCallback(std::function<void(int)> cb) {
        m_onVolumeScroll = cb;
    }

    /**
     * @brief シャッフル切り替えコマンドが入力された時のコールバックを設定する
     */
    void SetShuffleCallback(std::function<void()> cb) {
        m_onShuffleCommand = cb;
    }

    /**
     * @brief プレイリスト上でマウスホイールが回転されたときのコールバックを設定する
     */
    void SetPlaylistScrollCallback(std::function<void(int)> cb) {
        m_onPlaylistScroll = cb;
    }

    /**
     * @brief プレイリスト上がクリックされたときのコールバックを設定する
     */
    void SetPlaylistClickCallback(std::function<void(int, int)> cb) {
        m_onPlaylistClick = cb;
    }

    /**
     * @brief プレイリスト上がダブルクリックされたときのコールバックを設定する
     */
    void SetPlaylistDoubleClickCallback(std::function<void(int, int)> cb) {
        m_onPlaylistDoubleClick = cb;
    }


    void SetPlaylistToolbarClickCallback(std::function<void(int)> cb) {
        m_onPlaylistToolbarClick = cb;
    }

    /**
     * @brief プレイリストピン留め切り替えコールバック
     */
    void SetPlaylistPinnedToggleCallback(std::function<void(bool)> cb) {
        m_onPlaylistPinnedToggle = cb;
    }

    /**
     * @brief 背景フレーミングのスクロール（マウスホイール・キーボード）コールバック
     */
    void SetArtFramingScrollCallback(std::function<void(float)> cb) {
        m_onArtFramingScroll = cb;
    }

    /**
     * @brief 背景フレーミングの移動（ドラッグ・キーボード）コールバック
     */
    void SetArtFramingMoveCallback(std::function<void(float, float)> cb) {
        m_onArtFramingMove = cb;
    }

    /**
     * @brief 背景フレーミングのリセットコールバック
     */
    void SetArtFramingResetCallback(std::function<void()> cb) {
        m_onArtFramingReset = cb;
    }

    /**
     * @brief 背景フレーミングの保存コールバック
     */
    void SetArtFramingSaveCallback(std::function<void()> cb) {
        m_onArtFramingSave = cb;
    }


    enum HotkeyID {
        HK_NEXT_TRACK = 100,
        HK_PREV_TRACK,
        HK_PLAY_PAUSE,
        HK_STOP,
        HK_VOL_UP_5,
        HK_VOL_DOWN_5,
        HK_VOL_UP_25,
        HK_VOL_DOWN_25,
        HK_PREV_PLAYLIST,
        HK_NEXT_PLAYLIST,
        HK_ACTIVE_TOPMOST,
        HK_ACTIVE_BOTTOM,
        HK_EXIT_APP
    };

    void RegisterHotkeys();
    void UnregisterHotkeys();

    void SetHotkeyCallback(std::function<void(int)> cb) {
        m_onHotkey = cb;
    }

    /**
     * @brief ウィンドウサイズが変更された時のコールバックを設定する
     */
    void SetOnResizeCallback(std::function<void(int, int)> cb) {
        m_onResize = cb;
    }

    const std::vector<LogoMenuItem>& GetLogoMenuItems() const { return m_logoMenuItems; }
    std::vector<LogoMenuItem>& GetLogoMenuItemsMutable() { return m_logoMenuItems; }

    bool IsLogoMenuHovered() const { return m_isLogoMenuHovered; }
    void SetLogoMenuHovered(bool hovered) { m_isLogoMenuHovered = hovered; }
    bool IsInLogoMenuRegion(int x, int y, float progress) const;
    int GetLogoMenuButtonAt(int x, int y, float progress) const;
    int GetLogoMenuHoveredIndex() const { return m_logoMenuHoveredIndex; }

    int GetPlaybackHoveredIndex() const { return m_playbackHoveredIndex; }
    int GetPlaylistHoveredItemIndex() const { return m_playlistHoveredItemIndex; }
    void SetPlaylistHoveredItemIndex(int index) { m_playlistHoveredItemIndex = index; }

    bool IsPlaylistExpanded() const { return m_isPlaylistExpanded; }
    void SetPlaylistExpanded(bool expanded) { m_isPlaylistExpanded = expanded; }

    bool IsLogoMenuExpanded() const { return m_isLogoMenuExpanded; }
    void SetLogoMenuExpanded(bool expanded) { m_isLogoMenuExpanded = expanded; }
    const std::vector<std::wstring>& GetDynamicPlaylistPaths() const { return m_dynamicPlaylistPaths; }

    bool ConsumeLogoClicked() { bool b = m_isLogoClicked; m_isLogoClicked = false; return b; }
    int ConsumeLogoMenuClickedIndex() { int i = m_logoMenuClickedIndex; m_logoMenuClickedIndex = -1; return i; }
    int ConsumePlaybackClickedIndex() { int i = m_playbackClickedIndex; m_playbackClickedIndex = -1; return i; }

private:

    static LRESULT CALLBACK WindowProcStatic(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
    LRESULT WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

    HWND m_hwnd;
    HINSTANCE m_hInstance;
    const wchar_t* m_className = L"OZtoneWindowClass";

    ConfigManager* m_config;
    bool m_isHovered;
    bool m_isControlHovered;
    bool m_isVolumeHovered;
    bool m_isPlaylistHovered;
    bool m_isTrackingMouse;
    std::function<void(const std::vector<std::wstring>&)> m_onFilesDropped;
    std::function<void(int)> m_onMediaCommand;
    std::function<void(float)> m_onSkipCommand;
    std::function<void(const std::wstring&)> m_onCopyDataCallback;
    std::function<void(float)> m_onVolumeSetCommand;
    std::function<void(const std::wstring&)> m_onPlaylistSwitchCommand;
    std::function<void()> m_onShuffleCommand;
    std::function<void(int)> m_onVolumeScroll;
    std::function<void(int)> m_onPlaylistScroll;
    std::function<void(int, int)> m_onPlaylistClick;
    std::function<void(int, int)> m_onPlaylistDoubleClick;
    std::function<void(int)> m_onPlaylistToolbarClick;
    std::function<void(int)> m_onHotkey;
    std::function<void(int, int)> m_onResize;
    std::function<void(float)> m_onArtFramingScroll;
    std::function<void(float, float)> m_onArtFramingMove;
    std::function<void()> m_onArtFramingReset;
    std::function<void()> m_onArtFramingSave;
    std::function<void(bool)> m_onPlaylistPinnedToggle;
    DropTarget* m_pDropTarget;

    static constexpr UINT WM_TRAYICON = WM_APP + 1;
    static constexpr UINT WM_APP_MEDIAKEY = WM_APP + 2;
    NOTIFYICONDATAW m_nid;

    std::vector<std::wstring> m_dynamicPlaylistPaths;
    bool m_isLogoClicked = false;
    int m_logoMenuClickedIndex = -1;
    int m_playbackClickedIndex = -1;

    HHOOK m_keyboardHook;
    static HWND s_hwnd;
    static LRESULT CALLBACK LowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam);

    std::vector<LogoMenuItem> m_logoMenuItems;
    bool m_isLogoMenuHovered;
    int m_logoMenuHoveredIndex = -1;
    int m_playlistToolbarHoveredIndex = -1;
    bool m_isPlaylistPinnedHovered = false;
    int m_playbackHoveredIndex = -1;
    int m_playlistHoveredItemIndex = -1;
    bool m_isPlaylistExpanded = false;
    bool m_isLogoMenuExpanded = false;
    bool m_isArtFramingDragging = false;
    POINT m_artFramingDragStartPt = {0, 0};
};

