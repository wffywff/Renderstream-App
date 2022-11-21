#pragma once

#include <d3d11.h>
#include <d3dcompiler.h>
#include <wrl/client.h>
#pragma comment (lib, "d3d11.lib")
#pragma comment (lib, "d3dcompiler.lib")

struct dx11device
{
    dx11device()
    {
#ifdef _DEBUG
        const uint32_t deviceFlags = D3D11_CREATE_DEVICE_DEBUG;
#else
        const uint32_t deviceFlags = 0;
#endif

        D3D_FEATURE_LEVEL levels[] =
        {
            D3D_FEATURE_LEVEL_11_1,
            D3D_FEATURE_LEVEL_11_0,
            D3D_FEATURE_LEVEL_10_1,
            D3D_FEATURE_LEVEL_10_0,
            D3D_FEATURE_LEVEL_9_3,
            D3D_FEATURE_LEVEL_9_2,
            D3D_FEATURE_LEVEL_9_1,
        };

        //Step1: create device
        D3D11CreateDevice(
            nullptr,                    // Specify nullptr to use the default adapter.
            D3D_DRIVER_TYPE_HARDWARE,   // Create a device using the hardware graphics driver.
            0,                          // Should be 0 unless the driver is D3D_DRIVER_TYPE_SOFTWARE.
            deviceFlags,                // Set debug and Direct2D compatibility flags.
            levels,                     // List of feature levels this app can support.
            ARRAYSIZE(levels),          // Size of the list above.
            D3D11_SDK_VERSION,          // Always set this to D3D11_SDK_VERSION for Windows Store apps.
            &dev,                    // [OUT] Returns the Direct3D device created.
            NULL,            // [OUT] Returns feature level of device created.
            &devcon              // [OUT]Returns the device immediate devcon.
        );
    }
    Microsoft::WRL::ComPtr<ID3D11Device>        dev;
    Microsoft::WRL::ComPtr<ID3D11DeviceContext> devcon;
};