#include "Window.h"
#include "ConfigManager.h"
#include <windowsx.h>

Window::Window() : m_hwnd(nullptr), m_hInstance(nullptr), m_config(nullptr), m_isHovered(false), m_isTrackingMouse(false) {}

Window::~Window() {
    if (m_hwnd) {
        DestroyWindow(m_hwnd);
    }
}

bool Window::Initialize(HINSTANCE hInstance, int nCmdShow, ConfigManager& config) {
    m_hInstance = hInstance;
    m_config = &config;

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
        dwExStyle |= WS_EX_LAYERED;
    }

    if (!config.GetShowTaskbar()) {
        dwExStyle |= WS_EX_TOOLWINDOW;
    } else {
        dwExStyle |= WS_EX_APPWINDOW;
    }

    // ConfigManagerからサイズ・座標を取得しDPIスケーリング
    int x = config.GetWindowX();
    int y = config.GetWindowY();
    int width = config.GetWindowWidth();
    int height = config.GetWindowHeight();

    UINT dpi = GetDpiForSystem();
    int scaledWidth = MulDiv(width, dpi, 96);
    int scaledHeight = MulDiv(height, dpi, 96);

    int scaledX = (x == CW_USEDEFAULT) ? CW_USEDEFAULT : MulDiv(x, dpi, 96);
    int scaledY = (y == CW_USEDEFAULT) ? CW_USEDEFAULT : MulDiv(y, dpi, 96);

    RECT rect = { 0, 0, scaledWidth, scaledHeight };
    AdjustWindowRectExForDpi(&rect, dwStyle, FALSE, dwExStyle, dpi);

    m_hwnd = CreateWindowExW(
        dwExStyle,
        m_className,
        L"OZtone",
        dwStyle,
        scaledX, scaledY,
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

bool Window::IsInLogoRegion(int x, int y) const {
    if (!m_config) return false;
    return (x >= m_config->GetLogoX() && x <= m_config->GetLogoX() + m_config->GetLogoWidth() &&
            y >= m_config->GetLogoY() && y <= m_config->GetLogoY() + m_config->GetLogoHeight());
}

LRESULT Window::WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
        case WM_MOUSEMOVE: {
            if (m_config) {
                int xPos = GET_X_LPARAM(lParam);
                int yPos = GET_Y_LPARAM(lParam);

                m_isHovered = IsInLogoRegion(xPos, yPos);

                if (!m_isTrackingMouse) {
                    TRACKMOUSEEVENT tme = {};
                    tme.cbSize = sizeof(TRACKMOUSEEVENT);
                    tme.dwFlags = TME_LEAVE;
                    tme.hwndTrack = hwnd;
                    TrackMouseEvent(&tme);
                    m_isTrackingMouse = true;
                }
            }
            return 0;
        }
        case WM_MOUSELEAVE: {
            m_isHovered = false;
            m_isTrackingMouse = false;
            return 0;
        }
        case WM_LBUTTONDOWN: {
            int xPos = GET_X_LPARAM(lParam);
            int yPos = GET_Y_LPARAM(lParam);
            if (IsInLogoRegion(xPos, yPos)) {
                ReleaseCapture();
                SendMessage(hwnd, WM_NCLBUTTONDOWN, HTCAPTION, 0);
            }
            return 0;
        }
        case WM_RBUTTONDOWN: {
            int xPos = GET_X_LPARAM(lParam);
            int yPos = GET_Y_LPARAM(lParam);
            if (IsInLogoRegion(xPos, yPos)) {
                PostMessage(hwnd, WM_CLOSE, 0, 0);
            }
            return 0;
        }
        case WM_DESTROY: {
            if (m_config) {
                RECT wndRect, clientRect;
                if (GetWindowRect(hwnd, &wndRect) && GetClientRect(hwnd, &clientRect)) {
                    UINT dpi = GetDpiForWindow(hwnd);
                    int logicalX = MulDiv(wndRect.left, 96, dpi);
                    int logicalY = MulDiv(wndRect.top, 96, dpi);
                    int logicalWidth = MulDiv(clientRect.right - clientRect.left, 96, dpi);
                    int logicalHeight = MulDiv(clientRect.bottom - clientRect.top, 96, dpi);
                    m_config->SaveWindowPosition(logicalX, logicalY, logicalWidth, logicalHeight);
                }
            }
            PostQuitMessage(0);
            return 0;
        }
    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}
