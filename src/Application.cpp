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
    
    if (!m_renderer.Initialize(m_window.GetHandle(), m_config)) {
        return false;
    }

    if (m_audioPlayer.Initialize()) {
        std::string defPlaylist = std::filesystem::path(m_config.GetDefaultPlaylistPath()).string();
        m_playlistManager.LoadFromFile(defPlaylist);

        if (!m_playlistManager.IsEmpty()) {
            std::string firstTrack = m_playlistManager.GetCurrentTrack();
            if (m_tagManager.Load(firstTrack)) {
                std::wstring title = m_tagManager.GetTitle();
                std::wstring artist = m_tagManager.GetArtist();
                if (title.empty()) title = std::filesystem::path(firstTrack).filename().wstring();
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
                std::wstring title = std::filesystem::path(firstTrack).filename().wstring();
                m_renderer.SetTrackInfo(title, L"---");
                m_renderer.SetAlbumArt(nullptr);
            }
            m_audioPlayer.Play(firstTrack);
            PrefetchNextTrack();
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

        if (wasEmpty && !m_audioPlayer.IsPlaying()) {
            std::string firstTrack = m_playlistManager.GetCurrentTrack();
            if (m_tagManager.Load(firstTrack)) {
                std::wstring title = m_tagManager.GetTitle();
                std::wstring artist = m_tagManager.GetArtist();
                if (title.empty()) title = std::filesystem::path(firstTrack).filename().wstring();
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
            }
            m_audioPlayer.Play(firstTrack);
            PrefetchNextTrack();
        }
    }
}

void Application::Run() {
    while (m_window.ProcessMessages()) {
        if (m_audioPlayer.IsAtEnd()) {
            // ロードが完了するまで待機（このフレームはスキップして待つ）
            if (m_isPrefetchReady.load()) {
                // 自動的に次の曲へ移行
                m_playlistManager.Advance();
                
                // 先読みデータをRendererに即時反映
                m_renderer.SetTrackInfo(m_prefetchedTitle, m_prefetchedArtist);
                m_renderer.SetAlbumArt(m_prefetchedAlbumArt.Get());

                // 次の曲を再生
                m_audioPlayer.Play(m_playlistManager.GetCurrentTrack());

                // さらにその次の曲を先読み
                PrefetchNextTrack();
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
            if (m_tagManager.Load(nextFile)) {
                m_prefetchedTitle = m_tagManager.GetTitle();
                m_prefetchedArtist = m_tagManager.GetArtist();
                
                if (m_prefetchedTitle.empty()) {
                    m_prefetchedTitle = std::filesystem::path(nextFile).filename().wstring();
                }
                if (m_prefetchedArtist.empty()) {
                    m_prefetchedArtist = L"---";
                }

                const auto& artBytes = m_tagManager.GetAlbumArtBytes();
                if (!artBytes.empty()) {
                    m_renderer.LoadBitmapFromMemory(artBytes, &m_prefetchedAlbumArt);
                }
            }
        }

        m_isPrefetchReady.store(true);

        if (SUCCEEDED(hr)) {
            CoUninitialize();
        }
    });
}
