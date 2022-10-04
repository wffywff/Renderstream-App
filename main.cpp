#include "engine.hpp"
#include "window.hpp"
#include "helper.hpp"

int main()
{
#ifdef _DEBUG
    MessageBoxA(NULL, "Attach Debugger Now", NULL, NULL);
#endif
    std::cout << "Hello World!\n";
    Engine engine;
    engine.graphics.makeDxDevice();
    engine.log.log("Starting RenderStream Tester app.\n");

    HMODULE hLib = engine.loadRenderStreamDll();
    if (!hLib)
    {
        engine.log.popMessageBox("Failed to load RenderStream DLL");
        return 1;
    };

#define LOAD_FN(FUNC_NAME) \
    decltype(FUNC_NAME)* FUNC_NAME = reinterpret_cast<decltype(FUNC_NAME)>(GetProcAddress(hLib, #FUNC_NAME)); \
    if (!FUNC_NAME) { \
        engine.log.popMessageBox("Failed to get function " #FUNC_NAME " from DLL"); \
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
        engine.log.popMessageBox("Failed to initialise RenderStream");
        return 3;
    }
    engine.graphics.makeDxDevice();
    if (rs_initialiseGpGpuWithDX11Device(engine.graphics.dev.Get()) != RS_ERROR_SUCCESS)
    {
        engine.log.popMessageBox("Failed to initalise GpGPU with DX11 Device");
        return 4;
    }

    FrameData frameData;
    uint32_t desSize = 0;
    std::vector<uint32_t> streamData;
    StreamDescriptions* descriptions = NULL;

    while (true)
    {
        RS_ERROR err = rs_awaitFrameData(5000, &frameData);
        if (err == RS_ERROR_STREAMS_CHANGED)
        {
            engine.timer.start();
            try
            {
                rs_getStreams(nullptr, &desSize); // returns RS_ERROR_BUFFER_OVERFLOW, and sets descriptionsSize to required size in bytes.
                streamData.resize(desSize);
                rs_getStreams(reinterpret_cast<StreamDescriptions*>(streamData.data()), &desSize); // returns RS_ERROR_SUCCESS
                descriptions = reinterpret_cast<StreamDescriptions*>(streamData.data());

                const size_t numStreams = descriptions ? descriptions->nStreams : 0;
                for (size_t i = 0; i < numStreams; i++)
                {
                    //find the streamdescription 
                    const StreamDescription& description = descriptions->streams[i];
                    engine.setup(description);
                    std::string streamName = static_cast<std::string>(description.channel) + description.name;
                    HWND window = engine.window.makeWindow(streamName.c_str(), description.width, description.height);
                    if (engine.graphics.initDx(window, description) != 0)
                    {
                        engine.log.popMessageBox("initating DirectX with stream description failed.");
                        return 5;
                    };
                }
            }
            catch (const std::exception& e)
            {
                engine.log.popMessageBox(e.what());
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
            engine.log.popMessageBox("Exiting due to quit request.");
            RS_ERROR err = rs_shutdown();
            if (err == RS_ERROR_SUCCESS)
                return 0;
            else
                return 99;
        }
        else if (err != RS_ERROR_SUCCESS)
        {
            engine.log.popMessageBox("rs_awaitFrameData returned error.");
            break;
        }

        while (engine.window.processMessage())
        {
            const size_t numStreams = descriptions ? descriptions->nStreams : 0;
            for (size_t i = 0; i < numStreams; i++)
            {
                const StreamDescription& description = descriptions->streams[i];

                CameraResponseData response;
                response.tTracked = frameData.tTracked;

                if (rs_getFrameCamera(description.handle, &response.camera) == RS_ERROR_SUCCESS)
                {
                    engine.renderFrame(description, response);
                    SenderFrameTypeData data;
                    data.dx11.resource = engine.renderstreamTarget[description.handle].texture.Get();
                    if (rs_sendFrame(description.handle, RS_FRAMETYPE_DX11_TEXTURE, data, &response) != RS_ERROR_SUCCESS)
                    {
                        engine.log.popMessageBox("Failed to send frame");
                        rs_shutdown();
                        return 7;
                    }
                }
            }
        }
    }
    if (rs_shutdown() != RS_ERROR_SUCCESS)
    {
        engine.log.popMessageBox("Failed to shutdown RenderStream");
        return 99;
    }
    return 0;
}
