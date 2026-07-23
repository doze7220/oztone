#include "Window.h"
#include "ConfigManager.h"
#include <filesystem>

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

void Window::HandleTrayIcon(HWND hwnd, LPARAM lParam) {
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
          std::wstring defaultPath = m_config->GetPlaylist().DefaultPlaylistPath;
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
        AppendMenuW(hAdvMenu, MF_STRING, Window::ID_TRAY_SHOW_OSD,
                    L"OSD表示");

        if (m_config) {
          int zOrder = m_config->GetWindow().ZOrder;
          CheckMenuRadioItem(hAdvMenu, ID_TRAY_ZORDER_NORMAL,
                             ID_TRAY_ZORDER_BOTTOM,
                             ID_TRAY_ZORDER_NORMAL + zOrder, MF_BYCOMMAND);
          if (m_config->GetWindow().SavePositionOnExit) {
            CheckMenuItem(hAdvMenu, ID_TRAY_SAVE_POS,
                          MF_BYCOMMAND | MF_CHECKED);
          } else {
            CheckMenuItem(hAdvMenu, ID_TRAY_SAVE_POS,
                          MF_BYCOMMAND | MF_UNCHECKED);
          }
          if (m_config->GetGlobalHotkeys().ShowHotkeys) {
            CheckMenuItem(hAdvMenu, ID_TRAY_SHOW_HOTKEYS,
                          MF_BYCOMMAND | MF_CHECKED);
          }
          if (m_config->GetLayoutOSD().EnableOSD) {
            CheckMenuItem(hAdvMenu, ID_TRAY_SHOW_OSD,
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
}

void Window::HandleCommand(HWND hwnd, WPARAM wParam) {
  int wmId = LOWORD(wParam);
  if (wmId >= ID_TRAY_PLAYLIST_START &&
      wmId < ID_TRAY_PLAYLIST_START + m_dynamicPlaylistPaths.size()) {
    if (m_onPlaylistSwitchCommand) {
      m_onPlaylistSwitchCommand(
          m_dynamicPlaylistPaths[wmId - ID_TRAY_PLAYLIST_START]);
    }
    return;
  }
  switch (wmId) {
  case ID_TRAY_RESET_ALL:
    if (m_onResetAllCallback) {
      m_onResetAllCallback();
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
      m_config->SetShowHotkeys(!m_config->GetGlobalHotkeys().ShowHotkeys);
    }
    break;
  case ID_TRAY_SAVE_POS:
    if (m_config) {
      bool currentState = m_config->GetWindow().SavePositionOnExit;
      m_config->SetSavePositionOnExit(!currentState);
    }
    break;
  case ID_TRAY_SHOW_OSD:
    if (m_config) {
      m_config->SetEnableOSD(!m_config->GetLayoutOSD().EnableOSD);
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
}
