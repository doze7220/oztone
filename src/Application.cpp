#include "Application.h"

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
        m_audioPlayer.Play("assets/test.mp3");
    }

    return true;
}

void Application::Run() {
    // メインループ
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
