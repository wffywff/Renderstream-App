#define WIN32_LEAN_AND_MEAN

#pragma once
#include <windows.h>

class Window
{
public:
    Window() = delete;
    Window(const char* streamName, size_t width, size_t height);
    HWND getHandle() { return m_handle; };
    bool processMessage();

private:
    static LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
    HWND m_handle = NULL; 
    LPCWSTR m_window_class_wide;
};