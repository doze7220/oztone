#include "Window.h"
#include "ConfigManager.h"

Window::Window() : m_hwnd(nullptr), m_hInstance(nullptr) {}

Window::~Window() {
    if (m_hwnd) {
        DestroyWindow(m_hwnd);
    }
}

bool Window::Initialize(HINSTANCE hInstance, int nCmdShow, const ConfigManager& config) {
    m_hInstance = hInstance;

    WNDCLASSEXW wc = {};
    wc.cbSize = sizeof(WNDCLASSEXW);
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = WindowProcStatic;
    wc.hInstance = m_hInstance;
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.lpszClassName = m_className;

    if (!RegisterClassExW(&wc)) {
        return false;
    }

    // 設定からウィンドウスタイルを決定
    DWORD dwStyle = 0;
    DWORD dwExStyle = 0;

    if (config.GetShowTitleBar() && config.GetShowWindowFrame()) {
        dwStyle = WS_OVERLAPPEDWINDOW;
    } else if (config.GetShowTitleBar()) {
        dwStyle = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX;
    } else if (config.GetShowWindowFrame()) {
        dwStyle = WS_POPUP | WS_THICKFRAME;
    } else {
        dwStyle = WS_POPUP; // 完全枠なし
    }

    if (!config.GetShowTaskbar()) {
        dwExStyle |= WS_EX_TOOLWINDOW;
    } else {
        dwExStyle |= WS_EX_APPWINDOW;
    }

    // サイズは1024x512
    RECT rect = { 0, 0, 1024, 512 };
    AdjustWindowRectEx(&rect, dwStyle, FALSE, dwExStyle);

    m_hwnd = CreateWindowExW(
        dwExStyle,
        m_className,
        L"OZtone",
        dwStyle,
        CW_USEDEFAULT, CW_USEDEFAULT,
        rect.right - rect.left, rect.bottom - rect.top,
        nullptr, nullptr, m_hInstance, this
    );

    if (!m_hwnd) {
        return false;
    }

    ShowWindow(m_hwnd, nCmdShow);
    UpdateWindow(m_hwnd);

    return true;
}

bool Window::ProcessMessages() {
    MSG msg = {};
    while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
        if (msg.message == WM_QUIT) {
            return false;
        }
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return true;
}

LRESULT CALLBACK Window::WindowProcStatic(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    Window* pThis = nullptr;

    if (uMsg == WM_NCCREATE) {
        CREATESTRUCT* pCreate = reinterpret_cast<CREATESTRUCT*>(lParam);
        pThis = reinterpret_cast<Window*>(pCreate->lpCreateParams);
        SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pThis));
    } else {
        pThis = reinterpret_cast<Window*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
    }

    if (pThis) {
        return pThis->WindowProc(hwnd, uMsg, wParam, lParam);
    }

    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

LRESULT Window::WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}
