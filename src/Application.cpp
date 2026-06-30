#include "Application.h"
#include <filesystem>

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
    
    if (!m_renderer.Initialize(m_window.GetHandle(), m_config)) {
        return false;
    }

    if (m_audioPlayer.Initialize()) {
        wchar_t exePathBuf[MAX_PATH];
        GetModuleFileNameW(NULL, exePathBuf, MAX_PATH);
        std::filesystem::path exePath(exePathBuf);
        // build/Debug/OZtone.exe または build/Release/OZtone.exe を想定して3つ上へ
        std::filesystem::path projectRoot = exePath.parent_path().parent_path().parent_path();
        
        // FIXME: 一時的なテストコード。次のステップ(D&D実装)で削除・置換する
        std::filesystem::path assetPath1 = projectRoot / L"assets" / L"test1.mp3";
        std::filesystem::path assetPath2 = projectRoot / L"assets" / L"test2.mp3";
        std::filesystem::path assetPath3 = projectRoot / L"assets" / L"test3.mp3";
        
        m_playlistManager.Add(assetPath1.string());
        m_playlistManager.Add(assetPath2.string());
        m_playlistManager.Add(assetPath3.string());

        std::string testFile = m_playlistManager.GetCurrentTrack();

        bool loadResult = m_tagManager.Load(testFile);
        
        if (loadResult) {
            std::wstring title = m_tagManager.GetTitle();
            std::wstring artist = m_tagManager.GetArtist();
            
            // フォールバック処理: 曲名が空ならファイル名を抽出、アーティストが空なら「---」
            if (title.empty()) {
                title = std::filesystem::path(testFile).filename().wstring();
            }
            if (artist.empty()) {
                artist = L"---";
            }
            
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

        m_audioPlayer.Play(testFile);
        PrefetchNextTrack();
    }

    return true;
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
