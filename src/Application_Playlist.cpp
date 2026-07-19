#include "Application.h"
#include "LayoutCalculator.h"
#include <chrono>
#include <filesystem>
#include <iomanip>
#include <sstream>

void Application::OnPlaylistToolbarClicked(int btnIndex) {
  if (m_isPlaylistListViewMode) {
    if (btnIndex == 1) { // ➕ (新規作成)
      m_renderer.TriggerFlyText(L"NEW PLAYLIST CREATED");
      this->CreateNewPlaylist();
      m_isPlaylistListViewMode = false;
    } else if (btnIndex == 2) { // 🗑️ (リスト削除)
      m_renderer.TriggerFlyText(L"PLAYLIST DELETED");
      if (m_focusedPlaylistIndex.has_value()) {
        std::vector<std::wstring> available =
            m_config.GetAvailablePlaylists();
        if (m_focusedPlaylistIndex.value() < available.size()) {
          std::wstring targetPath = available[m_focusedPlaylistIndex.value()];
          if (std::filesystem::exists(targetPath)) {
            std::error_code ec;
            std::filesystem::remove(targetPath, ec);
          }

          UpdatePlaylistSummaries();

          if (targetPath == m_config.GetDefaultPlaylistPath()) {
            std::vector<std::wstring> newAvailable =
                m_config.GetAvailablePlaylists();
            if (!newAvailable.empty()) {
              this->SwitchPlaylist(newAvailable[0]);
            } else {
              this->CreateNewPlaylist();
            }
            m_isPlaylistListViewMode = false;
          }
        }
      }
    }
  } else {
    if (btnIndex == 0) { // 📁 (上の階層へ)
      std::wstring currentPlaylist = m_config.GetDefaultPlaylistPath();
      std::vector<std::wstring> available = m_config.GetAvailablePlaylists();
      for (size_t i = 0; i < available.size(); ++i) {
        if (available[i] == currentPlaylist) {
          m_focusedPlaylistIndex = static_cast<int>(i);
          break;
        }
      }
      m_isPlaylistListViewMode = true;
    } else if (btnIndex == 1) { // ➖ (曲削除)
      m_renderer.TriggerFlyText(L"TRACK REMOVED");
      if (!m_playlistManager.IsEmpty()) {
        m_playlistManager.RemoveCurrentTrack();
        m_playlistManager.SaveToFile(m_config.GetDefaultPlaylistPath());
        UpdatePlaylistSummaries();

        m_audioPlayer.Stop();
        if (!m_playlistManager.IsEmpty()) {
          PlayCurrentTrack();
        } else {
          m_renderer.GetTrackDrum().StartDrumAnimation(0, nullptr, nullptr);
          m_renderer.GetTrackDrum().SetAlbumArt(nullptr);
        }
      }
    } else if (btnIndex == 2) { // 🗑️ (全曲削除)
      m_renderer.TriggerFlyText(L"PLAYLIST CLEARED");
      this->ClearPlaylist();
    }
  }
}

void Application::OnPlaylistClicked(int x, int y) {
  HWND hwnd = m_window.GetHandle();
  UINT dpi = GetDpiForWindow(hwnd);
  float logicalY = static_cast<float>(y) / (static_cast<float>(dpi) / 96.0f);

  RECT rect;
  GetClientRect(hwnd, &rect);
  float logicalWidth = static_cast<float>(rect.right - rect.left) /
                       (static_cast<float>(dpi) / 96.0f);
  float logicalHeight = static_cast<float>(rect.bottom - rect.top) /
                        (static_cast<float>(dpi) / 96.0f);

  float toolbarHeight = m_config.GetPlaylistToolbarHeight();
  if (logicalY < toolbarHeight) {
    // ツールバー領域へのクリックはWindow.cpp(WM_LBUTTONDOWN)から
    // SetPlaylistToolbarClickCallbackを通じて飛んでくるためここでは無視する。
    return;
  }

  if (m_isPlaylistListViewMode) {
    std::vector<std::wstring> playlists = m_config.GetAvailablePlaylists();
    int totalPlaylists = static_cast<int>(playlists.size());
    if (totalPlaylists == 0)
      return;

    std::wstring currentPlaylist = m_config.GetDefaultPlaylistPath();
    int currentIndex = 0;
    for (int i = 0; i < totalPlaylists; ++i) {
      if (playlists[i] == currentPlaylist) {
        currentIndex = i;
        break;
      }
    }

    PlaylistLayout layout = LayoutCalculator::CalculatePlaylistLayout(
        logicalWidth, logicalHeight, &m_config, 0.0f,
        m_renderer.GetPlaylistManualScrollY(), currentIndex, totalPlaylists);
    int index = LayoutCalculator::GetPlaylistItemIndexAt(logicalY, layout,
                                                         totalPlaylists);

    if (index >= 0) {
      m_focusedPlaylistIndex = index;
    }
    return;
  }

  int totalTracks = static_cast<int>(m_playlistManager.GetCount());
  if (totalTracks == 0)
    return;

  int currentTrackIndex =
      static_cast<int>(m_playlistManager.GetCurrentIndex());

  PlaylistLayout layout = LayoutCalculator::CalculatePlaylistLayout(
      logicalWidth, logicalHeight, &m_config, 0.0f,
      m_renderer.GetPlaylistManualScrollY(), currentTrackIndex, totalTracks);
  int index =
      LayoutCalculator::GetPlaylistItemIndexAt(logicalY, layout, totalTracks);

  if (index >= 0) {
    m_focusedPlaylistIndex = index;

    if (!m_playlistManager.IsEmpty()) {
      m_playlistManager.SaveToFile(m_config.GetDefaultPlaylistPath());
      if (!m_framingDbPath.empty()) {
        m_framingDb.SaveToFile(m_framingDbPath);
      }
    }
    int oldIndex = static_cast<int>(m_playlistManager.GetCurrentIndex());
    m_playlistManager.JumpToIndex(index);

    float itemHeight = static_cast<float>(m_config.GetPlaylistItemOffsetY());
    float offsetCorrection =
        static_cast<float>(index - oldIndex) * itemHeight;
    m_renderer.AddPlaylistScroll(offsetCorrection);

    m_audioPlayer.Stop();

    auto list = m_playlistManager.GetShuffleList();
    if (index < list.size()) {
      int distance = static_cast<int>(oldIndex) - static_cast<int>(index);
      if (!PlayCurrentTrack(distance)) {
        m_renderer.GetTrackDrum().StartDrumAnimation(0, nullptr, nullptr);
        m_renderer.GetTrackDrum().SetAlbumArt(nullptr);
      }
    }
  }
}

void Application::OnPlaylistDoubleClicked(int x, int y) {
  HWND hwnd = m_window.GetHandle();
  UINT dpi = GetDpiForWindow(hwnd);
  float logicalY = static_cast<float>(y) / (static_cast<float>(dpi) / 96.0f);

  RECT rect;
  GetClientRect(hwnd, &rect);
  float logicalWidth = static_cast<float>(rect.right - rect.left) /
                       (static_cast<float>(dpi) / 96.0f);
  float logicalHeight = static_cast<float>(rect.bottom - rect.top) /
                        (static_cast<float>(dpi) / 96.0f);

  float toolbarHeight = m_config.GetPlaylistToolbarHeight();
  if (logicalY < toolbarHeight) {
    return;
  }

  if (m_isPlaylistListViewMode) {
    std::vector<std::wstring> playlists = m_config.GetAvailablePlaylists();
    int totalPlaylists = static_cast<int>(playlists.size());
    if (totalPlaylists == 0)
      return;

    std::wstring currentPlaylist = m_config.GetDefaultPlaylistPath();
    int currentIndex = 0;
    for (int i = 0; i < totalPlaylists; ++i) {
      if (playlists[i] == currentPlaylist) {
        currentIndex = i;
        break;
      }
    }

    PlaylistLayout layout = LayoutCalculator::CalculatePlaylistLayout(
        logicalWidth, logicalHeight, &m_config, 0.0f,
        m_renderer.GetPlaylistManualScrollY(), currentIndex, totalPlaylists);
    int index = LayoutCalculator::GetPlaylistItemIndexAt(logicalY, layout,
                                                         totalPlaylists);

    if (index >= 0) {
      this->SwitchPlaylist(playlists[index]);
      m_isPlaylistListViewMode = false;
    }
    return;
  }
}

void Application::ClearPlaylist() {
  m_focusedPlaylistIndex.reset();
  m_playlistManager.Clear();

  m_trackAnalyzer.ClearQueue();

  std::wstring defaultPath = m_config.GetDefaultPlaylistPath();
  m_playlistManager.SaveToFile(defaultPath);
  if (!m_framingDbPath.empty()) {
    m_framingDb.SaveToFile(m_framingDbPath);
  }
  UpdatePlaylistSummaries();

  m_audioPlayer.Stop();

  m_renderer.GetTrackDrum().StartDrumAnimation(0, nullptr, nullptr);
  m_renderer.GetTrackDrum().SetAlbumArt(nullptr);
}

void Application::SwitchPlaylist(const std::wstring &filepath) {
  std::wstring oldPath = m_config.GetDefaultPlaylistPath();
  std::filesystem::path newPath(filepath);
  std::filesystem::path currentPath(oldPath);

  std::error_code ec;
  bool isSame = std::filesystem::equivalent(newPath, currentPath, ec);
  if (ec) {
    isSame = (newPath == currentPath);
  }

  if (isSame) {
    m_isPlaylistListViewMode = false;
    return;
  }

  // 切り替え前に現在のプレイリスト状態を保存
  if (!m_playlistManager.IsEmpty()) {
    m_playlistManager.SaveToFile(oldPath);
    if (!m_framingDbPath.empty()) {
      m_framingDb.SaveToFile(m_framingDbPath);
    }
  }

  m_focusedPlaylistIndex.reset();
  m_config.SetDefaultPlaylistPath(filepath);

  // 既存の再生やキューをクリアする（ClearPlaylist()
  // はファイルを空にしてしまうので呼ばない）
  m_audioPlayer.Stop();
  m_trackAnalyzer.ClearQueue();
  m_renderer.GetTrackDrum().StartDrumAnimation(0, nullptr, nullptr);
  m_renderer.GetTrackDrum().SetAlbumArt(nullptr);

  m_playlistManager.Clear();
  m_playlistManager.LoadFromFile(filepath);
  m_playlistManager.RebuildQueue(m_config.GetShuffleMode());
  if (!m_playlistManager.IsEmpty()) {
    size_t skipCount = 0;
    bool played = false;
    size_t totalCount = m_playlistManager.GetCount();

    while (skipCount < totalCount) {
      if (PlayCurrentTrack(-1)) {
        played = true;
        break;
      }

      m_playlistManager.Advance();
      skipCount++;
    }

    if (!played) {
      m_renderer.GetTrackDrum().StartDrumAnimation(0, nullptr, nullptr);
      m_renderer.GetTrackDrum().SetAlbumArt(nullptr);
    }
  }

  std::vector<std::wstring> unparsed = m_playlistManager.GetShuffleList();
  if (!unparsed.empty()) {
    for (const auto &path : unparsed) {
      m_trackAnalyzer.AddTrackToQueue(path);
      bool isNew = false;
      uint32_t thumbId = m_thumbnailDatabase.GetOrGenerateThumbId(path, isNew);
      if (isNew) {
        m_thumbCacher.EnqueueTrack(thumbId, path);
      }
    }
  }
}

void Application::CreateNewPlaylist() {
  m_focusedPlaylistIndex.reset();
  auto now = std::chrono::system_clock::now();
  auto in_time_t = std::chrono::system_clock::to_time_t(now);
  std::tm bt{};
  localtime_s(&bt, &in_time_t);

  std::wstringstream ss;
  ss << L"playlist_" << std::put_time<wchar_t>(&bt, L"%Y%m%d_%H%M");
  std::wstring baseName = ss.str();

  std::wstring defaultPath = m_config.GetDefaultPlaylistPath();
  std::filesystem::path currentPath(defaultPath);
  std::filesystem::path dir = currentPath.parent_path();

  std::filesystem::path newPath;
  if (dir.empty()) {
    newPath = baseName + L".ozl";
  } else {
    newPath = dir / (baseName + L".ozl");
  }

  int sequence = 1;
  while (std::filesystem::exists(newPath)) {
    std::wstringstream seq_ss;
    seq_ss << baseName << L"_" << sequence << L".ozl";
    if (dir.empty()) {
      newPath = seq_ss.str();
    } else {
      newPath = dir / seq_ss.str();
    }
    sequence++;
  }

  // 新しいパスに変更する前に現在の状態を保存
  if (!m_playlistManager.IsEmpty()) {
    m_playlistManager.SaveToFile(defaultPath);
  }

  m_config.SetDefaultPlaylistPath(newPath.wstring());
  ClearPlaylist();
}
