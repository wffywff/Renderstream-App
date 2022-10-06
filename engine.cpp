#include "engine.hpp"

HMODULE Engine::loadRenderStreamDll()
{
    HKEY hKey;
    if (FAILED(RegOpenKeyEx(HKEY_CURRENT_USER, TEXT("Software\\d3 Technologies\\d3 Production Suite"), 0, KEY_READ, &hKey)))
    {
        log.log("Failed to open 'Software\\d3 Technologies\\d3 Production Suite' registry key");
        return nullptr;
    }

    TCHAR buffer[512];
    DWORD bufferSize = sizeof(buffer);
    if (FAILED(RegQueryValueEx(hKey, TEXT("exe path"), 0, nullptr, reinterpret_cast<LPBYTE>(buffer), &bufferSize)))
    {
        log.log("Failed to query value of 'exe path'");
        return nullptr;
    }

    if (!PathRemoveFileSpec(buffer))
    {
        log.log("Failed to remove file spec from path");
        return nullptr;
    }

    if (!PathAppend(buffer, TEXT("\\d3renderstream.dll")))
    {
        log.log("Failed to append filename to path");
        return nullptr;
    }

    //TCHAR buffer[512];

    //PathAppend(buffer, TEXT("C:\\Program Files\\d3 Production Suite\\build\\msvc\\d3renderstream.dll"));
        
    HMODULE hLib = ::LoadLibraryEx(buffer, NULL, LOAD_LIBRARY_SEARCH_DLL_LOAD_DIR | LOAD_LIBRARY_SEARCH_APPLICATION_DIR | LOAD_LIBRARY_SEARCH_SYSTEM32 | LOAD_LIBRARY_SEARCH_USER_DIRS);
    if (!hLib)
    {
        log.log("Failed to load dll");
        return nullptr;
    }
    return hLib;
}

void Engine::setup(const StreamDescription& streamDesc)
{
    // populate the handle as the key
    // setup a reference to target struct so we can create them 
    RenderTarget& target = renderstreamTarget[streamDesc.handle];

    // make description for a 2dTexture
    D3D11_TEXTURE2D_DESC rtDesc;
    ZeroMemory(&rtDesc, sizeof(D3D11_TEXTURE2D_DESC));

    //populate the 2d texture's description from d3
    rtDesc.Width = streamDesc.width;
    rtDesc.Height = streamDesc.height;
    // miplevel specfces the subresource from a 1d texutre to use in a shader-resource view
    // mipmap is a sequence of textures, each of which is a progressively lower resolution representation of the same image
    // the height and width of each image, or level, in the mipmap is a power of two smaller than the previous level. 

    rtDesc.MipLevels = 1;
    rtDesc.ArraySize = 1;
    rtDesc.Format = graphics.toDxgiFormat(streamDesc.format);
    rtDesc.SampleDesc.Count = 1;
    rtDesc.Usage = D3D11_USAGE_DEFAULT;
    rtDesc.BindFlags = D3D11_BIND_RENDER_TARGET;
    rtDesc.CPUAccessFlags = 0;
    rtDesc.MiscFlags = D3D11_RESOURCE_MISC_SHARED;

    if (FAILED(graphics.getDxDevice()->CreateTexture2D(&rtDesc, nullptr, target.texture.GetAddressOf())))
        log.popMessageBox("Failed to create render target texture for stream");

    D3D11_RENDER_TARGET_VIEW_DESC rtvDesc;
    ZeroMemory(&rtvDesc, sizeof(D3D11_RENDER_TARGET_VIEW_DESC));
    rtvDesc.Format = rtDesc.Format;
    rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
    if (FAILED(graphics.getDxDevice()->CreateRenderTargetView(target.texture.Get(), &rtvDesc, target.view.GetAddressOf())))
        log.popMessageBox("Failed to create render target view for stream");
}

int Engine::renderFrame(const StreamDescription& description, const CameraResponseData& response)
{
    static float Time = 0.0f; Time += 0.01f;

    // create a world matrices
    const DirectX::XMMATRIX matRotate = DirectX::XMMatrixRotationY(Time);

    const float pitch = -DirectX::XMConvertToRadians(response.camera.rx);
    const float yaw = DirectX::XMConvertToRadians(response.camera.ry);
    const float roll = -DirectX::XMConvertToRadians(response.camera.rz);

    const DirectX::XMMATRIX cameraTranslation = DirectX::XMMatrixTranslation(response.camera.x, response.camera.y, response.camera.z);
    const DirectX::XMMATRIX cameraRotation = DirectX::XMMatrixRotationRollPitchYaw(pitch, yaw, roll);
    //why do we need to inverse the matrix of translation? and also Transpose of cameraRotation? 
    const DirectX::XMMATRIX view = DirectX::XMMatrixInverse(nullptr, cameraTranslation) * DirectX::XMMatrixTranspose(cameraRotation);

    const float throwRatioH = response.camera.focalLength / response.camera.sensorX;
    const float throwRatioV = response.camera.focalLength / response.camera.sensorY;
    const float fovH = 2.0f * atan(0.5f / throwRatioH);
    const float fovV = 2.0f * atan(0.5f / throwRatioV);

    const bool orthographic = response.camera.orthoWidth > 0.0f;
    const float cameraAspect = response.camera.sensorX / response.camera.sensorY;
    float imageHeight, imageWidth;
    if (orthographic)
    {
        imageHeight = response.camera.orthoWidth / cameraAspect;
        imageWidth = cameraAspect * imageHeight;
    }
    else
    {
        imageWidth = 2.0f * tan(0.5f * fovH);
        imageHeight = 2.0f * tan(0.5f * fovV);
    }

    const DirectX::XMMATRIX overscan = DirectX::XMMatrixTranslation(response.camera.cx, response.camera.cy, 0.f);

    const float nearZ = response.camera.nearZ;
    const float farZ = response.camera.farZ;

    const float l = (-0.5f + description.clipping.left) * imageWidth;
    const float r = (-0.5f + description.clipping.right) * imageWidth;
    const float t = (-0.5f + 1.f - description.clipping.top) * imageHeight;
    const float b = (-0.5f + 1.f - description.clipping.bottom) * imageHeight;

    const DirectX::XMMATRIX projection = orthographic ? DirectX::XMMatrixOrthographicOffCenterLH(l, r, b, t, nearZ, farZ) : DirectX::XMMatrixPerspectiveOffCenterLH(l * nearZ, r * nearZ, b * nearZ, t * nearZ, nearZ, farZ);

    DirectX::XMMATRIX matFinal = DirectX::XMMatrixTranspose(matRotate * view * projection * overscan);

    const RenderTarget& target = renderstreamTarget.at(description.handle);
    //make a list of two targets: backbuffer for the swapchain and the targetview to send over RS.
    ID3D11RenderTargetView* targets = target.view.Get();

    graphics.render(targets, matFinal);

    static int frameCount = 0;
    frameCount++;
    auto milliseconds_elapsed = timer.getDuration().count();
    guiWindow.displayFPS(0);
    if (milliseconds_elapsed > 1000)
    {
        guiWindow.displayFPS(frameCount);
        frameCount = 0;
        timer.start();
    }
    graphics.getSwapChain()->Present(0, 0);

    return 0;
}