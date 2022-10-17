#pragma once
#include <d3d11.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>
#include <wrl/client.h>
#include "include/d3renderstream.h"
#include <vector>
#include <Shlwapi.h>
#pragma comment (lib, "Shlwapi.lib")
#pragma comment (lib, "d3d11.lib")
#pragma comment (lib, "d3dcompiler.lib")

#include "Generated_Code/VertexShader.h"
#include "Generated_Code/PixelShader.h"
#include <unordered_map>

struct VERTEX { DirectX::XMFLOAT3 Position; DirectX::XMFLOAT3 Color; };
struct RenderTarget
{
    Microsoft::WRL::ComPtr<ID3D11Texture2D> texture;
    Microsoft::WRL::ComPtr<ID3D11RenderTargetView> view;
};

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

struct GraphicsInfo
{
    GraphicsInfo(const uint32_t width, const uint32_t height, const HWND window, const StreamHandle handle, const RSPixelFormat format)
        :m_width(width),m_height(height),m_window(window),m_streamHandle(handle),m_format(format)
    {
    }
    uint32_t m_width;
    uint32_t m_height;
    HWND m_window;
    StreamHandle m_streamHandle;
    RSPixelFormat m_format;
};

class Graphics
{
public:
    Graphics() {};
    Graphics(const dx11device& dxObject, const GraphicsInfo& info);
    // don't think we need a desctructor since all the member variables are comPtr
    Graphics(const Graphics& other);
    Graphics& operator=(const Graphics& other);
    // rule of five - shall we get move constructor and assignment as well? 
    // since its comPtr, should just increment the count. 
    // maybe should steal the ownership of the dev and devon out of dx11device?
    // would it be significantly faster?



    int initDx(const GraphicsInfo& info);
    DXGI_FORMAT toDxgiFormat(RSPixelFormat rsFormat);
    void render(const DirectX::XMMATRIX matFinal);
    Microsoft::WRL::ComPtr<ID3D11Device> getDxDevice() const { return dev; };
    Microsoft::WRL::ComPtr<IDXGISwapChain> getSwapChain() const { return swapchain; };
    Microsoft::WRL::ComPtr<ID3D11Texture2D> getRsTexture() const { return m_rsTexture; };
    std::unordered_map<StreamHandle, RenderTarget> renderstreamTarget;

private:
    Microsoft::WRL::ComPtr<ID3D11Device>        dev;
    Microsoft::WRL::ComPtr<ID3D11DeviceContext> devcon;
    Microsoft::WRL::ComPtr<IDXGISwapChain> swapchain;
    Microsoft::WRL::ComPtr<ID3D11RenderTargetView> backbuffer;
    Microsoft::WRL::ComPtr<ID3D11DepthStencilView> depthStencilView;
    Microsoft::WRL::ComPtr<ID3D11InputLayout> pLayout;
    Microsoft::WRL::ComPtr<ID3D11VertexShader> pVS;
    Microsoft::WRL::ComPtr<ID3D11PixelShader> pPS;
    Microsoft::WRL::ComPtr<ID3D11Buffer> pVBuffer;
    Microsoft::WRL::ComPtr<ID3D11Buffer> pCBuffer;
    Microsoft::WRL::ComPtr<ID3D11Buffer> pIBuffer;
    Microsoft::WRL::ComPtr<ID3D11Texture2D> m_rsTexture;
    Microsoft::WRL::ComPtr<ID3D11RenderTargetView> m_rsTargetView;
};