#include "include/renderInstance.hpp"

RenderStream* RenderStream::instancePtr = NULL;
int main()
{
#ifdef _DEBUG
    MessageBoxA(NULL, "Attach Debugger Now", NULL, NULL);
#endif

    RenderStream* rs = RenderStream::getInstance();
    rs->loadAPI();
    rs->initialise();

    FrameData frameData;
    std::vector<RenderInstance> renderInstances;
    while (true)
    {
        RS_ERROR err = rs->awaitFrameData(5000, &frameData);
        if (err == RS_ERROR_STREAMS_CHANGED)
        {
            try
            {
                StreamDescriptions* descriptions = rs->getStreamsDescriptions();
                const size_t numStreams = descriptions ? descriptions->nStreams : 0;
                for (size_t i = 0; i < numStreams; i++)
                {
                    StreamDescription& description = descriptions->streams[i];
                    renderInstances.emplace_back(description);
                }
            }
            catch (const std::exception& e)
            {
                ErrorLogger::popMessageBox(e.what());
                rs->shutdown();
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
            rs->shutdown();
            return 0;
        }
        else if (err != RS_ERROR_SUCCESS)
        {
            ErrorLogger::popMessageBox("rs_awaitFrameData returned error.");
            break;
        }

        for (RenderInstance &renderInstance : renderInstances)
        {
            CameraResponseData response;
            response.tTracked = frameData.tTracked;
            renderInstance.sendFrame(response, frameData.scene);
            renderInstance.present();
        }
    }
    rs->shutdown();
    return 0;
}
