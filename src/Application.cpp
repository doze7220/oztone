#include "Application.h"
#include <filesystem>
#include <fstream>
#include <algorithm>

Application::Application() {}

Application::~Application() {
    if (m_prefetchThread.joinable()) {
        m_prefetchThread.join();
    }
    m_audioPlayer.Uninitialize();
}

bool Application::Initialize(HINSTANCE hInstance, int nCmdShow) {
    if (!m_config.Initialize()) {
        return false;
    }

    if (!m_window.Initialize(hInstance, nCmdShow, m_config)) {
        return false;
    }
    
    m_window.SetOnFilesDroppedCallback([this](const std::vector<std::wstring>& files) {
        this->OnFilesDropped(files);
    });
    
    m_window.SetCopyDataCallback([this](const std::wstring& path) {
        std::vector<std::wstring> files = { path };
        this->OnFilesDropped(files);
    });
    
    m_window.SetClearPlaylistCallback([this]() {
        this->ClearPlaylist();
    });
    
    m_window.SetMediaCommandCallback([this](int cmd) {
        if (cmd == APPCOMMAND_MEDIA_PLAY_PAUSE) {
            m_audioPlayer.TogglePlayPause();
        } else if (cmd == APPCOMMAND_MEDIA_STOP) {
            m_audioPlayer.Stop();
        } else if (cmd == APPCOMMAND_MEDIA_NEXTTRACK || cmd == APPCOMMAND_MEDIA_PREVIOUSTRACK) {
            if (cmd == APPCOMMAND_MEDIA_NEXTTRACK) {
                m_playlistManager.Advance();
            } else {
                m_playlistManager.Previous();
            }
            
            size_t skipCount = 0;
            bool played = false;
            size_t totalCount = m_playlistManager.GetCount();
            
            m_audioPlayer.Stop();
            
            while (skipCount < totalCount) {
                std::string track = m_playlistManager.GetCurrentTrack();

                if (cmd == APPCOMMAND_MEDIA_NEXTTRACK && skipCount == 0 && m_isPrefetchReady.load()) {
                    m_renderer.SetTrackInfo(m_prefetchedTitle, m_prefetchedArtist);
                    m_renderer.SetAlbumArt(m_prefetchedAlbumArt.Get());
                } else {
                    if (m_tagManager.Load(track)) {
                        std::wstring title = m_tagManager.GetTitle();
                        std::wstring artist = m_tagManager.GetArtist();
                        if (title.empty()) title = std::filesystem::path(track).filename().wstring();
                        if (artist.empty()) artist = L"---";
                        m_renderer.SetTrackInfo(title, artist);

                        const auto& artBytes = m_tagManager.GetAlbumArtBytes();
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
                        try { title = std::filesystem::path(track).filename().wstring(); } catch(...) { title = L"Unknown"; }
                        m_renderer.SetTrackInfo(title, L"---");
                        m_renderer.SetAlbumArt(nullptr);
                    }
                }

                if (m_audioPlayer.Play(track)) {
                    PrefetchNextTrack();
                    played = true;
                    break;
                }

                if (cmd == APPCOMMAND_MEDIA_PREVIOUSTRACK) {
                    m_playlistManager.Previous();
                } else {
                    m_playlistManager.Advance();
                }
                skipCount++;
            }

            if (!played) {
                m_renderer.SetTrackInfo(L"No Track", L"---");
                m_renderer.SetAlbumArt(nullptr);
            }
        }
    });
    
    if (!m_renderer.Initialize(m_window.GetHandle(), m_config)) {
        return false;
    }

    if (m_audioPlayer.Initialize()) {
        std::string defPlaylist = std::filesystem::path(m_config.GetDefaultPlaylistPath()).string();
        m_playlistManager.LoadFromFile(defPlaylist);

        if (!m_playlistManager.IsEmpty()) {
            size_t skipCount = 0;
            bool played = false;
            size_t totalCount = m_playlistManager.GetCount();

            while (skipCount < totalCount) {
                std::string currentTrack = m_playlistManager.GetCurrentTrack();
                if (m_tagManager.Load(currentTrack)) {
                    std::wstring title = m_tagManager.GetTitle();
                    std::wstring artist = m_tagManager.GetArtist();
                    if (title.empty()) title = std::filesystem::path(currentTrack).filename().wstring();
                    if (artist.empty()) artist = L"---";
                    m_renderer.SetTrackInfo(title, artist);

                    const auto& artBytes = m_tagManager.GetAlbumArtBytes();
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
                    try { title = std::filesystem::path(currentTrack).filename().wstring(); } catch(...) { title = L"Unknown"; }
                    m_renderer.SetTrackInfo(title, L"---");
                    m_renderer.SetAlbumArt(nullptr);
                }

                if (m_audioPlayer.Play(currentTrack)) {
                    PrefetchNextTrack();
                    played = true;
                    break;
                }

                m_playlistManager.Advance();
                skipCount++;
            }

            if (!played) {
                // UIの初期表示（空状態）
                m_renderer.SetTrackInfo(L"No Track", L"---");
                m_renderer.SetAlbumArt(nullptr);
            }
        } else {
            // UIの初期表示（空状態）
            m_renderer.SetTrackInfo(L"No Track", L"---");
            m_renderer.SetAlbumArt(nullptr);
        }
    }

    return true;
}

void Application::OnFilesDropped(const std::vector<std::wstring>& paths) {
    bool wasEmpty = m_playlistManager.IsEmpty();
    bool addedAny = false;

    auto IsMp3File = [](const std::filesystem::path& p) {
        if (p.extension().wstring() == L".mp3" || p.extension().wstring() == L".MP3") return true;
        std::wstring ext = p.extension().wstring();
        std::transform(ext.begin(), ext.end(), ext.begin(), ::towlower);
        return ext == L".mp3";
    };

    auto IsValidMp3 = [](const std::filesystem::path& p) {
        std::ifstream file(p, std::ios::binary);
        if (!file.is_open()) return false;
        unsigned char header[3] = {0};
        file.read(reinterpret_cast<char*>(header), 3);
        if (file.gcount() >= 2) {
            // ID3v2: "ID3"
            if (header[0] == 'I' && header[1] == 'D' && header[2] == '3') return true;
            // MP3 Sync word: 0xFF 0xFB, 0xFF 0xFA, 0xFF 0xF3, etc...
            if (header[0] == 0xFF && (header[1] & 0xE0) == 0xE0) return true;
        }
        return false;
    };

    for (const auto& pathWStr : paths) {
        try {
            std::filesystem::path p(pathWStr);
            if (std::filesystem::is_directory(p)) {
                for (auto it = std::filesystem::recursive_directory_iterator(p); it != std::filesystem::recursive_directory_iterator(); ++it) {
                    if (it->is_regular_file()) {
                        bool isMp3 = IsMp3File(it->path());
                        bool isValid = IsValidMp3(it->path());
                        char dbg[1024];
                        sprintf_s(dbg, "File: %ls, IsMp3: %d, IsValid: %d\n", it->path().wstring().c_str(), isMp3, isValid);
                        OutputDebugStringA(dbg);
                        
                        if (isMp3 && isValid) {
                            if (m_playlistManager.Add(it->path().string())) {
                                addedAny = true;
                            }
                        }
                    }
                }
            } else if (std::filesystem::is_regular_file(p)) {
                bool isMp3 = IsMp3File(p);
                bool isValid = IsValidMp3(p);
                char dbg[1024];
                sprintf_s(dbg, "File: %ls, IsMp3: %d, IsValid: %d\n", p.wstring().c_str(), isMp3, isValid);
                OutputDebugStringA(dbg);
                
                if (isMp3 && isValid) {
                    if (m_playlistManager.Add(p.string())) {
                        addedAny = true;
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
        m_playlistManager.SaveToFile(defaultPath.string());
        m_playlistManager.ShuffleNextLoop();

        if (wasEmpty && !m_audioPlayer.IsPlaying()) {
            size_t skipCount = 0;
            bool played = false;
            size_t totalCount = m_playlistManager.GetCount();

            while (skipCount < totalCount) {
                std::string currentTrack = m_playlistManager.GetCurrentTrack();
                if (m_tagManager.Load(currentTrack)) {
                    std::wstring title = m_tagManager.GetTitle();
                    std::wstring artist = m_tagManager.GetArtist();
                    if (title.empty()) title = std::filesystem::path(currentTrack).filename().wstring();
                    if (artist.empty()) artist = L"---";
                    m_renderer.SetTrackInfo(title, artist);

                    const auto& artBytes = m_tagManager.GetAlbumArtBytes();
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
                    try { title = std::filesystem::path(currentTrack).filename().wstring(); } catch(...) { title = L"Unknown"; }
                    m_renderer.SetTrackInfo(title, L"---");
                    m_renderer.SetAlbumArt(nullptr);
                }

                if (m_audioPlayer.Play(currentTrack)) {
                    PrefetchNextTrack();
                    played = true;
                    break;
                }

                m_playlistManager.Advance();
                skipCount++;
            }

            if (!played) {
                m_renderer.SetTrackInfo(L"No Track", L"---");
                m_renderer.SetAlbumArt(nullptr);
            }
        }
    }
}

void Application::Run() {
    while (m_window.ProcessMessages()) {
        if (m_audioPlayer.IsAtEnd()) {
            // ロードが完了するまで待機（このフレームはスキップして待つ）
            if (m_isPrefetchReady.load()) {
                m_playlistManager.Advance();
                
                size_t skipCount = 0;
                bool played = false;
                size_t totalCount = m_playlistManager.GetCount();
                
                while (skipCount < totalCount) {
                    std::string track = m_playlistManager.GetCurrentTrack();

                    if (skipCount == 0) {
                        // 最初の曲は先読みデータを利用
                        m_renderer.SetTrackInfo(m_prefetchedTitle, m_prefetchedArtist);
                        m_renderer.SetAlbumArt(m_prefetchedAlbumArt.Get());
                    } else {
                        // スキップされた場合は同期的にタグを読み直す
                        if (m_tagManager.Load(track)) {
                            std::wstring title = m_tagManager.GetTitle();
                            std::wstring artist = m_tagManager.GetArtist();
                            if (title.empty()) title = std::filesystem::path(track).filename().wstring();
                            if (artist.empty()) artist = L"---";
                            m_renderer.SetTrackInfo(title, artist);
                            
                            const auto& artBytes = m_tagManager.GetAlbumArtBytes();
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
                            try { title = std::filesystem::path(track).filename().wstring(); } catch(...) { title = L"Unknown"; }
                            m_renderer.SetTrackInfo(title, L"---");
                            m_renderer.SetAlbumArt(nullptr);
                        }
                    }

                    if (m_audioPlayer.Play(track)) {
                        PrefetchNextTrack();
                        played = true;
                        break;
                    }

                    m_playlistManager.Advance();
                    skipCount++;
                }

                if (!played) {
                    m_renderer.SetTrackInfo(L"No Track", L"---");
                    m_renderer.SetAlbumArt(nullptr);
                }
            }
        }

        float posSec = m_audioPlayer.GetPositionSeconds();
        float lenSec = m_audioPlayer.GetLengthSeconds();

        int posM = static_cast<int>(posSec) / 60;
        int posS = static_cast<int>(posSec) % 60;
        int lenM = static_cast<int>(lenSec) / 60;
        int lenS = static_cast<int>(lenSec) % 60;

        wchar_t timeBuf[32];
        swprintf_s(timeBuf, L"%d:%02d / %d:%02d", posM, posS, lenM, lenS);
        std::wstring timeString(timeBuf);

        float progress = 0.0f;
        if (lenSec > 0.0f) {
            progress = posSec / lenSec;
            if (progress > 1.0f) progress = 1.0f;
            if (progress < 0.0f) progress = 0.0f;
        }

        m_renderer.SetNextTrackInfo(
            m_isPrefetchReady.load(),
            m_prefetchedAlbumArt.Get(),
            m_prefetchedTitle,
            m_prefetchedArtist
        );

        m_renderer.Render(m_window.IsHovered(), progress, timeString);
        Sleep(1); // CPU使用率を抑えるための仮のスリープ
    }
}

void Application::PrefetchNextTrack() {
    m_isPrefetchReady.store(false);

    if (m_prefetchThread.joinable()) {
        m_prefetchThread.join();
    }

    m_prefetchThread = std::thread([this]() {
        HRESULT hr = CoInitializeEx(nullptr, COINIT_MULTITHREADED);

        m_prefetchedTitle.clear();
        m_prefetchedArtist.clear();
        m_prefetchedAlbumArt.Reset();

        std::string nextFile = m_playlistManager.GetNextTrack();
        if (!nextFile.empty()) {
            bool loadSuccess = false;
            try {
                if (std::filesystem::exists(std::filesystem::path(nextFile))) {
                    loadSuccess = m_tagManager.Load(nextFile);
                }
            } catch (...) {
                loadSuccess = false;
            }

            if (loadSuccess) {
                m_prefetchedTitle = m_tagManager.GetTitle();
                m_prefetchedArtist = m_tagManager.GetArtist();
                
                if (m_prefetchedTitle.empty()) {
                    try { m_prefetchedTitle = std::filesystem::path(nextFile).filename().wstring(); } catch(...) { m_prefetchedTitle = L"Unknown"; }
                }
                if (m_prefetchedArtist.empty()) {
                    m_prefetchedArtist = L"---";
                }

                const auto& artBytes = m_tagManager.GetAlbumArtBytes();
                if (!artBytes.empty()) {
                    m_renderer.LoadBitmapFromMemory(artBytes, &m_prefetchedAlbumArt);
                }
            } else {
                try { m_prefetchedTitle = std::filesystem::path(nextFile).filename().wstring(); } catch(...) { m_prefetchedTitle = L"Unknown"; }
                m_prefetchedArtist = L"---";
            }
        }

        m_isPrefetchReady.store(true);

        if (SUCCEEDED(hr)) {
            CoUninitialize();
        }
    });
}

void Application::ProcessCommandLineArgs(int argc, LPWSTR* argv) {
    if (argc <= 1 || !argv) return;
    std::vector<std::wstring> files;
    for (int i = 1; i < argc; ++i) {
        files.push_back(argv[i]);
    }
    if (!files.empty()) {
        OnFilesDropped(files);
    }
}

void Application::ClearPlaylist() {
    m_playlistManager.Clear();
    
    std::string defaultPath = std::filesystem::path(m_config.GetDefaultPlaylistPath()).string();
    m_playlistManager.SaveToFile(defaultPath);
    
    m_audioPlayer.Stop();
    
    m_isPrefetchReady.store(false);
    m_renderer.SetTrackInfo(L"No Track", L"---");
    m_renderer.SetAlbumArt(nullptr);
}
