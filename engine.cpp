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
}

DirectX::XMMATRIX Engine::calculateFrame(const StreamDescription& description, const CameraResponseData& response)
{
    static float Time = 0.0f; Time += 0.01f;

    // create a world matrices
    const DirectX::XMMATRIX matRotate = DirectX::XMMatrixRotationY(Time);

    const float pitch = -DirectX::XMConvertToRadians(response.camera.rx);
    const float yaw = DirectX::XMConvertToRadians(response.camera.ry);
    const float roll = -DirectX::XMConvertToRadians(response.camera.rz);

    //TODO use ImGui::DragFloat3() to override the value here
    //static float translationOffset[3] = { 0, 0, 0 };
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

    return matFinal;
}