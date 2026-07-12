#include "Window.h"
#include "ConfigManager.h"
#include "resource.h"
#include <shellapi.h>

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

void Window::HandleDestroy(HWND hwnd) {
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
}
