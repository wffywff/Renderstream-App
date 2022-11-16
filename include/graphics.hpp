#pragma once
#include <d3d11.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>
#include <wrl/client.h>
#include "include/d3renderstream.h"
#include <vector>
#include <Shlwapi.h>
#include <unordered_map>
#pragma comment (lib, "Shlwapi.lib")
#pragma comment (lib, "d3d11.lib")
#pragma comment (lib, "d3dcompiler.lib")


//struct VERTEX { DirectX::XMFLOAT3 Position; DirectX::XMFLOAT3 Color; };
struct texVERTEX { DirectX::XMFLOAT3 pos; DirectX::XMFLOAT2 uv; };

static constexpr DirectX::XMFLOAT3 cubeVertices[] =
{
    DirectX::XMFLOAT3(-0.5f, 0.5f,-0.5f),
    DirectX::XMFLOAT3(0.5f, 0.5f,-0.5f),
    DirectX::XMFLOAT3(-0.5f,-0.5f,-0.5f),
    DirectX::XMFLOAT3(0.5f,-0.5f,-0.5f),

    DirectX::XMFLOAT3(-0.5f, 0.5f, 0.5f),
    DirectX::XMFLOAT3(0.5f, 0.5f, 0.5f),
    DirectX::XMFLOAT3(-0.5f,-0.5f, 0.5f),
    DirectX::XMFLOAT3(0.5f,-0.5f, 0.5f),
};

static constexpr uint16_t cubeIndices[] =
{
    0, 1, 2,    // side 1
    2, 1, 3,
    4, 0, 6,    // side 2
    6, 0, 2,
    7, 5, 6,    // side 3
    6, 5, 4,
    3, 1, 7,    // side 4
    7, 1, 5,
    4, 5, 0,    // side 5
    0, 5, 1,
    3, 7, 2,    // side 6
    2, 7, 6,
};

static constexpr texVERTEX planeVertices[] =
{
    DirectX::XMFLOAT3(-8.0f, 4.5f, -0.0f), DirectX::XMFLOAT2 (0.0f, 0.0f),
    DirectX::XMFLOAT3(8.0f,4.5f, -0.0f), DirectX::XMFLOAT2(0.0f, 1.0f),
    DirectX::XMFLOAT3(-8.0f,-4.5f,0.0f),DirectX::XMFLOAT2(1.0f, 0.0f),
    DirectX::XMFLOAT3(8.0f,-4.5f,0.0f), DirectX::XMFLOAT2(1.0f, 1.0f),
};

static constexpr uint16_t planeIndices[] =
{
    0, 1, 2,
    2, 1, 3,
};

class Scene
{
public:
    virtual int loadMesh() = 0;
    virtual void render() = 0;
    Microsoft::WRL::ComPtr<ID3D11Device>        dev;
    Microsoft::WRL::ComPtr<ID3D11DeviceContext> devcon;
    Microsoft::WRL::ComPtr<ID3D11InputLayout> pLayout;
    Microsoft::WRL::ComPtr<ID3D11VertexShader> pVS;
    Microsoft::WRL::ComPtr<ID3D11PixelShader> pPS;
    Microsoft::WRL::ComPtr<ID3D11Buffer> pVBuffer;
    Microsoft::WRL::ComPtr<ID3D11Buffer> pCBuffer;
    Microsoft::WRL::ComPtr<ID3D11Buffer> pIBuffer;
    Microsoft::WRL::ComPtr<ID3D11SamplerState> samplerState;
};

class Scene0 : public Scene
{
public:
    Scene0(const dx11device& dx11)
    {
        dev = dx11.dev;
        devcon = dx11.devcon;
    }
    int loadMesh() override;
    void render() override;

};

class Scene1 : public Scene
{
public:
    Scene1(const dx11device& dx11)
    {
        dev = dx11.dev;
        devcon = dx11.devcon;
    }
    int loadMesh() override;
    void render() override;
};

class SceneFactory
{
public:
    static std::unique_ptr<Scene> createScene(int sceneNumber, const dx11device& dx11)
    {
        switch (sceneNumber)
        {
        case 0:
            return std::make_unique<Scene0>(dx11);
        case 1:
            return std::make_unique<Scene1>(dx11);
        defualt:
            return nullptr;
        }
    }
};

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
    //TODO: can not delete copy constructor because RenderInstance's constructo rely on that. 
    // However is that what I want?

    int initDx(const GraphicsInfo& info);
    void render(const DirectX::XMMATRIX matFinal);
    Microsoft::WRL::ComPtr<ID3D11Device> getDxDevice() const { return dev; };
    Microsoft::WRL::ComPtr<IDXGISwapChain> getSwapChain() const { return swapchain; };
    DXGI_FORMAT toDxgiFormat(RSPixelFormat rsFormat);

    std::unordered_map<StreamHandle, RenderTarget> renderstreamTarget;
    StreamHandle m_sHandle = 0;

private:
    Microsoft::WRL::ComPtr<ID3D11Device>        dev;
    Microsoft::WRL::ComPtr<ID3D11DeviceContext> devcon;
    Microsoft::WRL::ComPtr<IDXGISwapChain> swapchain;
    Microsoft::WRL::ComPtr<ID3D11RenderTargetView> backbuffer;
    Microsoft::WRL::ComPtr<ID3D11DepthStencilView> depthStencilView;
};

