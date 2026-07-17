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

    m_playlistPosition = LoadOrWriteInt(L"Layout_Playlist", L"PlaylistPosition");
    m_isPlaylistPinned = LoadOrWriteInt(L"Layout_Playlist", L"IsPlaylistPinned") != 0;
    m_playlistHoverWidth = LoadOrWriteInt(L"Layout_Playlist", L"PlaylistHoverWidth");
    m_playlistWidth = LoadOrWriteInt(L"Layout_Playlist", L"PlaylistWidth");
    m_playlistItemOffsetY = LoadOrWriteInt(L"Layout_Playlist", L"PlaylistItemOffsetY");

    m_playlistTitleFontSize = LoadOrWriteFloat(L"Layout_Playlist", L"PlaylistTitleFontSize");
    m_playlistTitleOffsetX = LoadOrWriteInt(L"Layout_Playlist", L"PlaylistTitleOffsetX");
    m_playlistTitleOffsetY = LoadOrWriteInt(L"Layout_Playlist", L"PlaylistTitleOffsetY");

    m_playlistArtistFontSize = LoadOrWriteFloat(L"Layout_Playlist", L"PlaylistArtistFontSize");
    m_playlistArtistColor = LoadOrWriteString(L"Layout_Playlist", L"PlaylistArtistColor");
    m_playlistArtistOffsetX = LoadOrWriteInt(L"Layout_Playlist", L"PlaylistArtistOffsetX");
    m_playlistArtistOffsetY = LoadOrWriteInt(L"Layout_Playlist", L"PlaylistArtistOffsetY");

    m_playlistTimeFontSize = LoadOrWriteFloat(L"Layout_Playlist", L"PlaylistTimeFontSize");
    m_playlistTimeColor = LoadOrWriteString(L"Layout_Playlist", L"PlaylistTimeColor");
    m_playlistTimeOffsetX = LoadOrWriteInt(L"Layout_Playlist", L"PlaylistTimeOffsetX");
    m_playlistTimeOffsetY = LoadOrWriteInt(L"Layout_Playlist", L"PlaylistTimeOffsetY");
    m_playlistTimeLetterSpacing = LoadOrWriteFloat(L"Layout_Playlist", L"PlaylistTimeLetterSpacing");

    m_playlistBgOpacity = LoadOrWriteFloat(L"Layout_Playlist", L"PlaylistBgOpacity");
    m_playlistGripOffset = LoadOrWriteFloat(L"Layout_Playlist", L"PlaylistGripOffset");
    m_playlistGripLineWidth = LoadOrWriteFloat(L"Layout_Playlist", L"PlaylistGripLineWidth");
    m_playlistGripLineColor = LoadOrWriteString(L"Layout_Playlist", L"PlaylistGripLineColor");
    m_playlistGripArrowHeight = LoadOrWriteFloat(L"Layout_Playlist", L"PlaylistGripArrowHeight");
    m_playlistGripArrowWidth = LoadOrWriteFloat(L"Layout_Playlist", L"PlaylistGripArrowWidth");
    m_playlistGripArrowColor = LoadOrWriteString(L"Layout_Playlist", L"PlaylistGripArrowColor");

    m_playlistToolbarHeight = LoadOrWriteFloat(L"Layout_Playlist", L"ToolbarHeight");
    m_playlistToolbarIconSize = LoadOrWriteFloat(L"Layout_Playlist", L"ToolbarIconSize");
    m_playlistToolbarIconSpacing = LoadOrWriteFloat(L"Layout_Playlist", L"ToolbarIconSpacing");
    m_playlistToolbarTextOffsetY = LoadOrWriteFloat(L"Layout_Playlist", L"ToolbarTextOffsetY");
    m_playlistToolbarTextFontSize = LoadOrWriteFloat(L"Layout_Playlist", L"ToolbarTextFontSize");

    m_pinSubIconOffsetX = LoadOrWriteInt(L"Layout_Playlist", L"PinSubIconOffsetX");
    m_pinSubIconOffsetY = LoadOrWriteInt(L"Layout_Playlist", L"PinSubIconOffsetY");
    m_pinSubIconFontSize = LoadOrWriteFloat(L"Layout_Playlist", L"PinSubIconFontSize");

    m_playlistTrackCountOffsetX = LoadOrWriteFloat(L"Layout_Playlist", L"TrackCountOffsetX");
    m_playlistTrackCountBoxWidth = LoadOrWriteFloat(L"Layout_Playlist", L"TrackCountBoxWidth");
    m_playlistTrackCountUnderLineX = LoadOrWriteFloat(L"Layout_Playlist", L"TrackCountUnderLineX");
    m_playlistTrackCountUnderLineWidth = LoadOrWriteFloat(L"Layout_Playlist", L"TrackCountUnderLineWidth");
    m_playlistTrackCountFontSize = LoadOrWriteFloat(L"Layout_Playlist", L"TrackCountFontSize");
    m_playlistTrackCountLetterSpacing = LoadOrWriteFloat(L"Layout_Playlist", L"TrackCountLetterSpacing");
    m_playlistTrackCountBoxFontColor = LoadOrWriteString(L"Layout_Playlist", L"TrackCountBoxFontColor");
    m_playlistTrackCountBoxBaseColor = LoadOrWriteString(L"Layout_Playlist", L"TrackCountBoxBaseColor");
    m_playlistTrackCountBoxBaseOpacity = LoadOrWriteFloat(L"Layout_Playlist", L"TrackCountBoxBaseOpacity");

    m_playlistLeaveDelay = LoadOrWriteFloat(L"Layout_Playlist", L"PlaylistLeaveDelay");
}
