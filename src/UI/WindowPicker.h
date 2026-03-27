#pragma once
#include <windows.h>
#include <vector>
#include <string>

struct WindowInfo {
    HWND hwnd;
    std::wstring title;
};

class WindowPicker {
public:
    static std::vector<WindowInfo> FindWindows();
};