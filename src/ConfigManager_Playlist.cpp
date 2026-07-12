#include "ConfigManager.h"
#include <algorithm>
#include <filesystem>
#include <string>
#include <vector>

void ConfigManager::SetIsPlaylistPinned(bool pinned) {
  m_isPlaylistPinned = pinned;
  WritePrivateProfileStringW(L"Layout_Playlist", L"IsPlaylistPinned",
                             pinned ? L"1" : L"0", m_iniFilePath.c_str());
}

void ConfigManager::SetDefaultPlaylistPath(const std::wstring &path) {
  m_defaultPlaylistPath = path;
  WritePrivateProfileStringW(L"Playlist", L"DefaultPlaylistPath", path.c_str(),
                             m_iniFilePath.c_str());
}

void ConfigManager::SetPlaylistPosition(int position) {
  m_playlistPosition = position;
  WritePrivateProfileStringW(L"Layout_Playlist", L"PlaylistPosition",
                             std::to_wstring(position).c_str(),
                             m_iniFilePath.c_str());
}

std::vector<std::wstring> ConfigManager::GetAvailablePlaylists() const {
  std::vector<std::wstring> playlists;
  try {
    std::filesystem::path currentPath(m_defaultPlaylistPath);
    std::filesystem::path dir = currentPath.parent_path();

    if (dir.empty()) {
      std::wstring exePath = GetExecutablePath();
      if (!exePath.empty()) {
        size_t pos = exePath.find_last_of(L"\\/");
        if (pos != std::wstring::npos) {
          dir = exePath.substr(0, pos);
        }
      }
      if (dir.empty()) {
        dir = std::filesystem::current_path();
      }
    }

    if (std::filesystem::exists(dir) && std::filesystem::is_directory(dir)) {
      for (const auto &entry : std::filesystem::directory_iterator(dir)) {
        if (entry.is_regular_file()) {
          std::wstring ext = entry.path().extension().wstring();
          std::transform(ext.begin(), ext.end(), ext.begin(), ::towlower);
          if (ext == L".ozl" || ext == L".lst") {
            playlists.push_back(entry.path().wstring());
          }
        }
      }
    }
  } catch (...) {
    // ignore errors
  }

  std::sort(playlists.begin(), playlists.end());
  return playlists;
}

void ConfigManager::LoadPlaylistSettings() {
    wchar_t pathBuf[MAX_PATH];
    GetPrivateProfileStringW(L"Playlist", L"DefaultPlaylistPath", L"", pathBuf, MAX_PATH, m_iniFilePath.c_str());
    std::wstring loadedPlaylistPath = pathBuf;
    if (loadedPlaylistPath.empty()) {
        std::wstring exePath = GetExecutablePath();
        size_t pos = exePath.find_last_of(L"\\/");
        if (pos != std::wstring::npos) {
            m_defaultPlaylistPath = exePath.substr(0, pos) + L"\\playlist.ozl";
        } else {
            m_defaultPlaylistPath = L"playlist.ozl";
        }
        WritePrivateProfileStringW(L"Playlist", L"DefaultPlaylistPath", m_defaultPlaylistPath.c_str(), m_iniFilePath.c_str());
    } else {
        m_defaultPlaylistPath = loadedPlaylistPath;
    }

    m_playlistPosition = LoadOrWriteInt(L"Layout_Playlist", L"PlaylistPosition", 1);
    m_isPlaylistPinned = LoadOrWriteInt(L"Layout_Playlist", L"IsPlaylistPinned", 0) != 0;
    m_playlistHoverWidth = LoadOrWriteInt(L"Layout_Playlist", L"PlaylistHoverWidth", 120);
    m_playlistWidth = LoadOrWriteInt(L"Layout_Playlist", L"PlaylistWidth", 250);
    m_playlistItemOffsetY = LoadOrWriteInt(L"Layout_Playlist", L"PlaylistItemOffsetY", 45);

    m_playingItemColor = LoadOrWriteString(L"Layout_Playlist", L"PlayingItemColor", L"#FFA500");
    m_hoverItemColor = LoadOrWriteString(L"Layout_Playlist", L"HoverItemColor", L"#a3a362");

    m_playlistTitleFontSize = LoadOrWriteFloat(L"Layout_Playlist", L"PlaylistTitleFontSize", 16.0f);
    m_playlistTitleFontFamily = LoadOrWriteString(L"Layout_Playlist", L"PlaylistTitleFontFamily", L"Meiryo");
    m_playlistTitleOffsetX = LoadOrWriteInt(L"Layout_Playlist", L"PlaylistTitleOffsetX", 20);
    m_playlistTitleOffsetY = LoadOrWriteInt(L"Layout_Playlist", L"PlaylistTitleOffsetY", 4);

    m_playlistArtistFontSize = LoadOrWriteFloat(L"Layout_Playlist", L"PlaylistArtistFontSize", 12.0f);
    m_playlistArtistColor = LoadOrWriteString(L"Layout_Playlist", L"PlaylistArtistColor", L"#888888");
    m_playlistArtistFontFamily = LoadOrWriteString(L"Layout_Playlist", L"PlaylistArtistFontFamily", L"Meiryo");
    m_playlistArtistOffsetX = LoadOrWriteInt(L"Layout_Playlist", L"PlaylistArtistOffsetX", 20);
    m_playlistArtistOffsetY = LoadOrWriteInt(L"Layout_Playlist", L"PlaylistArtistOffsetY", 25);

    m_playlistTimeFontSize = LoadOrWriteFloat(L"Layout_Playlist", L"PlaylistTimeFontSize", 12.0f);
    m_playlistTimeColor = LoadOrWriteString(L"Layout_Playlist", L"PlaylistTimeColor", L"#888888");
    m_playlistTimeFontFamily = LoadOrWriteString(L"Layout_Playlist", L"PlaylistTimeFontFamily", L"Meiryo");
    m_playlistTimeOffsetX = LoadOrWriteInt(L"Layout_Playlist", L"PlaylistTimeOffsetX", 10);
    m_playlistTimeOffsetY = LoadOrWriteInt(L"Layout_Playlist", L"PlaylistTimeOffsetY", 25);
    m_playlistTimeLetterSpacing = LoadOrWriteFloat(L"Layout_Playlist", L"PlaylistTimeLetterSpacing", 0.0f);

    m_playlistBgOpacity = LoadOrWriteFloat(L"Layout_Playlist", L"PlaylistBgOpacity", 0.8f);
    m_playlistGripOffset = LoadOrWriteFloat(L"Layout_Playlist", L"PlaylistGripOffset", 5.0f);
    m_playlistGripLineWidth = LoadOrWriteFloat(L"Layout_Playlist", L"PlaylistGripLineWidth", 1.0f);
    m_playlistGripLineColor = LoadOrWriteString(L"Layout_Playlist", L"PlaylistGripLineColor", L"#AAAAAA");
    m_playlistGripArrowHeight = LoadOrWriteFloat(L"Layout_Playlist", L"PlaylistGripArrowHeight", 35.0f);
    m_playlistGripArrowWidth = LoadOrWriteFloat(L"Layout_Playlist", L"PlaylistGripArrowWidth", 15.0f);
    m_playlistGripArrowColor = LoadOrWriteString(L"Layout_Playlist", L"PlaylistGripArrowColor", L"#AAAAAA");
    m_playlistGripShadowOffsetX = LoadOrWriteFloat(L"Layout_Playlist", L"PlaylistGripShadowOffsetX", 2.0f);
    m_playlistGripShadowOffsetY = LoadOrWriteFloat(L"Layout_Playlist", L"PlaylistGripShadowOffsetY", 2.0f);
    m_playlistGripShadowOpacity = LoadOrWriteFloat(L"Layout_Playlist", L"PlaylistGripShadowOpacity", 0.7f);

    m_playlistToolbarHeight = LoadOrWriteFloat(L"Layout_Playlist", L"ToolbarHeight", 60.0f);
    m_playlistToolbarIconSize = LoadOrWriteFloat(L"Layout_Playlist", L"ToolbarIconSize", 18.0f);
    m_playlistToolbarIconSpacing = LoadOrWriteFloat(L"Layout_Playlist", L"ToolbarIconSpacing", 10.0f);
    m_playlistToolbarTextOffsetY = LoadOrWriteFloat(L"Layout_Playlist", L"ToolbarTextOffsetY", 30.0f);
    m_playlistToolbarTextFontSize = LoadOrWriteFloat(L"Layout_Playlist", L"ToolbarTextFontSize", 12.0f);

    m_pinSubIconOffsetX = LoadOrWriteInt(L"Layout_Playlist", L"PinSubIconOffsetX", 6);
    m_pinSubIconOffsetY = LoadOrWriteInt(L"Layout_Playlist", L"PinSubIconOffsetY", 6);
    m_pinSubIconFontSize = LoadOrWriteFloat(L"Layout_Playlist", L"PinSubIconFontSize", 10.0f);

    m_playlistLeaveDelay = LoadOrWriteFloat(L"Layout_Playlist", L"PlaylistLeaveDelay", 0.5f);
}
