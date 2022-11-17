#pragma once
#include "include/utility.hpp"


bool RenderStream::loadAPI()
{
    HKEY hKey;
    if (FAILED(RegOpenKeyEx(HKEY_CURRENT_USER, TEXT("Software\\d3 Technologies\\d3 Production Suite"), 0, KEY_READ, &hKey)))
    {
        ErrorLogger::log("Failed to open 'Software\\d3 Technologies\\d3 Production Suite' registry key");
        return false;
    }

    TCHAR buffer[512];
    DWORD bufferSize = sizeof(buffer);
    if (FAILED(RegQueryValueEx(hKey, TEXT("exe path"), 0, nullptr, reinterpret_cast<LPBYTE>(buffer), &bufferSize)))
    {
        ErrorLogger::log("Failed to query value of 'exe path'");
        return false;
    }

    if (!PathRemoveFileSpec(buffer))
    {
        ErrorLogger::log("Failed to remove file spec from path");
        return false;
    }

    if (!PathAppend(buffer, TEXT("\\d3renderstream.dll")))
    {
        ErrorLogger::log("Failed to append filename to path");
        return false;
    }

    HMODULE hLib = ::LoadLibraryEx(buffer, NULL, LOAD_LIBRARY_SEARCH_DLL_LOAD_DIR | LOAD_LIBRARY_SEARCH_APPLICATION_DIR | LOAD_LIBRARY_SEARCH_SYSTEM32 | LOAD_LIBRARY_SEARCH_USER_DIRS);
    if (!hLib)
    {
        ErrorLogger::popMessageBox("Failed to load dll");
        return false;
    }

#define LOAD_FN(FUNC_NAME) \
    m_ ## FUNC_NAME = reinterpret_cast<decltype(rs_ ## FUNC_NAME) *>(GetProcAddress(hLib, "rs_" #FUNC_NAME)); \
    if (!m_ ## FUNC_NAME) { \
        throw std::runtime_error("Failed to get function " #FUNC_NAME " from DLL"); \
    }

    LOAD_FN(initialise);
    LOAD_FN(getStreams);
    LOAD_FN(awaitFrameData);
    LOAD_FN(getFrameCamera);
    LOAD_FN(sendFrame);
    LOAD_FN(shutdown);
    LOAD_FN(initialiseGpGpuWithDX11Device);
    return true;
}

bool RenderStream::initialise()
{
    if (m_initialise(RENDER_STREAM_VERSION_MAJOR, RENDER_STREAM_VERSION_MINOR) != RS_ERROR_SUCCESS)
    {
        ErrorLogger::popMessageBox("Failed to initialise RenderStream");
        return false;
    }
    if (m_dx11.dev.Get() == nullptr)
    {
        ErrorLogger::popMessageBox("Failed to create DxDevice");
        return false;
    }
    if (m_initialiseGpGpuWithDX11Device(m_dx11.dev.Get()) != RS_ERROR_SUCCESS)
    {
        ErrorLogger::popMessageBox("Failed to initalise GpGPU with DX11 Device");
        return false;
    }
    return true;
}

StreamDescriptions* RenderStream::getStreamsDescriptions()
{
    uint32_t desSize = 0;
    // first call would return RS_ERROR_BUFFER_OVERFLOW but populate desSize -- required size in bytes.
    m_getStreams(nullptr, &desSize);
    descriptionData.resize(desSize);
    // this time should be able to read bytes into the buffer of streamData, then reinterpret the memory as the struct StreamDescription
    m_getStreams(reinterpret_cast<StreamDescriptions*>(descriptionData.data()), &desSize);
    return reinterpret_cast<StreamDescriptions*>(descriptionData.data());
}

RS_ERROR RenderStream::awaitFrameData(int timeoutMs, FrameData* data)
{
    return m_awaitFrameData(timeoutMs, data);
}

bool RenderStream::getFrameCamera(StreamHandle streamHandle, CameraData* outCameraData)
{
    if (m_getFrameCamera(streamHandle, outCameraData) != RS_ERROR_SUCCESS)
    {
        ErrorLogger::popMessageBox("Failed to getFrameCamera");
        return false;
    }
    return true;
}

bool RenderStream::sendFrame(StreamHandle streamHandle, SenderFrameType frameType, SenderFrameTypeData data, const CameraResponseData* sendData)
{
    if (m_sendFrame(streamHandle, frameType, data, sendData) != RS_ERROR_SUCCESS)
    {
        ErrorLogger::popMessageBox("Failed to sendFrame");
        return false;
    }
    return true;
}

bool RenderStream::shutdown()
{
    if (m_shutdown() != RS_ERROR_SUCCESS)
    {
        ErrorLogger::popMessageBox("Failed to shutdown");
        return false;
    }
    return true;
}

void ErrorLogger::log(const std::string& info)
{
    std::fstream fs;
    wchar_t path[MAX_PATH];
    GetModuleFileName(NULL, path, MAX_PATH);
    PathRemoveFileSpec(path);
    PathAppend(path, L"app_log.txt");
    fs.open(path, std::ios::out | std::ios::app);

    time_t now = time(NULL);
    char str[26] = {};
    ctime_s(str, 26, &now);

    fs << str << info <<"\n";
    fs.flush();
    fs.close();
}

void ErrorLogger::popMessageBox(const std::string& popupInfo)
{
    log(popupInfo);
    std::wstring wstr(popupInfo.begin(), popupInfo.end());
    MessageBox(NULL, wstr.c_str(), NULL, MB_ABORTRETRYIGNORE);
}

void Timer::start()
{
    m_start = std::chrono::high_resolution_clock::now();
}

std::chrono::duration<double, std::milli> Timer::getDuration()
{
    auto now = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> milliDuration = now - m_start;
    return milliDuration;
}
