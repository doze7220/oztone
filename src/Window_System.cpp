#include "Window.h"
#include "ConfigManager.h"
#include <windowsx.h>

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

void Window::HandleHotkey(WPARAM wParam, LPARAM lParam) {
  if (m_onHotkey) {
    m_onHotkey(static_cast<int>(wParam));
  }
}

void Window::HandleAppMediaKey(WPARAM wParam) {
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
}

void Window::HandleCopyData(LPARAM lParam) {
  COPYDATASTRUCT *pcds = reinterpret_cast<COPYDATASTRUCT *>(lParam);
  if (pcds && pcds->lpData && m_onCopyDataCallback) {
    std::wstring path(reinterpret_cast<LPCWSTR>(pcds->lpData),
                      pcds->cbData / sizeof(wchar_t));
    if (!path.empty() && path.back() == L'\0') {
      path.pop_back();
    }
    m_onCopyDataCallback(path);
  }
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

void Window::HandleGetMinMaxInfo(HWND hwnd, LPARAM lParam) {
  MINMAXINFO *pMinMaxInfo = reinterpret_cast<MINMAXINFO *>(lParam);
  UINT dpi = GetDpiForWindow(hwnd);
  if (dpi == 0)
    dpi = 96;
  pMinMaxInfo->ptMinTrackSize.x = MulDiv(495, dpi, 96);
  pMinMaxInfo->ptMinTrackSize.y = MulDiv(260, dpi, 96);
}

void Window::HandleSize(WPARAM wParam, LPARAM lParam) {
  if (wParam != SIZE_MINIMIZED && m_onResize) {
    int width = LOWORD(lParam);
    int height = HIWORD(lParam);
    m_onResize(width, height);
  }
}

LRESULT Window::HandleNcHitTest(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
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

void Window::HandleWindowPosChanging(LPARAM lParam) {
  if (m_config && m_config->GetZOrder() == 2) {
    WINDOWPOS *pos = reinterpret_cast<WINDOWPOS *>(lParam);
    pos->hwndInsertAfter = HWND_BOTTOM;
  }
}

bool Window::HandleKeyDown(WPARAM wParam, LPARAM lParam) {
  if (GetAsyncKeyState(VK_SHIFT) & 0x8000) {
      if (wParam == VK_HOME) {
          if (m_onArtFramingReset) m_onArtFramingReset();
          return true;
      }
      if (wParam == VK_UP || wParam == VK_DOWN || wParam == VK_LEFT || wParam == VK_RIGHT) {
          float dx = 0.0f;
          float dy = 0.0f;
          if (wParam == VK_UP) dy = -10.0f;
          if (wParam == VK_DOWN) dy = 10.0f;
          if (wParam == VK_LEFT) dx = -10.0f;
          if (wParam == VK_RIGHT) dx = 10.0f;
          if (m_onArtFramingMove) m_onArtFramingMove(dx, dy);
          return true;
      }
      if (wParam == VK_PRIOR) { // PAGEUP
          if (m_onArtFramingScroll) {
              RECT rect;
              GetClientRect(m_hwnd, &rect);
              m_onArtFramingScroll(120.0f, rect.right / 2, rect.bottom / 2);
          }
          return true;
      }
      if (wParam == VK_NEXT) { // PAGEDOWN
          if (m_onArtFramingScroll) {
              RECT rect;
              GetClientRect(m_hwnd, &rect);
              m_onArtFramingScroll(-120.0f, rect.right / 2, rect.bottom / 2);
          }
          return true;
      }
  }
  return false;
}
