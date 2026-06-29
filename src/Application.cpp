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
        m_renderer.Render(m_window.IsHovered());
        Sleep(1); // CPU使用率を抑えるための仮のスリープ
    }
}
