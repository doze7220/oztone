#include "Window.h"
#include "Config/ConfigManager.h"
#include "LayoutCalculator.h"
#include "resource.h"
#include <windowsx.h>
#include <algorithm>

bool Window::IsInLogoRegion(int x, int y) const {
  if (!m_config || !m_hwnd)
    return false;

  UINT dpi = GetDpiForWindow(m_hwnd);
  int logicalX = MulDiv(x, 96, dpi);
  int logicalY = MulDiv(y, 96, dpi);

  RECT clientRect;
  GetClientRect(m_hwnd, &clientRect);
  int logicalWidth = MulDiv(clientRect.right - clientRect.left, 96, dpi);

  if (m_config->GetLayoutPlaylist().IsPlaylistPinned && m_config->GetLayoutPlaylist().PlaylistPosition == 0) {
    if (logicalWidth > 495 + m_config->GetLayoutPlaylist().PlaylistWidth) {
      logicalX -= m_config->GetLayoutPlaylist().PlaylistWidth;
    }
  }

  return (logicalX >= m_config->GetLayoutAppLogo().X &&
          logicalX <= m_config->GetLayoutAppLogo().X + m_config->GetLayoutAppLogo().Width &&
          logicalY >= m_config->GetLayoutAppLogo().Y &&
          logicalY <= m_config->GetLayoutAppLogo().Y + m_config->GetLayoutAppLogo().Height);
}

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

  if (m_config->GetLayoutPlaylist().IsPlaylistPinned) {
    int playlistWidth = m_config->GetLayoutPlaylist().PlaylistWidth;
    if (m_config->GetLayoutPlaylist().PlaylistPosition == 0) {
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

  return (logicalY >= logicalHeight - m_config->GetLayoutPlaybackControls().ControlHoverHeight);
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

  if (m_config->GetLayoutPlaylist().IsPlaylistPinned) {
    int playlistWidth = m_config->GetLayoutPlaylist().PlaylistWidth;
    if (m_config->GetLayoutPlaylist().PlaylistPosition == 0) {
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

  float volX = static_cast<float>(m_config->GetLayoutVolumeControl().BaseLeftOffset);
  float volY =
      logicalHeight - static_cast<float>(m_config->GetLayoutVolumeControl().BaseBottomOffset);
  float size = static_cast<float>(m_config->GetLayoutVolumeControl().IconSize);
  float width = size + 80.0f;
  float height = size + 20.0f;

  return (logicalX >= volX && logicalX <= volX + width &&
          logicalY >= volY - height / 2 && logicalY <= volY + height / 2);
}

bool Window::IsInTrackInfoRegion(int x, int y) const {
  if (!m_config || !m_hwnd)
    return false;

  UINT dpi = GetDpiForWindow(m_hwnd);
  int logicalX = MulDiv(x, 96, dpi);
  int logicalY = MulDiv(y, 96, dpi);

  RECT rect;
  GetClientRect(m_hwnd, &rect);
  int logicalWidth = MulDiv(rect.right - rect.left, 96, dpi);
  int logicalHeight = MulDiv(rect.bottom - rect.top, 96, dpi);

  int rightLimit = logicalWidth;
  if (m_config->GetLayoutPlaylist().PlaylistPosition == 1) {
    float hoverWidth = (m_isPlaylistHovered || m_isPlaylistExpanded)
                           ? static_cast<float>(m_config->GetLayoutPlaylist().PlaylistWidth)
                           : static_cast<float>(m_config->GetLayoutPlaylist().PlaylistHoverWidth);
    rightLimit -= static_cast<int>(hoverWidth);
  } else if (m_config->GetLayoutPlaylist().PlaylistPosition == 0) {
    if (m_config->GetLayoutPlaylist().IsPlaylistPinned) {
      int playlistWidth = m_config->GetLayoutPlaylist().PlaylistWidth;
      logicalX -= playlistWidth;
      rightLimit -= playlistWidth;
      if (logicalX < 0) return false;
    }
  }

  float baseY = static_cast<float>(logicalHeight - m_config->GetLayoutNowPlaying().BaseBottomOffset);

  float artTop = baseY + static_cast<float>(m_config->GetLayoutNowPlaying().ArtOffsetY);
  float artBottom = artTop + static_cast<float>(m_config->GetLayoutNowPlaying().ArtSize);

  float titleTop = baseY + static_cast<float>(m_config->GetLayoutNowPlaying().TitleOffsetY);
  float titleBottom = titleTop + m_config->GetLayoutNowPlaying().TitleFontSize;

  float artistTop = baseY + static_cast<float>(m_config->GetLayoutNowPlaying().ArtistOffsetY);
  float artistBottom = artistTop + m_config->GetLayoutNowPlaying().ArtistFontSize;

  float topLimit = artTop;
  if (titleTop < topLimit) topLimit = titleTop;
  if (artistTop < topLimit) topLimit = artistTop;

  float bottomLimit = artBottom;
  if (titleBottom > bottomLimit) bottomLimit = titleBottom;
  if (artistBottom > bottomLimit) bottomLimit = artistBottom;

  return (logicalX >= 0 && logicalX < rightLimit) &&
         (logicalY >= topLimit && logicalY <= bottomLimit);
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
      logicalY <= m_config->GetLayoutAppLogo().Y + m_config->GetLayoutAppLogo().Height + 10.0f) {
    return false;
  }

  float hoverWidth =
      (m_isPlaylistHovered || m_isPlaylistExpanded)
          ? static_cast<float>(m_config->GetLayoutPlaylist().PlaylistWidth)
          : static_cast<float>(m_config->GetLayoutPlaylist().PlaylistHoverWidth);
  float controlHeight = m_config->GetLayoutPlaybackControls().ControlHoverHeight;

  bool isXMatch = false;
  if (m_config->GetLayoutPlaylist().PlaylistPosition == 0) {
    isXMatch = logicalX <= hoverWidth;
  } else {
    isXMatch = logicalX >= logicalWidth - hoverWidth;
  }

  // プレイリストが展開されている場合は、画面下部であってもリスト上にマウスがあればホバーを維持する。
  // 展開されていない場合、またはピン留め時は、右下のリサイズやコントロールとの干渉を避けるためY座標を制限する。
  bool isYMatch = ((m_isPlaylistHovered || m_isPlaylistExpanded) && !m_config->GetLayoutPlaylist().IsPlaylistPinned) ||
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

  if (m_config->GetLayoutPlaylist().IsPlaylistPinned) {
    int playlistWidth = m_config->GetLayoutPlaylist().PlaylistWidth;
    if (m_config->GetLayoutPlaylist().PlaylistPosition == 0) {
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

  float centerX = (logicalWidth / 2.0f) + m_config->GetLayoutPlaybackControls().CenterOffsetX;
  float centerY = logicalHeight -
                  static_cast<float>(m_config->GetLayoutPlaybackControls().BaseBottomOffset);
  float size = static_cast<float>(m_config->GetLayoutPlaybackControls().ButtonSize);
  float spacing = static_cast<float>(m_config->GetLayoutPlaybackControls().ButtonSpacing);
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

bool Window::HandleMouseActivate(LRESULT& outResult) {
  if (m_config && m_config->GetWindow().ZOrder == 2) {
    outResult = MA_NOACTIVATE;
    return true;
  }
  return false;
}

void Window::HandleMouseMove(HWND hwnd, WPARAM wParam, LPARAM lParam) {
  if (m_config) {
    if (m_isArtFramingDragging && (wParam & MK_RBUTTON)) {
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
      return;
    } else if (m_isArtFramingDragging) {
      m_isArtFramingDragging = false;
      ReleaseCapture();
    }

    int xPos = GET_X_LPARAM(lParam);
    int yPos = GET_Y_LPARAM(lParam);

    bool isPinned = m_config->GetLayoutPlaylist().IsPlaylistPinned;
    bool isInPlaylistRegion = IsInPlaylistRegion(xPos, yPos);
    m_isPlaylistHovered = isInPlaylistRegion;

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
      m_isTrackInfoHovered = false;
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

      if (!m_isPlaylistHovered && !m_isControlHovered && !m_isVolumeHovered && !m_isLogoMenuHovered && !m_isHovered) {
        m_isTrackInfoHovered = IsInTrackInfoRegion(xPos, yPos);
      } else {
        m_isTrackInfoHovered = false;
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
}

void Window::HandleMouseLeave() {
  m_isHovered = false;
  m_isControlHovered = false;
  m_isVolumeHovered = false;
  m_isPlaylistHovered = false;
  m_isTrackInfoHovered = false;
  m_isLogoMenuHovered = false;
  m_logoMenuHoveredIndex = -1;
  m_playlistToolbarHoveredIndex = -1;
  m_isPlaylistPinnedHovered = false;
  m_playbackHoveredIndex = -1;
  m_playlistHoveredItemIndex = -1;
  m_isTrackingMouse = false;
}

void Window::HandleLButtonDown(HWND hwnd, WPARAM wParam, LPARAM lParam) {
  if (m_isArtFramingDragging) {
    m_isArtFramingDragging = false;
    ReleaseCapture();
  }
  int xPos = GET_X_LPARAM(lParam);
  int yPos = GET_Y_LPARAM(lParam);

  if (!m_isPlaylistHovered && !m_isLogoMenuHovered && !m_isControlHovered && !m_isVolumeHovered && !IsInLogoRegion(xPos, yPos)) {
    if (m_onBackgroundClickCallback) m_onBackgroundClickCallback();
  }

  if (m_isPlaylistHovered) {
    if (IsPlaylistPinnedButtonAt(xPos, yPos)) {
      bool pinned = !m_config->GetLayoutPlaylist().IsPlaylistPinned;
      m_config->SetIsPlaylistPinned(pinned);
      if (m_onPlaylistPinnedToggle) {
          m_onPlaylistPinnedToggle(pinned);
      }
      return;
    }

    int btnIndex = GetPlaylistToolbarButtonAt(xPos, yPos);
    if (btnIndex != -1) {
      if (m_onPlaylistToolbarClick) {
        m_onPlaylistToolbarClick(btnIndex);
      }
      return;
    }

    if (IsInPlaylistRegion(xPos, yPos)) {
      if (m_onPlaylistClick) {
        m_onPlaylistClick(xPos, yPos);
      }
      return;
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
            int mode = m_config->GetBackground().BackgroundArtMode;
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
            m_config->SetEnableResize(!m_config->GetWindow().EnableResize);
          }
        } else if (item.commandId == ID_LOGO_LOCK_POS) {
          if (m_config) {
            m_config->SetLockWindowPosition(
                !m_config->GetWindow().LockWindowPosition);
          }
        } else if (item.commandId == ID_LOGO_VISUALIZER) {
          if (m_config) {
            int mode = m_config->GetVisualizer().VisualizerMode;
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
            int newPos = m_config->GetLayoutPlaylist().PlaylistPosition == 0 ? 1 : 0;
            m_config->SetPlaylistPosition(newPos);
          }
        }
        return;
      }
    }
  }

  int btnId = GetPlaybackButtonAt(xPos, yPos);
  if (btnId > 0) {
    m_playbackClickedIndex = btnId;
    if (btnId == 2 || btnId == 4) {
      if (m_onSkipCommand) {
        float delta = (btnId == 2) ? -m_config->GetAudio().SkipSeconds
                                   : m_config->GetAudio().SkipSeconds;
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
    return;
  }

  if (IsInLogoRegion(xPos, yPos) && !m_isLogoMenuHovered) {
    m_isLogoClicked = true;
  }


  if (!m_config || !m_config->GetWindow().LockWindowPosition) {
    ReleaseCapture();
    SendMessage(hwnd, WM_NCLBUTTONDOWN, HTCAPTION, 0);
  }
}

void Window::HandleLButtonUp(WPARAM wParam, LPARAM lParam) {
  // Currently nothing to do
}

bool Window::HandleRButtonDown(HWND hwnd, WPARAM wParam, LPARAM lParam) {
  if (m_isArtFramingDragging) {
    m_isArtFramingDragging = false;
    ReleaseCapture();
  }
  int xPos = GET_X_LPARAM(lParam);
  int yPos = GET_Y_LPARAM(lParam);

  if (!m_isPlaylistHovered && !m_isLogoMenuHovered && !m_isControlHovered && !m_isVolumeHovered && !IsInLogoRegion(xPos, yPos)) {
      if (m_onBackgroundClickCallback) m_onBackgroundClickCallback();
      m_isArtFramingDragging = true;
      m_artFramingDragStartPt.x = xPos;
      m_artFramingDragStartPt.y = yPos;
      SetCapture(hwnd);
      return true;
  }
  return false;
}

bool Window::HandleRButtonUp(WPARAM wParam, LPARAM lParam) {
  if (m_isArtFramingDragging) {
    m_isArtFramingDragging = false;
    ReleaseCapture();
    return true;
  }
  return false;
}

void Window::HandleLButtonDblClk(WPARAM wParam, LPARAM lParam) {
  int xPos = GET_X_LPARAM(lParam);
  int yPos = GET_Y_LPARAM(lParam);

  if (m_isPlaylistHovered) {
    if (IsInPlaylistRegion(xPos, yPos)) {
      if (m_onPlaylistDoubleClick) {
        m_onPlaylistDoubleClick(xPos, yPos);
      }
    }
  }
}

bool Window::HandleMouseWheel(HWND hwnd, WPARAM wParam, LPARAM lParam) {
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
      return true;
    }

    if (m_onVolumeScroll && IsInVolumeControlRegion(pt.x, pt.y)) {
      int zDelta = GET_WHEEL_DELTA_WPARAM(wParam);
      m_onVolumeScroll(zDelta);
      return true;
    }

    if (m_isTrackInfoHovered) {
      int zDelta = GET_WHEEL_DELTA_WPARAM(wParam);
      m_virtualScrollAccumulator += zDelta;
      int threshold = m_isVirtualScrolling ? WHEEL_DELTA : (WHEEL_DELTA * 2);

      if (std::abs(m_virtualScrollAccumulator) >= threshold) {
        m_isVirtualScrolling = true;
        int direction = (m_virtualScrollAccumulator > 0) ? 1 : -1;
        if (m_onVirtualScrollCallback) {
          m_onVirtualScrollCallback(direction);
        }
        m_virtualScrollAccumulator -= direction * threshold;
      }
      return true;
    }
    
    bool inPlaylist = m_isPlaylistHovered && IsInPlaylistRegion(pt.x, pt.y);
    if (!inPlaylist && !m_isVolumeHovered) {
      if ((GetAsyncKeyState(VK_SHIFT) & 0x8000) || (wParam & MK_RBUTTON)) {
        if (m_onArtFramingScroll) {
          int zDelta = GET_WHEEL_DELTA_WPARAM(wParam);
          m_onArtFramingScroll(static_cast<float>(zDelta), pt.x, pt.y);
        }
        return true;
      }
    }
  }
  return false;
}

bool Window::HandleMButtonDown(WPARAM wParam, LPARAM lParam) {
  if (wParam & MK_RBUTTON) {
    if (m_onArtFramingReset) m_onArtFramingReset();
    return true;
  }
  return false;
}
