#pragma once
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <string>
#include <fstream>
#include <d3d11.h>
#include <wrl/client.h>
#pragma comment (lib, "d3d11.lib")

#include "utility.hpp"
#include "graphics.hpp"
#include "widget.hpp"
#include "window.hpp"

//TODO: do I need a constructor or descrutor for the these classes..?
class RenderInstance
{
public:
    RenderInstance(Window& w, Graphics& g, StreamDescription &desc) :
        window(w), graphic(g), description(desc)
    {
        gui = Widget();
        timer = Timer();
    }
    RenderTarget render(const CameraResponseData&, int sceneNum);
    void fps();

    Widget gui;
    Timer timer;
    Window window;
    StreamDescription description;
    Graphics graphic;
    bool m_closedByUser = false;

};

