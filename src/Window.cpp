#include "Window.h"
#include "ConfigManager.h"
#include "resource.h"
#include <filesystem>
#include <shellapi.h>
#include <windowsx.h>

constexpr UINT TRAY_MENU_ORDER[] = {Window::ID_TRAY_ZORDER_NORMAL,
                                    Window::ID_TRAY_ZORDER_TOPMOST,
                                    Window::ID_TRAY_ZORDER_BOTTOM,
                                    0, // separator
                                    Window::ID_TRAY_BG_NOWPLAYING,
                                    Window::ID_TRAY_BG_HIDDEN,
                                    Window::ID_TRAY_BG_DEFAULT,
                                    0, // separator
                                    Window::ID_TRAY_VIS_NONE,
                                    Window::ID_TRAY_VIS_PRISM,
                                    Window::ID_TRAY_VIS_CIRCLE,
                                    0, // separator
                                    Window::ID_TRAY_PLAYLIST_MENU,
                                    Window::ID_TRAY_NEW_PLAYLIST,
                                    Window::ID_TRAY_CLEAR_PLAYLIST,
                                    0, // separator
                                    Window::ID_TRAY_ENABLE_RESIZE,
                                    Window::ID_TRAY_SAVE_POS,
                                    Window::ID_TRAY_RESET_POS,
                                    Window::ID_TRAY_RESET_ALL,
                                    0, // separator
                                    Window::ID_TRAY_EXIT};

HWND Window::s_hwnd = nullptr;

Window::Window()
    : m_hwnd(nullptr), m_hInstance(nullptr), m_config(nullptr),
      m_isHovered(false), m_isControlHovered(false), m_isPlaylistHovered(false),
      m_isTrackingMouse(false), m_pDropTarget(nullptr), m_keyboardHook(nullptr),
      m_isLogoMenuHovered(false) {
  m_logoMenuItems = {
      {ID_LOGO_EXIT, L"❌", false, false, L"OZtoneの終了"},
      {ID_LOGO_CLEAR, L"🗑️", false, false,
       L"再生中の曲をプレイリストから削除する"},
      {ID_LOGO_PIN_PLAYLIST, L"📜", true, false, L"プレイリスト固定表示"},
      {ID_LOGO_VISUALIZER, L"🎆", true, false, L"ビジュアライザ表示切り替え"},
      {ID_LOGO_SHUFFLE, L"🔀", true, false, L"シャッフル再生ON/OFF"},
      {ID_LOGO_PLAYLIST_POS, L"↔️", false, false, L"プレイリストの配置場所切り替え"}};
}

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

DropTarget::DropTarget(Window *pWindow) : m_refCount(1), m_pWindow(pWindow) {}

HRESULT STDMETHODCALLTYPE DropTarget::QueryInterface(REFIID riid,
                                                     void **ppvObject) {
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

HRESULT STDMETHODCALLTYPE DropTarget::DragEnter(IDataObject *pDataObj,
                                                DWORD grfKeyState, POINTL pt,
                                                DWORD *pdwEffect) {
  POINT clientPt = {pt.x, pt.y};
  ScreenToClient(m_pWindow->GetHandle(), &clientPt);
  bool hovered = m_pWindow->IsInLogoRegion(clientPt.x, clientPt.y);
  m_pWindow->SetHovered(hovered);
  *pdwEffect = hovered ? DROPEFFECT_COPY : DROPEFFECT_NONE;
  return S_OK;
}

HRESULT STDMETHODCALLTYPE DropTarget::DragOver(DWORD grfKeyState, POINTL pt,
                                               DWORD *pdwEffect) {
  POINT clientPt = {pt.x, pt.y};
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

HRESULT STDMETHODCALLTYPE DropTarget::Drop(IDataObject *pDataObj,
                                           DWORD grfKeyState, POINTL pt,
                                           DWORD *pdwEffect) {
  m_pWindow->SetHovered(false);
  POINT clientPt = {pt.x, pt.y};
  ScreenToClient(m_pWindow->GetHandle(), &clientPt);

  if (m_pWindow->IsInLogoRegion(clientPt.x, clientPt.y)) {
    FORMATETC fmt = {CF_HDROP, nullptr, DVASPECT_CONTENT, -1, TYMED_HGLOBAL};
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

bool Window::Initialize(HINSTANCE hInstance, int nCmdShow,
                        ConfigManager &config) {
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
    dwExStyle |= WS_EX_LAYERED | WS_EX_NOREDIRECTIONBITMAP;
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

  RECT rect = {0, 0, scaledWidth, scaledHeight};
  AdjustWindowRectExForDpi(&rect, dwStyle, FALSE, dwExStyle, dpi);

  m_hwnd =
      CreateWindowExW(dwExStyle, m_className, L"OZtone", dwStyle, scaledX,
                      scaledY, rect.right - rect.left, rect.bottom - rect.top,
                      nullptr, nullptr, m_hInstance, this);

  if (!m_hwnd) {
    return false;
  }

  // DirectComposition使用時に、WS_EX_LAYEREDウィンドウの入力判定領域が
  // リサイズ時に自動更新されないOSの仕様を回避するため、LWA_ALPHAを設定する。
  if (dwExStyle & WS_EX_LAYERED) {
    SetLayeredWindowAttributes(m_hwnd, 0, 255, LWA_ALPHA);
  }

  s_hwnd = m_hwnd;
  m_keyboardHook =
      SetWindowsHookExW(WH_KEYBOARD_LL, LowLevelKeyboardProc, m_hInstance, 0);

  ShowWindow(m_hwnd, nCmdShow);
  UpdateWindow(m_hwnd);

  // Initialize Z-Order
  int zOrder = m_config->GetZOrder();
  HWND hWndInsertAfter = HWND_NOTOPMOST;
  if (zOrder == 1)
    hWndInsertAfter = HWND_TOPMOST;
  else if (zOrder == 2)
    hWndInsertAfter = HWND_BOTTOM;
  SetWindowPos(m_hwnd, hWndInsertAfter, 0, 0, 0, 0,
               SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);

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

LRESULT CALLBACK Window::WindowProcStatic(HWND hwnd, UINT uMsg, WPARAM wParam,
                                          LPARAM lParam) {
  Window *pThis = nullptr;

  if (uMsg == WM_NCCREATE) {
    CREATESTRUCT *pCreate = reinterpret_cast<CREATESTRUCT *>(lParam);
    pThis = reinterpret_cast<Window *>(pCreate->lpCreateParams);
    SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pThis));
  } else {
    pThis = reinterpret_cast<Window *>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
  }

  if (pThis) {
    return pThis->WindowProc(hwnd, uMsg, wParam, lParam);
  }

  return DefWindowProcW(hwnd, uMsg, wParam, lParam);
}

LRESULT CALLBACK Window::LowLevelKeyboardProc(int nCode, WPARAM wParam,
                                              LPARAM lParam) {
  if (nCode == HC_ACTION) {
    if (wParam == WM_KEYDOWN || wParam == WM_SYSKEYDOWN) {
      KBDLLHOOKSTRUCT *pKeyBoard = reinterpret_cast<KBDLLHOOKSTRUCT *>(lParam);
      DWORD vkCode = pKeyBoard->vkCode;
      if (vkCode == VK_MEDIA_PLAY_PAUSE || vkCode == VK_MEDIA_STOP ||
          vkCode == VK_MEDIA_NEXT_TRACK || vkCode == VK_MEDIA_PREV_TRACK) {
        PostMessage(s_hwnd, WM_APP_MEDIAKEY, vkCode, 0);
      }
    }
  }
  return CallNextHookEx(nullptr, nCode, wParam, lParam);
}

bool Window::IsInLogoRegion(int x, int y) const {
  if (!m_config || !m_hwnd)
    return false;

  UINT dpi = GetDpiForWindow(m_hwnd);
  int logicalX = MulDiv(x, 96, dpi);
  int logicalY = MulDiv(y, 96, dpi);

  return (logicalX >= m_config->GetLogoX() &&
          logicalX <= m_config->GetLogoX() + m_config->GetLogoWidth() &&
          logicalY >= m_config->GetLogoY() &&
          logicalY <= m_config->GetLogoY() + m_config->GetLogoHeight());
}

#include "LayoutCalculator.h"

bool Window::IsInLogoMenuRegion(int x, int y, float progress) const {
  if (!m_config || !m_hwnd)
    return false;

  UINT dpi = GetDpiForWindow(m_hwnd);
  int logicalX = MulDiv(x, 96, dpi);
  int logicalY = MulDiv(y, 96, dpi);

  LogoMenuLayout layout = LayoutCalculator::CalculateLogoMenuLayout(
      m_config, progress, m_logoMenuItems.size());

  return (logicalX >= layout.fullRegionRect.left &&
          logicalX <= layout.fullRegionRect.right &&
          logicalY >= layout.fullRegionRect.top &&
          logicalY <= layout.fullRegionRect.bottom);
}

int Window::GetLogoMenuButtonAt(int x, int y, float progress) const {
  if (!m_config || !m_hwnd)
    return -1;

  UINT dpi = GetDpiForWindow(m_hwnd);
  int logicalX = MulDiv(x, 96, dpi);
  int logicalY = MulDiv(y, 96, dpi);

  LogoMenuLayout layout = LayoutCalculator::CalculateLogoMenuLayout(
      m_config, progress, m_logoMenuItems.size());
  for (size_t i = 0; i < layout.items.size(); ++i) {
    if (logicalX >= layout.items[i].hitRect.left &&
        logicalX <= layout.items[i].hitRect.right &&
        logicalY >= layout.items[i].hitRect.top &&
        logicalY <= layout.items[i].hitRect.bottom) {
      return static_cast<int>(i);
    }
  }
  return -1;
}

bool Window::IsInPlaybackControlRegion(int x, int y) const {
  if (!m_config || !m_hwnd)
    return false;

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
  if (!m_config || !m_hwnd)
    return false;
  UINT dpi = GetDpiForWindow(m_hwnd);
  int logicalX = MulDiv(x, 96, dpi);
  int logicalY = MulDiv(y, 96, dpi);
  RECT rect;
  GetClientRect(m_hwnd, &rect);
  int logicalHeight = MulDiv(rect.bottom - rect.top, 96, dpi);

  float volX = static_cast<float>(m_config->GetVolumeBaseLeftOffset());
  float volY =
      logicalHeight - static_cast<float>(m_config->GetVolumeBaseBottomOffset());
  float size = static_cast<float>(m_config->GetVolumeIconSize());
  float width = size + 80.0f;
  float height = size + 20.0f;

  return (logicalX >= volX && logicalX <= volX + width &&
          logicalY >= volY - height / 2 && logicalY <= volY + height / 2);
}

bool Window::IsInPlaylistRegion(int x, int y) const {
  if (!m_isPlaylistHovered && IsInLogoRegion(x, y))
    return false;

  if (!m_config || !m_hwnd)
    return false;
  UINT dpi = GetDpiForWindow(m_hwnd);
  int logicalX = MulDiv(x, 96, dpi);
  int logicalY = MulDiv(y, 96, dpi);
  RECT rect;
  GetClientRect(m_hwnd, &rect);
  int logicalWidth = MulDiv(rect.right - rect.left, 96, dpi);
  int logicalHeight = MulDiv(rect.bottom - rect.top, 96, dpi);

  // ロゴ拡張メニュー等との干渉排除
  if (!m_isPlaylistHovered && logicalY <= m_config->GetLogoY() + m_config->GetLogoHeight() + 10.0f) {
    return false;
  }

  float hoverWidth =
      m_isPlaylistHovered
          ? static_cast<float>(m_config->GetPlaylistWidth())
          : static_cast<float>(m_config->GetPlaylistHoverWidth());
  float controlHeight = m_config->GetControlHoverHeight();

  bool isXMatch = false;
  if (m_config->GetPlaylistPosition() == 0) {
    isXMatch = logicalX <= hoverWidth;
  } else {
    isXMatch = logicalX >= logicalWidth - hoverWidth;
  }

  // プレイリストが展開されている場合は、画面下部であってもリスト上にマウスがあればホバーを維持する。
  // 展開されていない場合のみ、右下のリサイズやコントロールとの干渉を避けるためY座標を制限する。
  bool isYMatch =
      m_isPlaylistHovered || (logicalY < logicalHeight - controlHeight);

  return isXMatch && isYMatch;
}

int Window::GetPlaylistToolbarButtonAt(int x, int y) const {
  if (!m_config || !m_hwnd || !m_isPlaylistHovered) return -1;
  UINT dpi = GetDpiForWindow(m_hwnd);
  int logicalX = MulDiv(x, 96, dpi);
  int logicalY = MulDiv(y, 96, dpi);
  
  RECT rect;
  GetClientRect(m_hwnd, &rect);
  int logicalWidth = MulDiv(rect.right - rect.left, 96, dpi);
  int logicalHeight = MulDiv(rect.bottom - rect.top, 96, dpi);
  
  PlaylistLayout layout = LayoutCalculator::CalculatePlaylistLayout(logicalWidth, logicalHeight, m_config, 0.0f, 0.0f, 0, 0);
  for (int i = 0; i < 3; ++i) {
      if (logicalX >= layout.toolbarLayout.buttonHitRects[i].left &&
          logicalX <= layout.toolbarLayout.buttonHitRects[i].right &&
          logicalY >= layout.toolbarLayout.buttonHitRects[i].top &&
          logicalY <= layout.toolbarLayout.buttonHitRects[i].bottom) {
          return i;
      }
  }
  return -1;
}

int Window::GetPlaybackButtonAt(int x, int y) const {
  if (!m_config || !m_hwnd)
    return 0;

  UINT dpi = GetDpiForWindow(m_hwnd);
  int logicalX = MulDiv(x, 96, dpi);
  int logicalY = MulDiv(y, 96, dpi);

  RECT rect;
  GetClientRect(m_hwnd, &rect);
  int logicalWidth = MulDiv(rect.right - rect.left, 96, dpi);
  int logicalHeight = MulDiv(rect.bottom - rect.top, 96, dpi);

  float centerX = (logicalWidth / 2.0f) + m_config->GetPlaybackCenterOffsetX();
  float centerY = logicalHeight -
                  static_cast<float>(m_config->GetPlaybackBaseBottomOffset());
  float size = static_cast<float>(m_config->GetPlaybackButtonSize());
  float spacing = static_cast<float>(m_config->GetPlaybackButtonSpacing());
  float halfSize = size / 2.0f;

  if (logicalY >= centerY - halfSize && logicalY <= centerY + halfSize) {
    if (logicalX >= centerX - spacing - halfSize &&
        logicalX <= centerX - spacing + halfSize)
      return 1; // Previous
    if (logicalX >= centerX - halfSize && logicalX <= centerX + halfSize)
      return 2; // Play/Pause
    if (logicalX >= centerX + spacing - halfSize &&
        logicalX <= centerX + spacing + halfSize)
      return 3; // Next
  }
  return 0;
}

LRESULT Window::WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
  switch (uMsg) {
  case WM_GETMINMAXINFO: {
    MINMAXINFO *pMinMaxInfo = reinterpret_cast<MINMAXINFO *>(lParam);
    UINT dpi = GetDpiForWindow(hwnd);
    if (dpi == 0)
      dpi = 96;
    pMinMaxInfo->ptMinTrackSize.x = MulDiv(495, dpi, 96);
    pMinMaxInfo->ptMinTrackSize.y = MulDiv(260, dpi, 96);
    return 0;
  }
  case WM_SIZE: {
    if (wParam != SIZE_MINIMIZED && m_onResize) {
      int width = LOWORD(lParam);
      int height = HIWORD(lParam);
      m_onResize(width, height);
    }
    return 0;
  }
  case WM_NCHITTEST: {
    LRESULT hit = DefWindowProc(hwnd, uMsg, wParam, lParam);
    if (hit == HTCLIENT && m_config && m_config->GetEnableResize()) {
      POINT pt;
      pt.x = GET_X_LPARAM(lParam);
      pt.y = GET_Y_LPARAM(lParam);
      ScreenToClient(hwnd, &pt);

      RECT rect;
      GetClientRect(hwnd, &rect);

      UINT dpi = GetDpiForWindow(hwnd);
      int gripSize = MulDiv(15, dpi, 96);

      if (pt.x >= rect.right - gripSize && pt.y >= rect.bottom - gripSize) {
        return HTBOTTOMRIGHT;
      }
    }
    return hit;
  }
  case WM_WINDOWPOSCHANGING: {
    if (m_config && m_config->GetZOrder() == 2) {
      WINDOWPOS *pos = reinterpret_cast<WINDOWPOS *>(lParam);
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

      m_isPlaylistHovered = IsInPlaylistRegion(xPos, yPos);

      if (m_isPlaylistHovered) {
        m_playlistToolbarHoveredIndex = GetPlaylistToolbarButtonAt(xPos, yPos);
        m_isHovered = false;
        m_isControlHovered = false;
        m_isLogoMenuHovered = false;
      } else {
        m_playlistToolbarHoveredIndex = -1;
        m_isHovered = IsInLogoRegion(xPos, yPos);
        m_isControlHovered = IsInPlaybackControlRegion(xPos, yPos);

        if (m_isHovered ||
            (m_isLogoMenuHovered && IsInLogoMenuRegion(xPos, yPos, 1.0f))) {
          m_isLogoMenuHovered = true;
          m_logoMenuHoveredIndex = GetLogoMenuButtonAt(xPos, yPos, 1.0f);
        } else {
          m_isLogoMenuHovered = false;
          m_logoMenuHoveredIndex = -1;
        }
      }

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
    m_isPlaylistHovered = false;
    m_isLogoMenuHovered = false;
    m_logoMenuHoveredIndex = -1;
    m_playlistToolbarHoveredIndex = -1;
    m_isTrackingMouse = false;
    return 0;
  }
  case WM_LBUTTONDOWN: {
    int xPos = GET_X_LPARAM(lParam);
    int yPos = GET_Y_LPARAM(lParam);

    if (m_isPlaylistHovered) {
      int btnIndex = GetPlaylistToolbarButtonAt(xPos, yPos);
      if (btnIndex != -1) {
        if (m_onPlaylistToolbarClick) {
          m_onPlaylistToolbarClick(btnIndex);
        }
        return 0;
      }
      if (m_onPlaylistClick) {
        m_onPlaylistClick(xPos, yPos);
      }
      return 0;
    }

    if (m_isLogoMenuHovered) {
      UINT dpi = GetDpiForWindow(hwnd);
      int logicalX = MulDiv(xPos, 96, dpi);
      int logicalY = MulDiv(yPos, 96, dpi);

      LogoMenuLayout layout = LayoutCalculator::CalculateLogoMenuLayout(
          m_config, 1.0f, m_logoMenuItems.size());
      for (size_t i = 0; i < m_logoMenuItems.size(); ++i) {
        if (logicalX >= layout.items[i].hitRect.left &&
            logicalX <= layout.items[i].hitRect.right &&
            logicalY >= layout.items[i].hitRect.top &&
            logicalY <= layout.items[i].hitRect.bottom) {

          auto &item = m_logoMenuItems[i];
          if (item.isToggle) {
            item.toggleState = !item.toggleState;
          }
          if (item.commandId == ID_LOGO_EXIT) {
            PostQuitMessage(0);
          } else if (item.commandId == ID_LOGO_CLEAR) {
            if (m_onClearPlaylistCommand)
              m_onClearPlaylistCommand();
          } else if (item.commandId == ID_LOGO_PIN_PLAYLIST) {
            // m_config->SetPinPlaylist(item.toggleState);
          } else if (item.commandId == ID_LOGO_VISUALIZER) {
            if (m_config) {
              int mode = m_config->GetVisualizerMode();
              if (mode == 1)
                mode = 2;
              else if (mode == 2)
                mode = 0;
              else
                mode = 1;
              m_config->SetVisualizerMode(mode);
            }
          } else if (item.commandId == ID_LOGO_SHUFFLE) {
            if (m_onShuffleCommand) {
              m_onShuffleCommand();
            }
          } else if (item.commandId == ID_LOGO_PLAYLIST_POS) {
            if (m_config) {
              int newPos = m_config->GetPlaylistPosition() == 0 ? 1 : 0;
              m_config->SetPlaylistPosition(newPos);
            }
          }
          return 0;
        }
      }
    }

    int btnId = GetPlaybackButtonAt(xPos, yPos);
    if (btnId > 0) {
      if (m_onMediaCommand) {
        if (btnId == 1)
          m_onMediaCommand(APPCOMMAND_MEDIA_PREVIOUSTRACK);
        else if (btnId == 2)
          m_onMediaCommand(APPCOMMAND_MEDIA_PLAY_PAUSE);
        else if (btnId == 3)
          m_onMediaCommand(APPCOMMAND_MEDIA_NEXTTRACK);
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
    if (m_config) {
      POINT pt;
      pt.x = GET_X_LPARAM(lParam);
      pt.y = GET_Y_LPARAM(lParam);
      ScreenToClient(hwnd, &pt);

      if (m_isPlaylistHovered) {
        if (m_onPlaylistScroll) {
          int zDelta = GET_WHEEL_DELTA_WPARAM(wParam);
          m_onPlaylistScroll(zDelta);
        }
        return 0;
      }

      if (m_onVolumeScroll && IsInVolumeControlRegion(pt.x, pt.y)) {
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
        } else if (id == Window::ID_TRAY_PLAYLIST_MENU) {
          HMENU hPlaylistMenu = CreatePopupMenu();
          m_dynamicPlaylistPaths.clear();
          if (m_config) {
            m_dynamicPlaylistPaths = m_config->GetAvailablePlaylists();
            std::wstring defaultPath = m_config->GetDefaultPlaylistPath();
            std::filesystem::path defP(defaultPath);

            UINT index = 0;
            for (const auto &path : m_dynamicPlaylistPaths) {
              std::wstring displayName =
                  std::filesystem::path(path).stem().wstring();
              UINT flags = MF_STRING;
              try {
                if (std::filesystem::equivalent(std::filesystem::path(path),
                                                defP)) {
                  flags |= MF_CHECKED;
                }
              } catch (...) {
                if (path == defaultPath)
                  flags |= MF_CHECKED;
              }
              AppendMenuW(hPlaylistMenu, flags,
                          Window::ID_TRAY_PLAYLIST_START + index,
                          displayName.c_str());
              index++;
            }
          }
          AppendMenuW(hMenu, MF_POPUP, (UINT_PTR)hPlaylistMenu,
                      L"プレイリスト (Playlists)");
        } else {
          std::wstring text;
          switch (id) {
          case ID_TRAY_ZORDER_NORMAL:
            text = L"ウィンドウ順序: 通常";
            break;
          case ID_TRAY_ZORDER_TOPMOST:
            text = L"ウィンドウ順序: 最前面";
            break;
          case ID_TRAY_ZORDER_BOTTOM:
            text = L"ウィンドウ順序: 最背面";
            break;
          case ID_TRAY_ENABLE_RESIZE:
            text = L"リサイズモードを有効化";
            break;
          case ID_TRAY_SAVE_POS:
            text = L"終了時に位置とサイズを記憶";
            break;
          case ID_TRAY_RESET_POS:
            text = L"位置とサイズをリセット";
            break;
          case ID_TRAY_RESET_ALL:
            text = L"設定を初期化";
            break;
          case ID_TRAY_NEW_PLAYLIST:
            text = L"新規プレイリスト作成";
            break;
          case ID_TRAY_CLEAR_PLAYLIST:
            text = L"プレイリストをクリア";
            break;
          case ID_TRAY_BG_NOWPLAYING:
            text = L"背景: アルバムアート";
            break;
          case ID_TRAY_BG_HIDDEN:
            text = L"背景: 非表示";
            break;
          case ID_TRAY_BG_DEFAULT:
            text = L"背景: デフォルト背景";
            break;
          case ID_TRAY_VIS_NONE:
            text = L"ビジュアライザ: 非表示";
            break;
          case ID_TRAY_VIS_PRISM:
            text = L"ビジュアライザ: プリズム・ビート";
            break;
          case ID_TRAY_VIS_CIRCLE:
            text = L"ビジュアライザ: ヘイロー・ダスト";
            break;
          case ID_TRAY_EXIT:
            text = L"終了";
            break;
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
        CheckMenuRadioItem(hMenu, ID_TRAY_VIS_NONE, ID_TRAY_VIS_CIRCLE,
                           ID_TRAY_VIS_NONE + visMode, MF_BYCOMMAND);
        if (m_config->GetSavePositionOnExit()) {
          CheckMenuItem(hMenu, ID_TRAY_SAVE_POS, MF_BYCOMMAND | MF_CHECKED);
        }
        if (m_config->GetEnableResize()) {
          CheckMenuItem(hMenu, ID_TRAY_ENABLE_RESIZE,
                        MF_BYCOMMAND | MF_CHECKED);
        }
      }

      SetForegroundWindow(hwnd);
      TrackPopupMenu(hMenu, TPM_RIGHTBUTTON | TPM_BOTTOMALIGN, pt.x, pt.y, 0,
                     hwnd, nullptr);
      DestroyMenu(hMenu);
    }
    return 0;
  }
  case WM_COMMAND: {
    int wmId = LOWORD(wParam);
    if (wmId >= ID_TRAY_PLAYLIST_START &&
        wmId < ID_TRAY_PLAYLIST_START + m_dynamicPlaylistPaths.size()) {
      if (m_onPlaylistSwitchCommand) {
        m_onPlaylistSwitchCommand(
            m_dynamicPlaylistPaths[wmId - ID_TRAY_PLAYLIST_START]);
      }
      return 0;
    }
    switch (wmId) {
    case ID_TRAY_EXIT:
      PostMessage(hwnd, WM_CLOSE, 0, 0);
      break;
    case ID_TRAY_ZORDER_NORMAL:
    case ID_TRAY_ZORDER_TOPMOST:
    case ID_TRAY_ZORDER_BOTTOM: {
      int zOrder = wmId - ID_TRAY_ZORDER_NORMAL;
      if (m_config)
        m_config->SetZOrder(zOrder);
      HWND insertAfter = HWND_NOTOPMOST;
      if (zOrder == 1)
        insertAfter = HWND_TOPMOST;
      else if (zOrder == 2)
        insertAfter = HWND_BOTTOM;
      SetWindowPos(hwnd, insertAfter, 0, 0, 0, 0,
                   SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
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
    case ID_TRAY_VIS_NONE:
    case ID_TRAY_VIS_PRISM:
    case ID_TRAY_VIS_CIRCLE: {
      if (m_config) {
        m_config->SetVisualizerMode(wmId - ID_TRAY_VIS_NONE);
      }
      break;
    }
    case ID_TRAY_ENABLE_RESIZE: {
      if (m_config) {
        bool current = m_config->GetEnableResize();
        m_config->SetEnableResize(!current);
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
      SetWindowPos(hwnd, nullptr, x, y, width, height,
                   SWP_NOZORDER | SWP_NOACTIVATE);
      break;
    }
    case ID_TRAY_RESET_ALL: {
      if (m_config) {
        m_config->SaveDefaultSettings();
      }
      MessageBoxW(hwnd, L"設定を初期化しました。アプリを再起動します。",
                  L"通知", MB_OK | MB_ICONINFORMATION);
      PostMessage(hwnd, WM_CLOSE, 0, 0);
      break;
    }
    case ID_TRAY_NEW_PLAYLIST: {
      if (m_onNewPlaylistCommand) {
        m_onNewPlaylistCommand();
      }
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
    COPYDATASTRUCT *pcds = reinterpret_cast<COPYDATASTRUCT *>(lParam);
    if (pcds && pcds->lpData && m_onCopyDataCallback) {
      std::wstring path(reinterpret_cast<LPCWSTR>(pcds->lpData),
                        pcds->cbData / sizeof(wchar_t));
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
        m_config->SaveWindowPosition(logicalX, logicalY, logicalWidth,
                                     logicalHeight);
      }
    }
    PostQuitMessage(0);
    return 0;
  }
  }
  return DefWindowProcW(hwnd, uMsg, wParam, lParam);
}
