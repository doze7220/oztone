#include "Application.h"
#include <algorithm>
#include <filesystem>
#include <fstream>

void Application::OnFilesDropped(const std::vector<std::wstring> &paths) {
  bool wasEmpty = m_playlistManager.IsEmpty();
  bool addedAny = false;

  auto IsSupportedAudioFile = [](const std::filesystem::path &p) {
    std::wstring ext = p.extension().wstring();
    std::transform(ext.begin(), ext.end(), ext.begin(), ::towlower);
    if (ext == L".mp3" || ext == L".flac" || ext == L".wav" || ext == L".ogg") {
      return true;
    }
    // [EXPERIMENTAL] MP4/M4A Support
    // if (ext == L".mp4" || ext == L".m4a") {
    //   return true;
    // }
    return false;
  };

  auto IsValidAudioFile = [](const std::filesystem::path &p) {
    std::wstring ext = p.extension().wstring();
    std::transform(ext.begin(), ext.end(), ext.begin(), ::towlower);

    // if (ext == L".mp4" || ext == L".m4a") {
    //   return true;
    // }

    std::ifstream file(p, std::ios::binary);
    if (!file.is_open())
      return false;

    unsigned char header[4] = {0};
    file.read(reinterpret_cast<char *>(header), 4);
    std::streamsize bytesRead = file.gcount();

    if (bytesRead >= 4) {
      if (header[0] == 'f' && header[1] == 'L' && header[2] == 'a' &&
          header[3] == 'C')
        return true;
      if (header[0] == 'O' && header[1] == 'g' && header[2] == 'g' &&
          header[3] == 'S')
        return true;
      if (header[0] == 'R' && header[1] == 'I' && header[2] == 'F' &&
          header[3] == 'F')
        return true;
    }

    if (bytesRead >= 2) {
      if (header[0] == 'I' && header[1] == 'D' && header[2] == '3')
        return true;
      if (header[0] == 0xFF && (header[1] & 0xE0) == 0xE0)
        return true;
    }

    return false;
  };

  std::wstring firstAddedTrack;

  for (const auto &pathWStr : paths) {
    try {
      std::filesystem::path p(pathWStr);
      if (std::filesystem::is_directory(p)) {
        for (auto it = std::filesystem::recursive_directory_iterator(p);
             it != std::filesystem::recursive_directory_iterator(); ++it) {
          if (it->is_regular_file()) {
            bool isSupported = IsSupportedAudioFile(it->path());
            bool isValid = IsValidAudioFile(it->path());

            if (isSupported && isValid) {
              if (m_playlistManager.Add(it->path().wstring())) {
                addedAny = true;
                if (firstAddedTrack.empty())
                  firstAddedTrack = it->path().wstring();
              }
            }
          }
        }
      } else if (std::filesystem::is_regular_file(p)) {
        bool isSupported = IsSupportedAudioFile(p);
        bool isValid = IsValidAudioFile(p);

        if (isSupported && isValid) {
          if (m_playlistManager.Add(p.wstring())) {
            addedAny = true;
            if (firstAddedTrack.empty())
              firstAddedTrack = p.wstring();
          }
        }
      }
    } catch (...) {
      // アクセス拒否などのエラーは無視
    }
  }

  if (addedAny) {
    std::filesystem::path defaultPath = m_config.GetDefaultPlaylistPath();
    std::filesystem::path playlistDir = defaultPath.parent_path();
    if (!playlistDir.empty() && !std::filesystem::exists(playlistDir)) {
      std::filesystem::create_directories(playlistDir);
    }
    m_playlistManager.SaveToFile(defaultPath.wstring());
    UpdatePlaylistSummaries();

    bool isShiftPressed = (GetAsyncKeyState(VK_SHIFT) & 0x8000) != 0;
    if (!isShiftPressed) {
      m_playlistManager.RebuildQueue(m_config.GetShuffleMode());
      if (!firstAddedTrack.empty()) {
        m_playlistManager.WarpToTrack(firstAddedTrack);
      }
    } else {
      m_playlistManager.ShuffleNextLoop();
    }

    std::vector<std::wstring> unparsed = m_playlistManager.GetShuffleList();
    if (!unparsed.empty()) {
      for (const auto &path : unparsed) {
        m_trackAnalyzer.AddTrackToQueue(path);
      }
    }

    if (!isShiftPressed || (wasEmpty && !m_audioPlayer.IsPlaying())) {
      m_audioPlayer.Stop();

      size_t skipCount = 0;
      bool played = false;
      size_t totalCount = m_playlistManager.GetCount();

      while (skipCount < totalCount) {
        std::wstring currentTrack = m_playlistManager.GetCurrentTrack();
        if (m_tagManager.Load(currentTrack)) {
          std::wstring title = m_tagManager.GetTitle();
          std::wstring artist = m_tagManager.GetArtist();
          if (title.empty())
            title = std::filesystem::path(currentTrack).filename().wstring();
          if (artist.empty())
            artist = L"---";
          m_renderer.SetTrackInfo(title, artist, L"", DrumMoveType::Jump);

          const auto &artBytes = m_tagManager.GetAlbumArtBytes();
          if (!artBytes.empty()) {
            Microsoft::WRL::ComPtr<ID2D1Bitmap> artBitmap;
            if (m_renderer.LoadBitmapFromMemory(artBytes, &artBitmap)) {
              m_renderer.SetAlbumArt(artBitmap.Get());
            } else {
              m_renderer.SetAlbumArt(nullptr);
            }
          } else {
            m_renderer.SetAlbumArt(nullptr);
          }
        } else {
          std::wstring title;
          try {
            title = std::filesystem::path(currentTrack).filename().wstring();
          } catch (...) {
            title = L"UNKNOWN";
          }
          m_renderer.SetTrackInfo(title, L"---", L"", DrumMoveType::Jump);
          m_renderer.SetAlbumArt(nullptr);
        }

        float artX = 0.0f, artY = 0.0f, artScale = 1.0f;
      m_framingDb.GetFraming(currentTrack, artX, artY, artScale);
      m_renderer.SetBackgroundFraming(artX, artY, artScale);
      if (PlayCurrentTrack(DrumMoveType::Jump)) {
          played = true;
          break;
        }

        m_playlistManager.Advance();
        skipCount++;
      }

      if (!played) {
        m_renderer.SetTrackInfo(L"NO TRACK", L"---", L"", DrumMoveType::Reset);
        m_renderer.SetAlbumArt(nullptr);
      }
    }
  }
}

void Application::ProcessCommandLineArgs(int argc, LPWSTR *argv) {
  if (argc <= 1 || !argv)
    return;
  std::vector<std::wstring> files;
  for (int i = 1; i < argc; ++i) {
    files.push_back(argv[i]);
  }
  if (!files.empty()) {
    OnFilesDropped(files);
  }
}
