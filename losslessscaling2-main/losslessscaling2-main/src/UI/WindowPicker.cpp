#include "WindowPicker.h"

BOOL CALLBACK EnumWindowsProc(HWND hwnd, LPARAM lParam) {
    auto* windows = reinterpret_cast<std::vector<WindowInfo>*>(lParam);

    if (!IsWindowVisible(hwnd)) return TRUE;

    int length = GetWindowTextLengthW(hwnd);
    if (length == 0) return TRUE;

    wchar_t title[256];
    GetWindowTextW(hwnd, title, 256);
    std::wstring windowTitle(title);

    if (windowTitle == L"Program Manager" || windowTitle == L"Settings" || 
        windowTitle == L"Microsoft Text Input Application" || windowTitle == L"Calculadora") {
        return TRUE;
    }

    LONG style = GetWindowLong(hwnd, GWL_EXSTYLE);
    if (style & WS_EX_TOOLWINDOW) return TRUE; 

    windows->push_back({ hwnd, windowTitle });
    return TRUE;
}

std::vector<WindowInfo> WindowPicker::FindWindows() {
    std::vector<WindowInfo> windows;
    EnumWindows(EnumWindowsProc, reinterpret_cast<LPARAM>(&windows));
    return windows;
}