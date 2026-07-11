#include "Window.h"
#include "ConfigManager.h"
#include "resource.h"
#include <filesystem>
#include <shellapi.h>
#include <windowsx.h>

constexpr UINT TRAY_MENU_ORDER[] = {Window::ID_TRAY_PLAY_PAUSE,
                                    Window::ID_TRAY_PREV_TRACK,
                                    Window::ID_TRAY_NEXT_TRACK,
                                    0, // separator
                                    Window::ID_TRAY_VOL_MENU,
                                    0, // separator
                                    Window::ID_TRAY_PLAYLIST_MENU,
                                    0, // separator
                                    Window::ID_TRAY_ADVANCED_MENU,
                                    0, // separator
                                    Window::ID_TRAY_EXIT};

HWND Window::s_hwnd = nullptr;

Window::Window()
    : m_hwnd(nullptr), m_hInstance(nullptr), m_config(nullptr),
      m_isHovered(false), m_isControlHovered(false), m_isVolumeHovered(false),
      m_isPlaylistHovered(false), m_isTrackingMouse(false),
      m_pDropTarget(nullptr), m_keyboardHook(nullptr),
      m_isLogoMenuHovered(false), m_isPlaylistExpanded(false),
      m_isLogoMenuExpanded(false) {
  m_logoMenuItems = {
      {ID_LOGO_EXIT, L"❌", false, false, L"OZtoneの終了"},
      {ID_LOGO_VISUALIZER, L"📽️", false, false, L"ビジュアライザ表示切り替え"},
      {ID_LOGO_BG_MODE, L"🖼️", false, false, L"背景表示切り替え"},
      {ID_LOGO_SHUFFLE, L"🔀", false, false, L"シャッフル再生ON/OFF"},
      {ID_LOGO_PLAYLIST_POS, L"↔️", false, false,
       L"プレイリストの配置場所切り替え"},
      {ID_LOGO_RESIZE_MODE, L"◢", false, false, L"リサイズモード"},
      {ID_LOGO_LOCK_POS, L"⚓", false, false, L"画面位置の固定"}};
}

Window::~Window() {
  if (m_keyboardHook) {
    UnhookWindowsHookEx(m_keyboardHook);
    m_keyboardHook = nullptr;
  }
  UnregisterHotkeys();
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
  m_pWindow->SetHovered(true);
  *pdwEffect = DROPEFFECT_COPY;
  return S_OK;
}

HRESULT STDMETHODCALLTYPE DropTarget::DragOver(DWORD grfKeyState, POINTL pt,
                                               DWORD *pdwEffect) {
  m_pWindow->SetHovered(true);
  *pdwEffect = DROPEFFECT_COPY;
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
  return S_OK;
}

bool Window::Initialize(HINSTANCE hInstance, int nCmdShow,
                        ConfigManager &config) {
  m_hInstance = hInstance;
  m_config = &config;

  WNDCLASSEXW wc = {};
  wc.cbSize = sizeof(WNDCLASSEXW);
  wc.style = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
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

  int finalWidth = rect.right - rect.left;
  int finalHeight = rect.bottom - rect.top;

  if (x == CW_USEDEFAULT || y == CW_USEDEFAULT) {
    scaledX = (GetSystemMetrics(SM_CXSCREEN) - finalWidth) / 2;
    scaledY = (GetSystemMetrics(SM_CYSCREEN) - finalHeight) / 2;
  }

  m_hwnd =
      CreateWindowExW(dwExStyle, m_className, L"OZtone", dwStyle, scaledX,
                      scaledY, finalWidth, finalHeight,
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

  RegisterHotkeys();

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

void Window::RegisterHotkeys() {
  UnregisterHotkeys();

  if (!m_config)
    return;

  auto reg = [&](int id, int mod, int vk) {
    if (vk != 0) {
      RegisterHotKey(m_hwnd, id, mod | MOD_NOREPEAT, vk);
    }
  };

  reg(HK_NEXT_TRACK, m_config->GetModifierNextTrack(),
      m_config->GetVKNextTrack());
  reg(HK_PREV_TRACK, m_config->GetModifierPrevTrack(),
      m_config->GetVKPrevTrack());
  reg(HK_PLAY_PAUSE, m_config->GetModifierPlayPause(),
      m_config->GetVKPlayPause());
  reg(HK_STOP, m_config->GetModifierStop(), m_config->GetVKStop());
  reg(HK_VOL_UP_5, m_config->GetModifierVolUp5(), m_config->GetVKVolUp5());
  reg(HK_VOL_DOWN_5, m_config->GetModifierVolDown5(),
      m_config->GetVKVolDown5());
  reg(HK_VOL_UP_25, m_config->GetModifierVolUp25(), m_config->GetVKVolUp25());
  reg(HK_VOL_DOWN_25, m_config->GetModifierVolDown25(),
      m_config->GetVKVolDown25());
  reg(HK_PREV_PLAYLIST, m_config->GetModifierPrevPlaylist(),
      m_config->GetVKPrevPlaylist());
  reg(HK_NEXT_PLAYLIST, m_config->GetModifierNextPlaylist(),
      m_config->GetVKNextPlaylist());
  reg(HK_ACTIVE_TOPMOST, m_config->GetModifierActiveTopMost(),
      m_config->GetVKActiveTopMost());
  reg(HK_ACTIVE_BOTTOM, m_config->GetModifierActiveBottom(),
      m_config->GetVKActiveBottom());
  reg(HK_EXIT_APP, m_config->GetModifierExitApp(), m_config->GetVKExitApp());
}

void Window::UnregisterHotkeys() {
  for (int i = HK_NEXT_TRACK; i <= HK_EXIT_APP; ++i) {
    UnregisterHotKey(m_hwnd, i);
  }
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

  RECT clientRect;
  GetClientRect(m_hwnd, &clientRect);
  int logicalWidth = MulDiv(clientRect.right - clientRect.left, 96, dpi);

  if (m_config->GetIsPlaylistPinned() && m_config->GetPlaylistPosition() == 0) {
    if (logicalWidth > 495 + m_config->GetPlaylistWidth()) {
      logicalX -= m_config->GetPlaylistWidth();
    }
  }

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

  RECT rect;
  GetClientRect(m_hwnd, &rect);
  int logicalWidth = MulDiv(rect.right - rect.left, 96, dpi);

  LogoMenuLayout layout = LayoutCalculator::CalculateLogoMenuLayout(
      static_cast<float>(logicalWidth), m_config, progress,
      m_logoMenuItems.size());

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

  RECT rect;
  GetClientRect(m_hwnd, &rect);
  int logicalWidth = MulDiv(rect.right - rect.left, 96, dpi);

  LogoMenuLayout layout = LayoutCalculator::CalculateLogoMenuLayout(
      static_cast<float>(logicalWidth), m_config, progress,
      m_logoMenuItems.size());
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

  if (m_config->GetIsPlaylistPinned()) {
    int playlistWidth = m_config->GetPlaylistWidth();
    if (m_config->GetPlaylistPosition() == 0) {
      logicalX -= playlistWidth;
      logicalWidth -= playlistWidth;
      if (logicalX < 0)
        return false;
    } else {
      logicalWidth -= playlistWidth;
      if (logicalX >= logicalWidth)
        return false;
    }
  }

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
  int logicalWidth = MulDiv(rect.right - rect.left, 96, dpi);
  int logicalHeight = MulDiv(rect.bottom - rect.top, 96, dpi);

  if (m_config->GetIsPlaylistPinned()) {
    int playlistWidth = m_config->GetPlaylistWidth();
    if (m_config->GetPlaylistPosition() == 0) {
      logicalX -= playlistWidth;
      logicalWidth -= playlistWidth;
      if (logicalX < 0)
        return false;
    } else {
      logicalWidth -= playlistWidth;
      if (logicalX >= logicalWidth)
        return false;
    }
  }

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
  if (!(m_isPlaylistHovered || m_isPlaylistExpanded) && IsInLogoRegion(x, y))
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
  if (!(m_isPlaylistHovered || m_isPlaylistExpanded) &&
      logicalY <= m_config->GetLogoY() + m_config->GetLogoHeight() + 10.0f) {
    return false;
  }

  float hoverWidth =
      (m_isPlaylistHovered || m_isPlaylistExpanded)
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
  bool isYMatch = (m_isPlaylistHovered || m_isPlaylistExpanded) ||
                  (logicalY < logicalHeight - controlHeight);

  return isXMatch && isYMatch;
}

int Window::GetPlaylistToolbarButtonAt(int x, int y) const {
  if (!m_config || !m_hwnd || !m_isPlaylistHovered)
    return -1;
  UINT dpi = GetDpiForWindow(m_hwnd);
  int logicalX = MulDiv(x, 96, dpi);
  int logicalY = MulDiv(y, 96, dpi);

  RECT rect;
  GetClientRect(m_hwnd, &rect);
  int logicalWidth = MulDiv(rect.right - rect.left, 96, dpi);
  int logicalHeight = MulDiv(rect.bottom - rect.top, 96, dpi);

  PlaylistLayout layout = LayoutCalculator::CalculatePlaylistLayout(
      static_cast<float>(logicalWidth), static_cast<float>(logicalHeight),
      m_config, 0.0f, 0.0f, 0, 0);
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

bool Window::IsPlaylistPinnedButtonAt(int x, int y) const {
  if (!m_config || !m_hwnd || !m_isPlaylistHovered)
    return false;
  UINT dpi = GetDpiForWindow(m_hwnd);
  int logicalX = MulDiv(x, 96, dpi);
  int logicalY = MulDiv(y, 96, dpi);

  RECT rect;
  GetClientRect(m_hwnd, &rect);
  int logicalWidth = MulDiv(rect.right - rect.left, 96, dpi);
  int logicalHeight = MulDiv(rect.bottom - rect.top, 96, dpi);

  PlaylistLayout layout = LayoutCalculator::CalculatePlaylistLayout(
      static_cast<float>(logicalWidth), static_cast<float>(logicalHeight),
      m_config, 0.0f, 0.0f, 0, 0);

  return (logicalX >= layout.toolbarLayout.pinButtonHitRect.left &&
          logicalX <= layout.toolbarLayout.pinButtonHitRect.right &&
          logicalY >= layout.toolbarLayout.pinButtonHitRect.top &&
          logicalY <= layout.toolbarLayout.pinButtonHitRect.bottom);
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

  if (m_config->GetIsPlaylistPinned()) {
    int playlistWidth = m_config->GetPlaylistWidth();
    if (m_config->GetPlaylistPosition() == 0) {
      logicalX -= playlistWidth;
      logicalWidth -= playlistWidth;
      if (logicalX < 0)
        return 0;
    } else {
      logicalWidth -= playlistWidth;
      if (logicalX >= logicalWidth)
        return 0;
    }
  }

  float centerX = (logicalWidth / 2.0f) + m_config->GetPlaybackCenterOffsetX();
  float centerY = logicalHeight -
                  static_cast<float>(m_config->GetPlaybackBaseBottomOffset());
  float size = static_cast<float>(m_config->GetPlaybackButtonSize());
  float spacing = static_cast<float>(m_config->GetPlaybackButtonSpacing());
  float halfSize = size / 2.0f;

  if (logicalY >= centerY - halfSize && logicalY <= centerY + halfSize) {
    float positions[5] = {centerX - spacing * 2.0f, centerX - spacing, centerX,
                          centerX + spacing, centerX + spacing * 2.0f};
    for (int i = 0; i < 5; ++i) {
      if (logicalX >= positions[i] - halfSize &&
          logicalX <= positions[i] + halfSize) {
        return i + 1; // 1 to 5
      }
    }
  }
  return 0;
}

LRESULT Window::WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
  switch (uMsg) {
  case WM_HOTKEY: {
    if (m_onHotkey) {
      m_onHotkey(static_cast<int>(wParam));
    }
    return 0;
  }
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
      if (m_isArtFramingDragging) {
        int xPos = GET_X_LPARAM(lParam);
        int yPos = GET_Y_LPARAM(lParam);
        float deltaX = static_cast<float>(xPos - m_artFramingDragStartPt.x);
        float deltaY = static_cast<float>(yPos - m_artFramingDragStartPt.y);
        
        if (m_onArtFramingMove && (deltaX != 0.0f || deltaY != 0.0f)) {
            UINT dpi = GetDpiForWindow(hwnd);
            float logDX = deltaX * 96.0f / dpi;
            float logDY = deltaY * 96.0f / dpi;
            m_onArtFramingMove(logDX, logDY);
        }
        m_artFramingDragStartPt.x = xPos;
        m_artFramingDragStartPt.y = yPos;
        return 0;
      }

      int xPos = GET_X_LPARAM(lParam);
      int yPos = GET_Y_LPARAM(lParam);

      bool isPinned = m_config->GetIsPlaylistPinned();
      bool isInPlaylistRegion = IsInPlaylistRegion(xPos, yPos);
      m_isPlaylistHovered = isPinned || isInPlaylistRegion;

      if (m_isPlaylistHovered) {
        m_playlistToolbarHoveredIndex = GetPlaylistToolbarButtonAt(xPos, yPos);
        m_isPlaylistPinnedHovered = IsPlaylistPinnedButtonAt(xPos, yPos);
      } else {
        m_playlistToolbarHoveredIndex = -1;
        m_isPlaylistPinnedHovered = false;
      }

      if (m_isPlaylistHovered && !isPinned) {
        m_isHovered = false;
        m_isControlHovered = false;
        m_isVolumeHovered = false;
        m_isLogoMenuHovered = false;
        m_playbackHoveredIndex = -1;
      } else {
        m_isHovered = IsInLogoRegion(xPos, yPos);
        m_isControlHovered = IsInPlaybackControlRegion(xPos, yPos);
        m_isVolumeHovered = IsInVolumeControlRegion(xPos, yPos);

        if (m_isControlHovered) {
          m_playbackHoveredIndex = GetPlaybackButtonAt(xPos, yPos);
        } else {
          m_playbackHoveredIndex = -1;
        }

        if (m_isHovered || ((m_isLogoMenuHovered || m_isLogoMenuExpanded) &&
                            IsInLogoMenuRegion(xPos, yPos, 1.0f))) {
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
    m_isVolumeHovered = false;
    m_isPlaylistHovered = false;
    m_isLogoMenuHovered = false;
    m_logoMenuHoveredIndex = -1;
    m_playlistToolbarHoveredIndex = -1;
    m_isPlaylistPinnedHovered = false;
    m_playbackHoveredIndex = -1;
    m_playlistHoveredItemIndex = -1;
    m_isTrackingMouse = false;
    return 0;
  }
  case WM_LBUTTONDOWN: {
    if (m_isArtFramingDragging) {
      m_isArtFramingDragging = false;
      ReleaseCapture();
    }
    int xPos = GET_X_LPARAM(lParam);
    int yPos = GET_Y_LPARAM(lParam);

    if (!m_isPlaylistHovered && !m_isLogoMenuHovered && !m_isControlHovered && !m_isVolumeHovered && !IsInLogoRegion(xPos, yPos)) {
        if (GetAsyncKeyState(VK_SHIFT) & 0x8000) {
            m_isArtFramingDragging = true;
            m_artFramingDragStartPt.x = xPos;
            m_artFramingDragStartPt.y = yPos;
            SetCapture(hwnd);
            return 0;
        }
    }

    if (m_isPlaylistHovered) {
      if (IsPlaylistPinnedButtonAt(xPos, yPos)) {
        m_config->SetIsPlaylistPinned(!m_config->GetIsPlaylistPinned());
        return 0;
      }

      int btnIndex = GetPlaylistToolbarButtonAt(xPos, yPos);
      if (btnIndex != -1) {
        if (m_onPlaylistToolbarClick) {
          m_onPlaylistToolbarClick(btnIndex);
        }
        return 0;
      }

      if (IsInPlaylistRegion(xPos, yPos)) {
        if (m_onPlaylistClick) {
          m_onPlaylistClick(xPos, yPos);
        }
        return 0;
      }
    }

    if (m_isLogoMenuHovered) {
      UINT dpi = GetDpiForWindow(hwnd);
      int logicalX = MulDiv(xPos, 96, dpi);
      int logicalY = MulDiv(yPos, 96, dpi);

      RECT rect;
      GetClientRect(hwnd, &rect);
      int logicalWidth = MulDiv(rect.right - rect.left, 96, dpi);

      LogoMenuLayout layout = LayoutCalculator::CalculateLogoMenuLayout(
          static_cast<float>(logicalWidth), m_config, 1.0f,
          m_logoMenuItems.size());
      for (size_t i = 0; i < m_logoMenuItems.size(); ++i) {
        if (logicalX >= layout.items[i].hitRect.left &&
            logicalX <= layout.items[i].hitRect.right &&
            logicalY >= layout.items[i].hitRect.top &&
            logicalY <= layout.items[i].hitRect.bottom) {

          m_logoMenuClickedIndex = static_cast<int>(i);
          auto &item = m_logoMenuItems[i];
          if (item.isToggle) {
            item.toggleState = !item.toggleState;
          }
          if (item.commandId == ID_LOGO_EXIT) {
            PostQuitMessage(0);
          } else if (item.commandId == ID_LOGO_BG_MODE) {
            if (m_config) {
              int mode = m_config->GetBackgroundArtMode();
              if (mode == 0)
                mode = 1;
              else if (mode == 1)
                mode = 2;
              else
                mode = 0;
              m_config->SetBackgroundArtMode(mode);
            }
          } else if (item.commandId == ID_LOGO_RESIZE_MODE) {
            if (m_config) {
              m_config->SetEnableResize(!m_config->GetEnableResize());
            }
          } else if (item.commandId == ID_LOGO_LOCK_POS) {
            if (m_config) {
              m_config->SetLockWindowPosition(
                  !m_config->GetLockWindowPosition());
            }
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
      m_playbackClickedIndex = btnId;
      if (btnId == 2 || btnId == 4) {
        if (m_onSkipCommand) {
          float delta = (btnId == 2) ? -m_config->GetSkipSeconds()
                                     : m_config->GetSkipSeconds();
          m_onSkipCommand(delta);
        }
      } else {
        if (m_onMediaCommand) {
          if (btnId == 1)
            m_onMediaCommand(APPCOMMAND_MEDIA_PREVIOUSTRACK);
          else if (btnId == 3)
            m_onMediaCommand(APPCOMMAND_MEDIA_PLAY_PAUSE);
          else if (btnId == 5)
            m_onMediaCommand(APPCOMMAND_MEDIA_NEXTTRACK);
        }
      }
      return 0;
    }

    if (IsInLogoRegion(xPos, yPos) && !m_isLogoMenuHovered) {
      m_isLogoClicked = true;
    }

    if (!m_config || !m_config->GetLockWindowPosition()) {
      ReleaseCapture();
      SendMessage(hwnd, WM_NCLBUTTONDOWN, HTCAPTION, 0);
    }
    return 0;
  }
  case WM_LBUTTONUP: {
    if (m_isArtFramingDragging) {
      m_isArtFramingDragging = false;
      ReleaseCapture();
      return 0;
    }
    break;
  }
  case WM_LBUTTONDBLCLK: {
    int xPos = GET_X_LPARAM(lParam);
    int yPos = GET_Y_LPARAM(lParam);

    if (m_isPlaylistHovered) {
      if (IsInPlaylistRegion(xPos, yPos)) {
        if (m_onPlaylistDoubleClick) {
          m_onPlaylistDoubleClick(xPos, yPos);
        }
        return 0;
      }
    }
    return 0;
  }
  case WM_MOUSEWHEEL: {
    if (m_config) {
      POINT pt;
      pt.x = GET_X_LPARAM(lParam);
      pt.y = GET_Y_LPARAM(lParam);
      ScreenToClient(hwnd, &pt);

      if (m_isPlaylistHovered && IsInPlaylistRegion(pt.x, pt.y)) {
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
      
      if (!m_isPlaylistHovered && !m_isVolumeHovered) {
        if (GetAsyncKeyState(VK_SHIFT) & 0x8000) {
          if (m_onArtFramingScroll) {
            int zDelta = GET_WHEEL_DELTA_WPARAM(wParam);
            m_onArtFramingScroll(static_cast<float>(zDelta));
          }
          return 0;
        }
      }
    }
    break;
  }
  case WM_KEYDOWN: {
    if (GetAsyncKeyState(VK_SHIFT) & 0x8000) {
        if (wParam == VK_HOME) {
            if (m_onArtFramingReset) m_onArtFramingReset();
            return 0;
        }
        if (wParam == VK_UP || wParam == VK_DOWN || wParam == VK_LEFT || wParam == VK_RIGHT) {
            float dx = 0.0f;
            float dy = 0.0f;
            if (wParam == VK_UP) dy = -10.0f;
            if (wParam == VK_DOWN) dy = 10.0f;
            if (wParam == VK_LEFT) dx = -10.0f;
            if (wParam == VK_RIGHT) dx = 10.0f;
            if (m_onArtFramingMove) m_onArtFramingMove(dx, dy);
            return 0;
        }
        if (wParam == VK_PRIOR) { // PAGEUP
            if (m_onArtFramingScroll) m_onArtFramingScroll(120.0f);
            return 0;
        }
        if (wParam == VK_NEXT) { // PAGEDOWN
            if (m_onArtFramingScroll) m_onArtFramingScroll(-120.0f);
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
                      L"プレイリスト");
        } else if (id == Window::ID_TRAY_ADVANCED_MENU) {
          HMENU hAdvMenu = CreatePopupMenu();
          AppendMenuW(hAdvMenu, MF_STRING, Window::ID_TRAY_ZORDER_NORMAL,
                      L"ウィンドウ順序: 通常");
          AppendMenuW(hAdvMenu, MF_STRING, Window::ID_TRAY_ZORDER_TOPMOST,
                      L"ウィンドウ順序: 最前面");
          AppendMenuW(hAdvMenu, MF_STRING, Window::ID_TRAY_ZORDER_BOTTOM,
                      L"ウィンドウ順序: 最背面");
          AppendMenuW(hAdvMenu, MF_SEPARATOR, 0, nullptr);
          AppendMenuW(hAdvMenu, MF_STRING, Window::ID_TRAY_SAVE_POS,
                      L"終了時に位置とサイズを記憶");
          AppendMenuW(hAdvMenu, MF_STRING, Window::ID_TRAY_RESET_POS,
                      L"位置とサイズをリセット");
          AppendMenuW(hAdvMenu, MF_STRING, Window::ID_TRAY_RESET_ALL,
                      L"設定を初期化");
          AppendMenuW(hAdvMenu, MF_SEPARATOR, 0, nullptr);
          AppendMenuW(hAdvMenu, MF_STRING, Window::ID_TRAY_SHOW_HOTKEYS,
                      L"ホットキー表示");

          if (m_config) {
            int zOrder = m_config->GetZOrder();
            CheckMenuRadioItem(hAdvMenu, ID_TRAY_ZORDER_NORMAL,
                               ID_TRAY_ZORDER_BOTTOM,
                               ID_TRAY_ZORDER_NORMAL + zOrder, MF_BYCOMMAND);
            if (m_config->GetSavePositionOnExit()) {
              CheckMenuItem(hAdvMenu, ID_TRAY_SAVE_POS,
                            MF_BYCOMMAND | MF_CHECKED);
            }
            if (m_config->GetShowHotkeys()) {
              CheckMenuItem(hAdvMenu, ID_TRAY_SHOW_HOTKEYS,
                            MF_BYCOMMAND | MF_CHECKED);
            }
          }
          AppendMenuW(hMenu, MF_POPUP, (UINT_PTR)hAdvMenu, L"詳細設定");
        } else if (id == Window::ID_TRAY_VOL_MENU) {
          HMENU hVolMenu = CreatePopupMenu();
          AppendMenuW(hVolMenu, MF_STRING, Window::ID_TRAY_VOL_100,L"音量 100%");
          AppendMenuW(hVolMenu, MF_STRING, Window::ID_TRAY_VOL_75, L"音量 75%");
          AppendMenuW(hVolMenu, MF_STRING, Window::ID_TRAY_VOL_50, L"音量 50%");
          AppendMenuW(hVolMenu, MF_STRING, Window::ID_TRAY_VOL_25, L"音量 25%");
          AppendMenuW(hMenu, MF_POPUP, (UINT_PTR)hVolMenu, L"音量");
        } else {
          std::wstring text;
          switch (id) {
          case ID_TRAY_PLAY_PAUSE:
            text = L"再生 / 一時停止";
            break;
          case ID_TRAY_PREV_TRACK:
            text = L"前の曲へ";
            break;
          case ID_TRAY_NEXT_TRACK:
            text = L"次の曲へ";
            break;
          case ID_TRAY_EXIT:
            text = L"終了";
            break;
          }
          AppendMenuW(hMenu, MF_STRING, id, text.c_str());
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
    case ID_TRAY_RESET_ALL:
      if (m_config) {
        m_config->SaveDefaultSettings();
      }
      {
        wchar_t exePath[MAX_PATH];
        GetModuleFileNameW(nullptr, exePath, MAX_PATH);
        ShellExecuteW(nullptr, L"open", exePath, nullptr, nullptr, SW_SHOWDEFAULT);
        PostMessage(hwnd, WM_CLOSE, 0, 0);
      }
      break;
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
    case ID_TRAY_SHOW_HOTKEYS:
      if (m_config) {
        m_config->SetShowHotkeys(!m_config->GetShowHotkeys());
      }
      break;
    case ID_TRAY_PLAY_PAUSE:
      if (m_onMediaCommand)
        m_onMediaCommand(APPCOMMAND_MEDIA_PLAY_PAUSE);
      break;
    case ID_TRAY_PREV_TRACK:
      if (m_onMediaCommand)
        m_onMediaCommand(APPCOMMAND_MEDIA_PREVIOUSTRACK);
      break;
    case ID_TRAY_NEXT_TRACK:
      if (m_onMediaCommand)
        m_onMediaCommand(APPCOMMAND_MEDIA_NEXTTRACK);
      break;
    case ID_TRAY_VOL_100:
      if (m_onVolumeSetCommand)
        m_onVolumeSetCommand(1.0f);
      break;
    case ID_TRAY_VOL_75:
      if (m_onVolumeSetCommand)
        m_onVolumeSetCommand(0.75f);
      break;
    case ID_TRAY_VOL_50:
      if (m_onVolumeSetCommand)
        m_onVolumeSetCommand(0.50f);
      break;
    case ID_TRAY_VOL_25:
      if (m_onVolumeSetCommand)
        m_onVolumeSetCommand(0.25f);
      break;
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
