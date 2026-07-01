#include "Window.h"
#include "ConfigManager.h"
#include "resource.h"
#include <windowsx.h>
#include <shellapi.h>

constexpr UINT TRAY_MENU_ORDER[] = {
    Window::ID_TRAY_ZORDER_NORMAL,
    Window::ID_TRAY_ZORDER_TOPMOST,
    Window::ID_TRAY_ZORDER_BOTTOM,
    0, // separator
    Window::ID_TRAY_BG_NOWPLAYING,
    Window::ID_TRAY_BG_HIDDEN,
    Window::ID_TRAY_BG_DEFAULT,
    0, // separator
    Window::ID_TRAY_SAVE_POS,
    Window::ID_TRAY_RESET_POS,
    Window::ID_TRAY_RESET_ALL,
    Window::ID_TRAY_CLEAR_PLAYLIST,
    0, // separator
    Window::ID_TRAY_VIS_PRISM,
    Window::ID_TRAY_VIS_CIRCLE,
    0, // separator
    Window::ID_TRAY_EXIT
};

HWND Window::s_hwnd = nullptr;

Window::Window() : m_hwnd(nullptr), m_hInstance(nullptr), m_config(nullptr), m_isHovered(false), m_isControlHovered(false), m_isTrackingMouse(false), m_pDropTarget(nullptr), m_keyboardHook(nullptr) {}

Window::~Window() {
    if (m_keyboardHook) {
        UnhookWindowsHookEx(m_keyboardHook);
        m_keyboardHook = nullptr;
    }
    if (m_pDropTarget) {
        RevokeDragDrop(m_hwnd);
        m_pDropTarget->Release();
        m_pDropTarget = nullptr;
    }
    if (m_hwnd) {
        DestroyWindow(m_hwnd);
    }
}

DropTarget::DropTarget(Window* pWindow) : m_refCount(1), m_pWindow(pWindow) {}

HRESULT STDMETHODCALLTYPE DropTarget::QueryInterface(REFIID riid, void** ppvObject) {
    if (riid == IID_IUnknown || riid == IID_IDropTarget) {
        *ppvObject = this;
        AddRef();
        return S_OK;
    }
    *ppvObject = nullptr;
    return E_NOINTERFACE;
}

ULONG STDMETHODCALLTYPE DropTarget::AddRef() {
    return InterlockedIncrement(&m_refCount);
}

ULONG STDMETHODCALLTYPE DropTarget::Release() {
    LONG count = InterlockedDecrement(&m_refCount);
    if (count == 0) {
        delete this;
    }
    return count;
}

HRESULT STDMETHODCALLTYPE DropTarget::DragEnter(IDataObject* pDataObj, DWORD grfKeyState, POINTL pt, DWORD* pdwEffect) {
    POINT clientPt = { pt.x, pt.y };
    ScreenToClient(m_pWindow->GetHandle(), &clientPt);
    bool hovered = m_pWindow->IsInLogoRegion(clientPt.x, clientPt.y);
    m_pWindow->SetHovered(hovered);
    *pdwEffect = hovered ? DROPEFFECT_COPY : DROPEFFECT_NONE;
    return S_OK;
}

HRESULT STDMETHODCALLTYPE DropTarget::DragOver(DWORD grfKeyState, POINTL pt, DWORD* pdwEffect) {
    POINT clientPt = { pt.x, pt.y };
    ScreenToClient(m_pWindow->GetHandle(), &clientPt);
    bool hovered = m_pWindow->IsInLogoRegion(clientPt.x, clientPt.y);
    m_pWindow->SetHovered(hovered);
    *pdwEffect = hovered ? DROPEFFECT_COPY : DROPEFFECT_NONE;
    return S_OK;
}

HRESULT STDMETHODCALLTYPE DropTarget::DragLeave() {
    m_pWindow->SetHovered(false);
    return S_OK;
}

HRESULT STDMETHODCALLTYPE DropTarget::Drop(IDataObject* pDataObj, DWORD grfKeyState, POINTL pt, DWORD* pdwEffect) {
    m_pWindow->SetHovered(false);
    POINT clientPt = { pt.x, pt.y };
    ScreenToClient(m_pWindow->GetHandle(), &clientPt);

    if (m_pWindow->IsInLogoRegion(clientPt.x, clientPt.y)) {
        FORMATETC fmt = { CF_HDROP, nullptr, DVASPECT_CONTENT, -1, TYMED_HGLOBAL };
        STGMEDIUM stg = {0};
        if (SUCCEEDED(pDataObj->GetData(&fmt, &stg))) {
            HDROP hDrop = (HDROP)stg.hGlobal;
            UINT count = DragQueryFileW(hDrop, 0xFFFFFFFF, nullptr, 0);
            std::vector<std::wstring> files;
            for (UINT i = 0; i < count; ++i) {
                UINT length = DragQueryFileW(hDrop, i, nullptr, 0);
                std::wstring path(length, L'\0');
                DragQueryFileW(hDrop, i, &path[0], length + 1);
                files.push_back(path);
            }
            m_pWindow->NotifyFilesDropped(files);
            ReleaseStgMedium(&stg);
        }
        *pdwEffect = DROPEFFECT_COPY;
    } else {
        *pdwEffect = DROPEFFECT_NONE;
    }
    return S_OK;
}

bool Window::Initialize(HINSTANCE hInstance, int nCmdShow, ConfigManager& config) {
    m_hInstance = hInstance;
    m_config = &config;

    WNDCLASSEXW wc = {};
    wc.cbSize = sizeof(WNDCLASSEXW);
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = WindowProcStatic;
    wc.hInstance = m_hInstance;
    wc.hIcon = LoadIcon(m_hInstance, MAKEINTRESOURCE(IDI_APP_ICON));
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.lpszClassName = m_className;
    wc.hIconSm = LoadIcon(m_hInstance, MAKEINTRESOURCE(IDI_APP_ICON));

    if (!RegisterClassExW(&wc)) {
        return false;
    }

    // 設定からウィンドウスタイルを決定
    DWORD dwStyle = 0;
    DWORD dwExStyle = 0;

    if (config.GetShowTitleBar() && config.GetShowWindowFrame()) {
        dwStyle = WS_OVERLAPPEDWINDOW;
    } else if (config.GetShowTitleBar()) {
        dwStyle = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX;
    } else if (config.GetShowWindowFrame()) {
        dwStyle = WS_POPUP | WS_THICKFRAME;
    } else {
        dwStyle = WS_POPUP; // 完全枠なし
        dwExStyle |= WS_EX_LAYERED;
    }

    if (!config.GetShowTaskbar()) {
        dwExStyle |= WS_EX_TOOLWINDOW;
    } else {
        dwExStyle |= WS_EX_APPWINDOW;
    }

    // ConfigManagerからサイズ・座標を取得しDPIスケーリング
    int x = config.GetWindowX();
    int y = config.GetWindowY();
    int width = config.GetWindowWidth();
    int height = config.GetWindowHeight();

    UINT dpi = GetDpiForSystem();
    int scaledWidth = MulDiv(width, dpi, 96);
    int scaledHeight = MulDiv(height, dpi, 96);

    int scaledX = (x == CW_USEDEFAULT) ? CW_USEDEFAULT : MulDiv(x, dpi, 96);
    int scaledY = (y == CW_USEDEFAULT) ? CW_USEDEFAULT : MulDiv(y, dpi, 96);

    RECT rect = { 0, 0, scaledWidth, scaledHeight };
    AdjustWindowRectExForDpi(&rect, dwStyle, FALSE, dwExStyle, dpi);

    m_hwnd = CreateWindowExW(
        dwExStyle,
        m_className,
        L"OZtone",
        dwStyle,
        scaledX, scaledY,
        rect.right - rect.left, rect.bottom - rect.top,
        nullptr, nullptr, m_hInstance, this
    );

    if (!m_hwnd) {
        return false;
    }

    s_hwnd = m_hwnd;
    m_keyboardHook = SetWindowsHookExW(WH_KEYBOARD_LL, LowLevelKeyboardProc, m_hInstance, 0);

    ShowWindow(m_hwnd, nCmdShow);
    UpdateWindow(m_hwnd);

    // Initialize Z-Order
    int zOrder = m_config->GetZOrder();
    HWND hWndInsertAfter = HWND_NOTOPMOST;
    if (zOrder == 1) hWndInsertAfter = HWND_TOPMOST;
    else if (zOrder == 2) hWndInsertAfter = HWND_BOTTOM;
    SetWindowPos(m_hwnd, hWndInsertAfter, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
    
    // OLE Drag and Drop を有効化
    m_pDropTarget = new DropTarget(this);
    RegisterDragDrop(m_hwnd, m_pDropTarget);

    m_nid = {};
    m_nid.cbSize = sizeof(NOTIFYICONDATAW);
    m_nid.hWnd = m_hwnd;
    m_nid.uID = 1;
    m_nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
    m_nid.uCallbackMessage = WM_TRAYICON;
    m_nid.hIcon = LoadIcon(m_hInstance, MAKEINTRESOURCE(IDI_APP_ICON));
    wcscpy_s(m_nid.szTip, L"OZtone");
    Shell_NotifyIconW(NIM_ADD, &m_nid);

    return true;
}

bool Window::ProcessMessages() {
    MSG msg = {};
    while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
        if (msg.message == WM_QUIT) {
            return false;
        }
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return true;
}

LRESULT CALLBACK Window::WindowProcStatic(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    Window* pThis = nullptr;

    if (uMsg == WM_NCCREATE) {
        CREATESTRUCT* pCreate = reinterpret_cast<CREATESTRUCT*>(lParam);
        pThis = reinterpret_cast<Window*>(pCreate->lpCreateParams);
        SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pThis));
    } else {
        pThis = reinterpret_cast<Window*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
    }

    if (pThis) {
        return pThis->WindowProc(hwnd, uMsg, wParam, lParam);
    }

    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

LRESULT CALLBACK Window::LowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam) {
    if (nCode == HC_ACTION) {
        if (wParam == WM_KEYDOWN || wParam == WM_SYSKEYDOWN) {
            KBDLLHOOKSTRUCT* pKeyBoard = reinterpret_cast<KBDLLHOOKSTRUCT*>(lParam);
            DWORD vkCode = pKeyBoard->vkCode;
            if (vkCode == VK_MEDIA_PLAY_PAUSE ||
                vkCode == VK_MEDIA_STOP ||
                vkCode == VK_MEDIA_NEXT_TRACK ||
                vkCode == VK_MEDIA_PREV_TRACK) {
                PostMessage(s_hwnd, WM_APP_MEDIAKEY, vkCode, 0);
            }
        }
    }
    return CallNextHookEx(nullptr, nCode, wParam, lParam);
}

bool Window::IsInLogoRegion(int x, int y) const {
    if (!m_config || !m_hwnd) return false;
    
    UINT dpi = GetDpiForWindow(m_hwnd);
    int logicalX = MulDiv(x, 96, dpi);
    int logicalY = MulDiv(y, 96, dpi);

    return (logicalX >= m_config->GetLogoX() && logicalX <= m_config->GetLogoX() + m_config->GetLogoWidth() &&
            logicalY >= m_config->GetLogoY() && logicalY <= m_config->GetLogoY() + m_config->GetLogoHeight());
}

bool Window::IsInPlaybackControlRegion(int x, int y) const {
    if (!m_config || !m_hwnd) return false;
    
    UINT dpi = GetDpiForWindow(m_hwnd);
    int logicalX = MulDiv(x, 96, dpi);
    int logicalY = MulDiv(y, 96, dpi);

    RECT rect;
    GetClientRect(m_hwnd, &rect);
    int logicalWidth = MulDiv(rect.right - rect.left, 96, dpi);
    int logicalHeight = MulDiv(rect.bottom - rect.top, 96, dpi);

    return (logicalY >= logicalHeight - m_config->GetControlHoverHeight());
}

bool Window::IsInVolumeControlRegion(int x, int y) const {
    if (!m_config || !m_hwnd) return false;
    UINT dpi = GetDpiForWindow(m_hwnd);
    int logicalX = MulDiv(x, 96, dpi);
    int logicalY = MulDiv(y, 96, dpi);
    RECT rect;
    GetClientRect(m_hwnd, &rect);
    int logicalHeight = MulDiv(rect.bottom - rect.top, 96, dpi);

    float volX = static_cast<float>(m_config->GetVolumeBaseLeftOffset());
    float volY = logicalHeight - static_cast<float>(m_config->GetVolumeBaseBottomOffset());
    float size = static_cast<float>(m_config->GetVolumeIconSize());
    float width = size + 80.0f; 
    float height = size + 20.0f;

    return (logicalX >= volX && logicalX <= volX + width && logicalY >= volY - height/2 && logicalY <= volY + height/2);
}

int Window::GetPlaybackButtonAt(int x, int y) const {
    if (!m_config || !m_hwnd) return 0;
    
    UINT dpi = GetDpiForWindow(m_hwnd);
    int logicalX = MulDiv(x, 96, dpi);
    int logicalY = MulDiv(y, 96, dpi);

    RECT rect;
    GetClientRect(m_hwnd, &rect);
    int logicalWidth = MulDiv(rect.right - rect.left, 96, dpi);
    int logicalHeight = MulDiv(rect.bottom - rect.top, 96, dpi);

    float centerX = (logicalWidth / 2.0f) + m_config->GetPlaybackCenterOffsetX();
    float centerY = logicalHeight - m_config->GetPlaybackBaseBottomOffset();
    float size = m_config->GetPlaybackButtonSize();
    float spacing = m_config->GetPlaybackButtonSpacing();
    float halfSize = size / 2.0f;

    if (logicalY >= centerY - halfSize && logicalY <= centerY + halfSize) {
        if (logicalX >= centerX - spacing - halfSize && logicalX <= centerX - spacing + halfSize) return 1; // Previous
        if (logicalX >= centerX - halfSize && logicalX <= centerX + halfSize) return 2; // Play/Pause
        if (logicalX >= centerX + spacing - halfSize && logicalX <= centerX + spacing + halfSize) return 3; // Next
    }
    return 0;
}


LRESULT Window::WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
        case WM_WINDOWPOSCHANGING: {
            if (m_config && m_config->GetZOrder() == 2) {
                WINDOWPOS* pos = reinterpret_cast<WINDOWPOS*>(lParam);
                pos->hwndInsertAfter = HWND_BOTTOM;
            }
            break;
        }
        case WM_MOUSEACTIVATE: {
            if (m_config && m_config->GetZOrder() == 2) {
                return MA_NOACTIVATE;
            }
            break;
        }
        case WM_MOUSEMOVE: {
            if (m_config) {
                int xPos = GET_X_LPARAM(lParam);
                int yPos = GET_Y_LPARAM(lParam);

                m_isHovered = IsInLogoRegion(xPos, yPos);
                m_isControlHovered = IsInPlaybackControlRegion(xPos, yPos);

                if (!m_isTrackingMouse) {
                    TRACKMOUSEEVENT tme = {};
                    tme.cbSize = sizeof(TRACKMOUSEEVENT);
                    tme.dwFlags = TME_LEAVE;
                    tme.hwndTrack = hwnd;
                    TrackMouseEvent(&tme);
                    m_isTrackingMouse = true;
                }
            }
            return 0;
        }
        case WM_MOUSELEAVE: {
            m_isHovered = false;
            m_isControlHovered = false;
            m_isTrackingMouse = false;
            return 0;
        }
        case WM_LBUTTONDOWN: {
            int xPos = GET_X_LPARAM(lParam);
            int yPos = GET_Y_LPARAM(lParam);
            
            int btnId = GetPlaybackButtonAt(xPos, yPos);
            if (btnId > 0) {
                if (m_onMediaCommand) {
                    if (btnId == 1) m_onMediaCommand(APPCOMMAND_MEDIA_PREVIOUSTRACK);
                    else if (btnId == 2) m_onMediaCommand(APPCOMMAND_MEDIA_PLAY_PAUSE);
                    else if (btnId == 3) m_onMediaCommand(APPCOMMAND_MEDIA_NEXTTRACK);
                }
                return 0;
            }

            if (IsInLogoRegion(xPos, yPos)) {
                ReleaseCapture();
                SendMessage(hwnd, WM_NCLBUTTONDOWN, HTCAPTION, 0);
            }
            return 0;
        }
        case WM_MOUSEWHEEL: {
            if (m_config && m_onVolumeScroll) {
                POINT pt;
                pt.x = GET_X_LPARAM(lParam);
                pt.y = GET_Y_LPARAM(lParam);
                ScreenToClient(hwnd, &pt);
                if (IsInVolumeControlRegion(pt.x, pt.y)) {
                    int zDelta = GET_WHEEL_DELTA_WPARAM(wParam);
                    m_onVolumeScroll(zDelta);
                    return 0;
                }
            }
            break;
        }
        case WM_TRAYICON: {
            if (lParam == WM_RBUTTONUP) {
                POINT pt;
                GetCursorPos(&pt);
                HMENU hMenu = CreatePopupMenu();
                
                for (UINT id : TRAY_MENU_ORDER) {
                    if (id == 0) {
                        AppendMenuW(hMenu, MF_SEPARATOR, 0, nullptr);
                    } else {
                        std::wstring text;
                        switch (id) {
                            case ID_TRAY_ZORDER_NORMAL: text = L"ウィンドウ順序: 通常"; break;
                            case ID_TRAY_ZORDER_TOPMOST: text = L"ウィンドウ順序: 最前面"; break;
                            case ID_TRAY_ZORDER_BOTTOM: text = L"ウィンドウ順序: 最背面"; break;
                            case ID_TRAY_SAVE_POS: text = L"終了時に位置とサイズを記憶"; break;
                            case ID_TRAY_RESET_POS: text = L"位置とサイズをリセット"; break;
                            case ID_TRAY_RESET_ALL: text = L"設定を初期化"; break;
                            case ID_TRAY_CLEAR_PLAYLIST: text = L"プレイリストをクリア"; break;
                            case ID_TRAY_BG_NOWPLAYING: text = L"背景: アルバムアート"; break;
                            case ID_TRAY_BG_HIDDEN: text = L"背景: 非表示"; break;
                            case ID_TRAY_BG_DEFAULT: text = L"背景: デフォルト背景"; break;
                            case ID_TRAY_VIS_PRISM: text = L"ビジュアライザ: プリズム・ビート"; break;
                            case ID_TRAY_VIS_CIRCLE: text = L"ビジュアライザ: ヘイロー・ダスト"; break;
                            case ID_TRAY_EXIT: text = L"終了 (Exit)"; break;
                        }
                        AppendMenuW(hMenu, MF_STRING, id, text.c_str());
                    }
                }

                if (m_config) {
                    int zOrder = m_config->GetZOrder();
                    CheckMenuRadioItem(hMenu, ID_TRAY_ZORDER_NORMAL, ID_TRAY_ZORDER_BOTTOM, 
                                       ID_TRAY_ZORDER_NORMAL + zOrder, MF_BYCOMMAND);
                    int bgMode = m_config->GetBackgroundArtMode();
                    CheckMenuRadioItem(hMenu, ID_TRAY_BG_NOWPLAYING, ID_TRAY_BG_DEFAULT,
                                       ID_TRAY_BG_NOWPLAYING + bgMode, MF_BYCOMMAND);
                    int visMode = m_config->GetVisualizerMode();
                    CheckMenuRadioItem(hMenu, ID_TRAY_VIS_PRISM, ID_TRAY_VIS_CIRCLE,
                                       ID_TRAY_VIS_PRISM + visMode, MF_BYCOMMAND);
                    if (m_config->GetSavePositionOnExit()) {
                        CheckMenuItem(hMenu, ID_TRAY_SAVE_POS, MF_BYCOMMAND | MF_CHECKED);
                    }
                }

                SetForegroundWindow(hwnd);
                TrackPopupMenu(hMenu, TPM_RIGHTBUTTON | TPM_BOTTOMALIGN, pt.x, pt.y, 0, hwnd, nullptr);
                DestroyMenu(hMenu);
            }
            return 0;
        }
        case WM_COMMAND: {
            int wmId = LOWORD(wParam);
            switch (wmId) {
                case ID_TRAY_EXIT:
                    PostMessage(hwnd, WM_CLOSE, 0, 0);
                    break;
                case ID_TRAY_ZORDER_NORMAL:
                case ID_TRAY_ZORDER_TOPMOST:
                case ID_TRAY_ZORDER_BOTTOM: {
                    int zOrder = wmId - ID_TRAY_ZORDER_NORMAL;
                    if (m_config) m_config->SetZOrder(zOrder);
                    HWND insertAfter = HWND_NOTOPMOST;
                    if (zOrder == 1) insertAfter = HWND_TOPMOST;
                    else if (zOrder == 2) insertAfter = HWND_BOTTOM;
                    SetWindowPos(hwnd, insertAfter, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
                    break;
                }
                case ID_TRAY_BG_NOWPLAYING:
                case ID_TRAY_BG_HIDDEN:
                case ID_TRAY_BG_DEFAULT: {
                    if (m_config) {
                        m_config->SetBackgroundArtMode(wmId - ID_TRAY_BG_NOWPLAYING);
                    }
                    break;
                }
                case ID_TRAY_VIS_PRISM:
                case ID_TRAY_VIS_CIRCLE: {
                    if (m_config) {
                        m_config->SetVisualizerMode(wmId - ID_TRAY_VIS_PRISM);
                    }
                    break;
                }
                case ID_TRAY_SAVE_POS: {
                    if (m_config) {
                        bool current = m_config->GetSavePositionOnExit();
                        m_config->SetSavePositionOnExit(!current);
                    }
                    break;
                }
                case ID_TRAY_RESET_POS: {
                    UINT dpi = GetDpiForSystem();
                    int width = MulDiv(1024, dpi, 96);
                    int height = MulDiv(512, dpi, 96);
                    int screenW = GetSystemMetrics(SM_CXSCREEN);
                    int screenH = GetSystemMetrics(SM_CYSCREEN);
                    int x = (screenW - width) / 2;
                    int y = (screenH - height) / 2;
                    SetWindowPos(hwnd, nullptr, x, y, width, height, SWP_NOZORDER | SWP_NOACTIVATE);
                    break;
                }
                case ID_TRAY_RESET_ALL: {
                    if (m_config) {
                        m_config->SaveDefaultSettings();
                    }
                    MessageBoxW(hwnd, L"設定を初期化しました。アプリを再起動します。", L"通知", MB_OK | MB_ICONINFORMATION);
                    PostMessage(hwnd, WM_CLOSE, 0, 0);
                    break;
                }
                case ID_TRAY_CLEAR_PLAYLIST: {
                    if (m_onClearPlaylistCommand) {
                        m_onClearPlaylistCommand();
                    }
                    break;
                }
            }
            return 0;
        }
        case WM_APP_MEDIAKEY: {
            if (m_onMediaCommand) {
                DWORD vkCode = static_cast<DWORD>(wParam);
                int cmd = 0;
                if (vkCode == VK_MEDIA_PLAY_PAUSE) {
                    cmd = APPCOMMAND_MEDIA_PLAY_PAUSE;
                } else if (vkCode == VK_MEDIA_STOP) {
                    cmd = APPCOMMAND_MEDIA_STOP;
                } else if (vkCode == VK_MEDIA_NEXT_TRACK) {
                    cmd = APPCOMMAND_MEDIA_NEXTTRACK;
                } else if (vkCode == VK_MEDIA_PREV_TRACK) {
                    cmd = APPCOMMAND_MEDIA_PREVIOUSTRACK;
                }
                
                if (cmd != 0) {
                    m_onMediaCommand(cmd);
                }
            }
            return 0;
        }
        case WM_COPYDATA: {
            COPYDATASTRUCT* pcds = reinterpret_cast<COPYDATASTRUCT*>(lParam);
            if (pcds && pcds->lpData && m_onCopyDataCallback) {
                std::wstring path(reinterpret_cast<LPCWSTR>(pcds->lpData), pcds->cbData / sizeof(wchar_t));
                if (!path.empty() && path.back() == L'\0') {
                    path.pop_back();
                }
                m_onCopyDataCallback(path);
            }
            return 1;
        }
        case WM_DESTROY: {
            Shell_NotifyIconW(NIM_DELETE, &m_nid);
            if (m_config && m_config->GetSavePositionOnExit()) {
                RECT wndRect, clientRect;
                if (GetWindowRect(hwnd, &wndRect) && GetClientRect(hwnd, &clientRect)) {
                    UINT dpi = GetDpiForWindow(hwnd);
                    int logicalX = MulDiv(wndRect.left, 96, dpi);
                    int logicalY = MulDiv(wndRect.top, 96, dpi);
                    int logicalWidth = MulDiv(clientRect.right - clientRect.left, 96, dpi);
                    int logicalHeight = MulDiv(clientRect.bottom - clientRect.top, 96, dpi);
                    m_config->SaveWindowPosition(logicalX, logicalY, logicalWidth, logicalHeight);
                }
            }
            PostQuitMessage(0);
            return 0;
        }
    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}
