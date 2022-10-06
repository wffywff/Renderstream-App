#pragma once 

#include "include/imgui/imgui.h"
#include "include/imgui/imgui_impl_dx11.h"
#include "include/imgui/imgui_impl_win32.h"
#include "DirectXMath.h"

class Widget
{
public: 
    void displayFPS(const int fps);

private:
    int m_avgFrameRate = 0;
};