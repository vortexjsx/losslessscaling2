#pragma once
#include <windows.h>

class AppWindow {
public:
    bool Create(int width, int height, HINSTANCE hInst);
    void ProcessMessages();
    void SetTargetHWND(HWND target);
    
    bool IsOpen() const { return m_isOpen; }
    HWND GetHWND() const { return m_hWnd; }

private:
    HWND m_hWnd;
    bool m_isOpen = true;
};