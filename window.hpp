#define WIN32_LEAN_AND_MEAN

#pragma once
#include <windows.h>
#include <iostream>


//TODO 

struct Window
{
public:
    HWND makeWindow(const char* streamName, uint16_t width, uint16_t height);
    static LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
    bool processMessage();
private:
    HWND m_handle = NULL; 
    HINSTANCE m_hInstance = NULL; 
    std::string m_window_title = "";
    std::wstring m_window_title_wide = L""; 
    std::string m_window_class = "";
    std::wstring m_window_class_wide = L""; 
    int m_width = 0;
    int m_height = 0;
};