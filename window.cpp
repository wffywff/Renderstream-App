#include "include/window.hpp"
#include "include/widget.hpp"

Window::Window(const char* streamName, size_t width, size_t height)
{
    HWND hWnd;
    WNDCLASSEX wc;
    ZeroMemory(&wc, sizeof(WNDCLASSEX));

    wc.cbSize = sizeof(WNDCLASSEX);
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = NULL;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.lpszClassName = L"RenderStreamDebug";

    m_window_class_wide = L"RenderStreamDebug";

    RegisterClassEx(&wc);

    RECT wr = { 0, 0, (LONG)(width / 2), (LONG)(height / 2) };
    AdjustWindowRect(&wr, WS_OVERLAPPEDWINDOW, FALSE);

    wchar_t wStreamName[4096];
    MultiByteToWideChar(CP_ACP, 0, streamName, -1, wStreamName, 4096);

    hWnd = CreateWindowEx(NULL,
        m_window_class_wide,
        wStreamName,
        WS_OVERLAPPEDWINDOW,
        0,
        0,
        wr.right - wr.left,
        wr.bottom - wr.top,
        NULL,
        NULL,
        NULL,
        NULL);

    ShowWindow(hWnd, 10);
    SetForegroundWindow(hWnd);
    SetFocus(hWnd);
    m_handle = hWnd;
}

extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK Window::WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    if (ImGui_ImplWin32_WndProcHandler(hWnd, message, wParam, lParam))
        return true;

    return DefWindowProc(hWnd, message, wParam, lParam);
}

bool Window::processMessage()
{
    MSG msg;
    ZeroMemory(&msg, sizeof(MSG));

    while (PeekMessage(&msg,
        m_handle,
        0,
        0,   
        PM_REMOVE))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    if (msg.message == WM_NULL)
    {
        if (!IsWindow(m_handle))
        {
            m_handle = NULL; 
            UnregisterClass(m_window_class_wide, NULL);
            return false;
        }
    }
    return true;
}
