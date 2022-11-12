#include "engine.hpp"
#include "window.hpp"

HMODULE loadRenderStreamDll()
{
    HKEY hKey;
    if (FAILED(RegOpenKeyEx(HKEY_CURRENT_USER, TEXT("Software\\d3 Technologies\\d3 Production Suite"), 0, KEY_READ, &hKey)))
    {
        ErrorLogger::log("Failed to open 'Software\\d3 Technologies\\d3 Production Suite' registry key");
        return nullptr;
    }

    TCHAR buffer[512];
    DWORD bufferSize = sizeof(buffer);
    if (FAILED(RegQueryValueEx(hKey, TEXT("exe path"), 0, nullptr, reinterpret_cast<LPBYTE>(buffer), &bufferSize)))
    {
        ErrorLogger::log("Failed to query value of 'exe path'");
        return nullptr;
    }

    if (!PathRemoveFileSpec(buffer))
    {
        ErrorLogger::log("Failed to remove file spec from path");
        return nullptr;
    }

    if (!PathAppend(buffer, TEXT("\\d3renderstream.dll")))
    {
        ErrorLogger::log("Failed to append filename to path");
        return nullptr;
    }

    //TCHAR buffer[512];

    //PathAppend(buffer, TEXT("C:\\Program Files\\d3 Production Suite\\build\\msvc\\d3renderstream.dll"));

    HMODULE hLib = ::LoadLibraryEx(buffer, NULL, LOAD_LIBRARY_SEARCH_DLL_LOAD_DIR | LOAD_LIBRARY_SEARCH_APPLICATION_DIR | LOAD_LIBRARY_SEARCH_SYSTEM32 | LOAD_LIBRARY_SEARCH_USER_DIRS);
    if (!hLib)
    {
        ErrorLogger::log("Failed to load dll");
        return nullptr;
    }
    return hLib;
}

int main()
{
#ifdef _DEBUG
    MessageBoxA(NULL, "Attach Debugger Now", NULL, NULL);
#endif
    ErrorLogger::log("Starting RenderStream Tester app.\n");

    HMODULE hLib = loadRenderStreamDll();
    if (!hLib)
    {
        ErrorLogger::popMessageBox("Failed to load RenderStream DLL");
        return 1;
    };

    // https://davekilian.com/decltype-funcptrs.html
    // decltype evaluates to the type of expression
    // evaluate to the function pointer signature for all rs api functions
    //void myFunction(int arg);
    //typedef decltype(&myFunction)funcptr;
#define LOAD_FN(FUNC_NAME) \
    decltype(FUNC_NAME)* FUNC_NAME = reinterpret_cast<decltype(FUNC_NAME)>(GetProcAddress(hLib, #FUNC_NAME)); \
    if (!FUNC_NAME) { \
        ErrorLogger::popMessageBox("Failed to get function " #FUNC_NAME " from DLL"); \
        return 2; \
    }

    LOAD_FN(rs_initialise);
    LOAD_FN(rs_getStreams);
    LOAD_FN(rs_awaitFrameData);
    LOAD_FN(rs_getFrameCamera);
    LOAD_FN(rs_sendFrame);
    LOAD_FN(rs_shutdown);
    LOAD_FN(rs_initialiseGpGpuWithDX11Device);

    if (rs_initialise(RENDER_STREAM_VERSION_MAJOR, RENDER_STREAM_VERSION_MINOR) != RS_ERROR_SUCCESS)
    {
        ErrorLogger::popMessageBox("Failed to initialise RenderStream");
        return 3;
    }
    dx11device dx11;
    if (dx11.dev.Get() == nullptr)
    {
        ErrorLogger::popMessageBox("Failed to create DxDevice");
        return 4;
    }
    if (rs_initialiseGpGpuWithDX11Device(dx11.dev.Get()) != RS_ERROR_SUCCESS)
    {
        ErrorLogger::popMessageBox("Failed to initalise GpGPU with DX11 Device");
        return 5;
    }

    FrameData frameData;
    std::vector<RenderInstance> renderInstances;

    while (true)
    {
        RS_ERROR err = rs_awaitFrameData(5000, &frameData);
        if (err == RS_ERROR_STREAMS_CHANGED)
        {
            try
            {
                uint32_t desSize = 0;
                std::vector<char> data;
                StreamDescriptions* descriptions = nullptr;

                // first call would return RS_ERROR_BUFFER_OVERFLOW but populate desSize -- required size in bytes.
                rs_getStreams(nullptr, &desSize);
                data.resize(desSize);
                // this time should be able to read bytes into the buffer of streamData, then reinterpret the memory as the struct StreamDescription
                rs_getStreams(reinterpret_cast<StreamDescriptions*>(data.data()), &desSize);
                descriptions = reinterpret_cast<StreamDescriptions*>(data.data());

                const size_t numStreams = descriptions ? descriptions->nStreams : 0;
                for (size_t i = 0; i < numStreams; i++)
                {
                    StreamDescription& description = descriptions->streams[i];
                    Window w(description.name, description.width, description.height);
                    Graphics g(dx11, GraphicsInfo(description.width, description.height, w.getHandle(), description.handle, description.format));
                    renderInstances.emplace_back(w, g, description);
                }
            }
            catch (const std::exception& e)
            {
                ErrorLogger::popMessageBox(e.what());
                rs_shutdown();
                return 4;
            }
            continue;
        }
        else if (err == RS_ERROR_TIMEOUT)
        {
            continue;
        }
        else if (err == RS_ERROR_QUIT)
        {
            ErrorLogger::log("Exiting due to quit requested from rendersteam.");
            RS_ERROR err = rs_shutdown();
            if (err == RS_ERROR_SUCCESS)
                return 0;
            else
                return 99;
        }
        else if (err != RS_ERROR_SUCCESS)
        {
            ErrorLogger::popMessageBox("rs_awaitFrameData returned error.");
            break;
        }

        std::vector<RenderInstance>::iterator itr;
        for (itr = renderInstances.begin(); itr < renderInstances.end();itr++)
        {
            if (!itr->window.processMessage())
            {
                int instanceNumber = itr - renderInstances.begin();
                itr->m_closedByUser = true;
                std::stringstream info;
                info << "Exiting render instance #" << instanceNumber << " due to quit request from window interaction.\n";
                ErrorLogger::log(info.str());
            }
        }

        for (size_t i = 0; i< renderInstances.size(); i++)
        {
            if (renderInstances[i].m_closedByUser)
            {
                std::stringstream info;
                info << "Skipping sending instance #" << i << " because user has closed the window.\n";
                ErrorLogger::log(info.str());
                continue;
            }
            renderInstances[i].timer.start();
            CameraResponseData response;
            response.tTracked = frameData.tTracked;
            if (rs_getFrameCamera(renderInstances[i].description.handle, &response.camera) == RS_ERROR_SUCCESS)
            {
                renderInstances[i].render(response);
                SenderFrameTypeData data;
                data.dx11.resource = renderInstances[i].graphic.getRsTexture().texture.Get();
                if (rs_sendFrame(renderInstances[i].description.handle, RS_FRAMETYPE_DX11_TEXTURE, data, &response) != RS_ERROR_SUCCESS)
                {
                    ErrorLogger::popMessageBox("Failed to send frame");
                    rs_shutdown();
                    return 7;
                }
                static int fps_count = 0;
                fps_count++;
                auto milliseconds_elapsed = renderInstances[i].timer.getDuration().count();
                if (milliseconds_elapsed > 1000)
                {
                    renderInstances[i].gui.displayFPS(fps_count);
                    fps_count = 0;
                    renderInstances[i].timer.start();
                }
                renderInstances[i].graphic.getSwapChain()->Present(0, 0);
            }
        }
    }

    if (rs_shutdown() != RS_ERROR_SUCCESS)
    {
        ErrorLogger::popMessageBox("Failed to shutdown RenderStream");
        return 99;
    }
    return 0;
}
