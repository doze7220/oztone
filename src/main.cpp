#include <windows.h>
#include <crtdbg.h>
#include "Application.h"

/**
 * @brief Win32アプリケーションのエントリポイント
 */
int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    // High DPI対応 (Per-Monitor V2) を有効化
    SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);

    // OLEの初期化 (ドラッグ＆ドロップおよびWIC等に必要)
    HRESULT hr = OleInitialize(nullptr);
    if (FAILED(hr)) {
        return -1;
    }

    // メモリリーク検出（デバッグ時）
#if defined(_DEBUG)
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

    Application app;

    if (!app.Initialize(hInstance, nCmdShow)) {
        MessageBoxW(nullptr, L"Application initialization failed.", L"Error", MB_OK | MB_ICONERROR);
        return -1;
    }

    app.Run();

    OleUninitialize();
    return 0;
}
