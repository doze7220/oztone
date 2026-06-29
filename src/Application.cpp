#include "Application.h"

Application::Application() {}

Application::~Application() {}

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

    // 今後、AudioPlayerの初期化をここに追加する
    return true;
}

void Application::Run() {
    // メインループ
    while (m_window.ProcessMessages()) {
        m_renderer.Render(m_window.IsHovered());
        Sleep(1); // CPU使用率を抑えるための仮のスリープ
    }
}
