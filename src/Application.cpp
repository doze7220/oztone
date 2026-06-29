#include "Application.h"

Application::Application() {}

Application::~Application() {}

bool Application::Initialize(HINSTANCE hInstance, int nCmdShow) {
    if (!m_window.Initialize(hInstance, nCmdShow)) {
        return false;
    }
    
    // 今後、RendererやAudioPlayerの初期化をここに追加する
    return true;
}

void Application::Run() {
    // メインループ
    while (m_window.ProcessMessages()) {
        // 今後、ここで毎フレームの描画処理などを呼び出す
        Sleep(1); // CPU使用率を抑えるための仮のスリープ
    }
}
