#include "engine.hpp"
#include "window.hpp"

int main()
{
#ifdef _DEBUG
    MessageBoxA(NULL, "Attach Debugger Now", NULL, NULL);
#endif

    Engine engine;
    engine.log.log("Starting RenderStream Tester app.\n");

    HMODULE hLib = engine.loadRenderStreamDll();
    if (!hLib)
    {
        engine.log.popMessageBox("Failed to load RenderStream DLL");
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
    if (engine.dx11.dev.Get() == nullptr)
    {
        engine.log.popMessageBox("Failed to create DxDevice");
        return 4;
    }
    if (rs_initialiseGpGpuWithDX11Device(engine.dx11.dev.Get()) != RS_ERROR_SUCCESS)
    {
        engine.log.popMessageBox("Failed to initalise GpGPU with DX11 Device");
        return 5;
    }

    FrameData frameData;
    uint32_t desSize = 0;
    std::vector<char> streamData;
    StreamDescriptions* descriptions = NULL;
    Graphics graphic;

    while (true)
    {
        RS_ERROR err = rs_awaitFrameData(5000, &frameData);
        if (err == RS_ERROR_STREAMS_CHANGED)
        {
            try
            {
                // first call would return RS_ERROR_BUFFER_OVERFLOW but populate desSize -- required size in bytes.
                rs_getStreams(nullptr, &desSize); 
                streamData.resize(desSize);
                // this time should be able to read bytes into the buffer of streamData, then reinterpret the memory as the struct StreamDescription
                rs_getStreams(reinterpret_cast<StreamDescriptions*>(streamData.data()), &desSize); 
                descriptions = reinterpret_cast<StreamDescriptions*>(streamData.data());

                const size_t numStreams = descriptions ? descriptions->nStreams : 0;
                for (size_t i = 0; i < numStreams; i++)
                {
                    const StreamDescription& description = descriptions->streams[i];

                    //once we get the description, we make window and populte 1)handle 2)width&depth 
                    std::string streamName = std::string(description.channel) + description.name;
                    HWND window = engine.window.makeWindow(streamName.c_str(), description.width, description.height);
                    const Graphics temp(engine.dx11, GraphicsInfo(description.width, description.height, window, description.handle, description.format));
                    graphic = temp;
                    if (graphic.getDxDevice() == nullptr)
                    {
                        engine.log.popMessageBox("initating DirectX with stream description failed.");
                        return 6;
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
            engine.log.log("Exiting due to quit requested from rendersteam.");
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

        if (engine.window.processMessage())
        {
            const size_t numStreams = descriptions ? descriptions->nStreams : 0;
            for (size_t i = 0; i < numStreams; i++)
            {
                const StreamDescription& description = descriptions->streams[i];
                CameraResponseData response;
                response.tTracked = frameData.tTracked;
                if (rs_getFrameCamera(description.handle, &response.camera) == RS_ERROR_SUCCESS)
                {
                    auto matFinal = engine.calculateFrame(description, response);
                    graphic.render(matFinal);
                    graphic.getSwapChain()->Present(0, 0);

                    SenderFrameTypeData data;
                    data.dx11.resource = graphic.getRsTexture().Get();
                    if (rs_sendFrame(description.handle, RS_FRAMETYPE_DX11_TEXTURE, data, &response) != RS_ERROR_SUCCESS)
                    {
                        engine.log.popMessageBox("Failed to send frame");
                        rs_shutdown();
                        return 7;
                    }
                }
            }
        }
        else
        {
            engine.log.log("Exiting due to quit request from window interaction.");
            rs_shutdown();
            if (err == RS_ERROR_SUCCESS)
                return 0;
            else
                return 99;
        }
    }
    if (rs_shutdown() != RS_ERROR_SUCCESS)
    {
        engine.log.popMessageBox("Failed to shutdown RenderStream");
        return 99;
    }
    return 0;
}
