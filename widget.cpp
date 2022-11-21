#include "include/widget.hpp"
#include <string>

void Widget::displayFPS(const int fps)
{
    ImGui_ImplDX11_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();

    if (fps > 0)
    {
        m_avgFrameRate = fps;
    }

    ImGui::Begin("Debug Info");
    std::string fpsString = "FPS: " + std::to_string(m_avgFrameRate);
    ImGui::Text(fpsString.c_str());
    ImGui::End();

    ImGui::Render();
    ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
}
