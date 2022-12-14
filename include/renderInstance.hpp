#pragma once
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <string>
#include <fstream>

#include "utility.hpp"
#include "graphics.hpp"
#include "widget.hpp"
#include "window.hpp"


class RenderInstance
{
public:
    RenderInstance() = delete;
    RenderInstance(const StreamDescription& desc) :
        description(desc),
        window(Window(description.name, description.width, description.height)),
        graphic(Graphics(dx11device(), GraphicsInfo(description.width, description.height, window.getHandle(), description.handle, description.format)))
    {
    }

    void sendFrame(CameraResponseData& response, int sceneNum);
    void present();

private:
    RenderTarget graphicRender(CameraResponseData& response, int sceneNum);
    void fps();

    StreamDescription description;
    Window window;
    Graphics graphic;
    Widget gui;
    Timer timer;
};

