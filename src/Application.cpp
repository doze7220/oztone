#include "Application.h"
#include <filesystem>

Application::Application() {}

Application::~Application() {
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
        std::filesystem::path assetPath = projectRoot / L"assets" / L"test.mp3";
        
        m_playlistManager.Add(assetPath.string());
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
    }

    return true;
}

void Application::Run() {
    while (m_window.ProcessMessages()) {
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
