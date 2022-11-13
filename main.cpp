#include "include/engine.hpp"

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
                uint32_t desSize = 0;
                std::vector<char> data;
                StreamDescriptions* descriptions = nullptr;

                // first call would return RS_ERROR_BUFFER_OVERFLOW but populate desSize -- required size in bytes.
                rs->getStreams(nullptr, &desSize);
                data.resize(desSize);
                // this time should be able to read bytes into the buffer of streamData, then reinterpret the memory as the struct StreamDescription
                rs->getStreams(reinterpret_cast<StreamDescriptions*>(data.data()), &desSize);
                descriptions = reinterpret_cast<StreamDescriptions*>(data.data());

                const size_t numStreams = descriptions ? descriptions->nStreams : 0;
                for (size_t i = 0; i < numStreams; i++)
                {
                    StreamDescription& description = descriptions->streams[i];
                    Window w(description.name, description.width, description.height);
                    Graphics g(rs->getDxDevice(), GraphicsInfo(description.width, description.height, w.getHandle(), description.handle, description.format));
                    renderInstances.emplace_back(w, g, description);
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

        // make sure all renderInstances windows are still needed
        std::vector<RenderInstance>::iterator itr;
        for (itr = renderInstances.begin(); itr < renderInstances.end(); itr++)
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
            // only process the renderInstances still needed
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
            if (rs->getFrameCamera(renderInstances[i].description.handle, &response.camera))
            {
                SenderFrameTypeData data;
                data.dx11.resource = renderInstances[i].render(response).texture.Get();
                rs->sendFrame(renderInstances[i].description.handle, RS_FRAMETYPE_DX11_TEXTURE, data, &response);
                renderInstances[i].fps();
                renderInstances[i].graphic.getSwapChain()->Present(0, 0);
            }
        }
    }
    rs->shutdown();
    return 0;
}
