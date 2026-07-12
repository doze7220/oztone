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
