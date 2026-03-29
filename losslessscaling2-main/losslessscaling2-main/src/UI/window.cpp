#include "Window.h"

LRESULT CALLBACK WinProc(HWND hWnd, UINT msg, WPARAM wp, LPARAM lp) {
    if (msg == WM_CLOSE) {
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProcW(hWnd, msg, wp, lp);
}

bool AppWindow::Create(int width, int height, HINSTANCE hInst) {
    const wchar_t* wszClass = L"PicaScaling_UI";

    WNDCLASSW wc = {};
    wc.lpfnWndProc = WinProc;
    wc.hInstance = hInst;
    wc.lpszClassName = wszClass;
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);

    RegisterClassW(&wc);

    m_hWnd = CreateWindowExW(
        WS_EX_TOPMOST | WS_EX_LAYERED | WS_EX_TRANSPARENT, 
        wszClass, L"Pica Scaling Overlay", 
        WS_POPUP, 
        0, 0, width, height, 
        nullptr, nullptr, hInst, nullptr
    );

    if (!m_hWnd) return false;

    SetLayeredWindowAttributes(m_hWnd, 0, 255, LWA_ALPHA);

    ShowWindow(m_hWnd, SW_SHOW);
    return true;
}

void AppWindow::ProcessMessages() {
    MSG msg;
    while (PeekMessageW(&msg, nullptr, 0, 0, PM_REMOVE)) {
        if (msg.message == WM_QUIT) m_isOpen = false;
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }
}