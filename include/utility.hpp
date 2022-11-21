#pragma once
#define WIN32_LEAN_AND_MEAN
#include <string>
#include <Windows.h>
#include <fstream>
#include <libloaderapi.h>
#include <filesystem>
#include <iostream>
#include <Shlwapi.h>
#include "include/d3renderstream.h"
#include "include/dx11.hpp"

#define DECL_FN(FUNC_NAME) decltype(rs_ ## FUNC_NAME)* m_ ## FUNC_NAME = nullptr

class RenderStream
{
public:
    RenderStream(const RenderStream& obj) = delete;
    static RenderStream* getInstance()
    {
        if (instancePtr == nullptr)
        {
            instancePtr = new RenderStream();
            return instancePtr;
        }
        else
        {
            return instancePtr;
        }
    }
    bool loadAPI();
    bool initialise();
    StreamDescriptions* getStreamsDescriptions();
    RS_ERROR awaitFrameData(int timeoutMs, FrameData* data);
    bool getFrameCamera(StreamHandle streamHandle, CameraData* outCameraData);
    bool sendFrame(StreamHandle streamHandle, SenderFrameType frameType, SenderFrameTypeData data, const CameraResponseData* sendData);
    bool shutdown();
    dx11device getDxDevice() { return m_dx11; };

private:
    static RenderStream* instancePtr;
    RenderStream() {};
    dx11device m_dx11;
    std::vector<char> descriptionData;

    DECL_FN(initialise);
    DECL_FN(initialiseGpGpuWithDX11Device);
    DECL_FN(getStreams);
    DECL_FN(awaitFrameData);
    DECL_FN(getFrameCamera);
    DECL_FN(sendFrame);
    DECL_FN(shutdown);
};

class ErrorLogger
{
public:
    static void log(const std::string& info);
    static void popMessageBox(const std::string& popupinfo);
};

class Timer
{
public:
    void start();
    std::chrono::duration<double, std::milli> getDuration();
    
private:
    std::chrono::high_resolution_clock::time_point m_start;
};