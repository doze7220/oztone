#include "Window.h"
#include "ConfigManager.h"
#include "resource.h"
#include <filesystem>
#include <shellapi.h>
#include <windowsx.h>

HWND Window::s_hwnd = nullptr;


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



LRESULT CALLBACK Window::WindowProcStatic(HWND hwnd, UINT uMsg, WPARAM wParam,
                                          LPARAM lParam) {
  Window *pThis = nullptr;

  if (uMsg == WM_NCCREATE) {
    CREATESTRUCT *pCreate = reinterpret_cast<CREATESTRUCT *>(lParam);
    pThis = reinterpret_cast<Window *>(pCreate->lpCreateParams);
    SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pThis));
  } else {
    pThis = reinterpret_cast<Window *>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
  }

  if (pThis) {
    return pThis->WindowProc(hwnd, uMsg, wParam, lParam);
  }

  return DefWindowProcW(hwnd, uMsg, wParam, lParam);
}

LRESULT Window::WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
  switch (uMsg) {
  case WM_HOTKEY:
    HandleHotkey(wParam, lParam);
    return 0;
  case WM_GETMINMAXINFO:
    HandleGetMinMaxInfo(hwnd, lParam);
    return 0;
  case WM_SIZE:
    HandleSize(wParam, lParam);
    return 0;
  case WM_NCHITTEST:
    return HandleNcHitTest(hwnd, uMsg, wParam, lParam);
  case WM_WINDOWPOSCHANGING:
    HandleWindowPosChanging(lParam);
    break;
  case WM_MOUSEACTIVATE: {
    LRESULT res = 0;
    if (HandleMouseActivate(res)) return res;
    break;
  }
  case WM_MOUSEMOVE:
    HandleMouseMove(hwnd, wParam, lParam);
    return 0;
  case WM_MOUSELEAVE:
    HandleMouseLeave();
    return 0;
  case WM_LBUTTONDOWN:
    HandleLButtonDown(hwnd, wParam, lParam);
    return 0;
  case WM_LBUTTONUP:
    HandleLButtonUp(wParam, lParam);
    break;
  case WM_RBUTTONDOWN:
    if (HandleRButtonDown(hwnd, wParam, lParam)) return 0;
    break;
  case WM_RBUTTONUP:
    if (HandleRButtonUp(wParam, lParam)) return 0;
    break;
  case WM_LBUTTONDBLCLK:
    HandleLButtonDblClk(wParam, lParam);
    return 0;
  case WM_MOUSEWHEEL:
    if (HandleMouseWheel(hwnd, wParam, lParam)) return 0;
    break;
  case WM_KEYDOWN:
    if (HandleKeyDown(wParam, lParam)) return 0;
    break;
  case WM_TRAYICON:
    HandleTrayIcon(hwnd, lParam);
    return 0;
  case WM_COMMAND:
    HandleCommand(hwnd, wParam);
    return 0;
  case WM_APP_MEDIAKEY:
    HandleAppMediaKey(wParam);
    return 0;
  case WM_COPYDATA:
    HandleCopyData(lParam);
    return 1;
  case WM_DESTROY:
    HandleDestroy(hwnd);
    return 0;
  }
  return DefWindowProcW(hwnd, uMsg, wParam, lParam);
}

