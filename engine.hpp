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

struct RenderTarget
{
    Microsoft::WRL::ComPtr<ID3D11Texture2D> texture;
    Microsoft::WRL::ComPtr<ID3D11RenderTargetView> view;
};

//TODO: do I need a constructor or descrutor for the these classes..?
class Engine
{
public:
    ErrorLogger log;
    Graphics graphics;
    Widget guiWindow;
    Timer timer;
    Window window;
    std::unordered_map<StreamHandle, RenderTarget> renderstreamTarget; 
    HMODULE loadRenderStreamDll();
    void setup(const StreamDescription& streamDesc);
    int renderFrame(const StreamDescription& description, const CameraResponseData& response);
};

