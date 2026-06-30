#include <windows.h>
#include <crtdbg.h>
#include <shellapi.h>
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

    // 多重起動防止ミューテックスの作成
    HANDLE hMutex = CreateMutexW(nullptr, FALSE, L"OZtone_Mutex");
    bool alreadyExists = (GetLastError() == ERROR_ALREADY_EXISTS);

    int argc = 0;
    LPWSTR* argv = CommandLineToArgvW(GetCommandLineW(), &argc);

    if (alreadyExists) {
        if (argc > 1) {
            HWND hExistingWnd = FindWindowW(L"OZtoneWindowClass", nullptr);
            if (hExistingWnd) {
                for (int i = 1; i < argc; ++i) {
                    std::wstring path = argv[i];
                    COPYDATASTRUCT cds;
                    cds.dwData = 0;
                    cds.cbData = static_cast<DWORD>((path.length() + 1) * sizeof(wchar_t));
                    cds.lpData = const_cast<wchar_t*>(path.c_str());
                    SendMessageW(hExistingWnd, WM_COPYDATA, reinterpret_cast<WPARAM>(nullptr), reinterpret_cast<LPARAM>(&cds));
                }
            }
        }
        if (argv) LocalFree(argv);
        if (hMutex) CloseHandle(hMutex);
        return 0;
    }

    // OLEの初期化 (ドラッグ＆ドロップおよびWIC等に必要)
    HRESULT hr = OleInitialize(nullptr);
    if (FAILED(hr)) {
        if (argv) LocalFree(argv);
        if (hMutex) CloseHandle(hMutex);
        return -1;
    }

    // メモリリーク検出（デバッグ時）
#if defined(_DEBUG)
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

    Application app;

    if (!app.Initialize(hInstance, nCmdShow)) {
        MessageBoxW(nullptr, L"Application initialization failed.", L"Error", MB_OK | MB_ICONERROR);
        if (argv) LocalFree(argv);
        if (hMutex) CloseHandle(hMutex);
        OleUninitialize();
        return -1;
    }

    if (argc > 1) {
        app.ProcessCommandLineArgs(argc, argv);
    }

    if (argv) LocalFree(argv);

    app.Run();

    OleUninitialize();
    if (hMutex) CloseHandle(hMutex);
    return 0;
}
