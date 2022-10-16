#pragma once
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <string>
#include <fstream>
#include <d3d11.h>
#include <wrl/client.h>
#pragma comment (lib, "d3d11.lib")

#include "helper.hpp"
#include "graphics.hpp"
#include "widget.hpp"
#include "window.hpp"

//TODO: do I need a constructor or descrutor for the these classes..?
class Engine
{
public:
    ErrorLogger log;
    dx11device dx11;
    Widget guiWindow;
    Timer timer;
    Window window;

    HMODULE loadRenderStreamDll();
    void setup(const StreamDescription& streamDesc);
    DirectX::XMMATRIX calculateFrame(const StreamDescription& description, const CameraResponseData& response);
};

