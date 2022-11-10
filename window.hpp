#define WIN32_LEAN_AND_MEAN

#pragma once
#include <windows.h>
#include <iostream>


//TODO 

struct Window
{
public:
    Window(const char* streamName, uint16_t width, uint16_t height);
    HWND getHandle() { return m_handle; };
    static LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
    bool processMessage();
private:
    HWND m_handle = NULL; 
    std::wstring m_window_class_wide = L""; 
};