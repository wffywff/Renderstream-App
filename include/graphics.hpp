#pragma once
#include "include/d3renderstream.h"
#include "include/utility.hpp"
#include "dx11.hpp"
#include <DirectXMath.h>
#include <vector>
#include <Shlwapi.h>
#include <unordered_map>
#include <memory>
#pragma comment (lib, "Shlwapi.lib")



struct VERTEX { DirectX::XMFLOAT3 Position; DirectX::XMFLOAT3 Color; };
struct texVERTEX { DirectX::XMFLOAT3 Position; DirectX::XMFLOAT2 UV; };

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
    virtual int loadScene() = 0;
    dx11device dxDeviceScene;
    Microsoft::WRL::ComPtr<ID3D11InputLayout> pLayout;
    Microsoft::WRL::ComPtr<ID3D11VertexShader> pVS;
    Microsoft::WRL::ComPtr<ID3D11PixelShader> pPS;
    Microsoft::WRL::ComPtr<ID3D11Buffer> pVBuffer;
    Microsoft::WRL::ComPtr<ID3D11Buffer> pIBuffer;

};

class PlaneScene : public Scene
{
public:
    PlaneScene(const dx11device& dxDevice)
    {
        dxDeviceScene = dxDevice;
    }
    int loadScene() override;
private:
    void selectTexture();
    Microsoft::WRL::ComPtr<ID3D11SamplerState> samplerState;
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> preloadTexture;
    wchar_t* texturePath; 
};

class CubeScene : public Scene
{
public:
    CubeScene(const dx11device& dxDevice)
    {
        dxDeviceScene = dxDevice;
    }
    int loadScene() override;
};

class SceneFactory
{
public:
    static std::shared_ptr<Scene> createScene(int sceneNumber, const dx11device& dxDevice)
    {
        switch (sceneNumber)
        {
        case 0:
            return std::make_shared<PlaneScene>(dxDevice);
        case 1:
            return std::make_shared<CubeScene>(dxDevice);
        default:
            return nullptr;
        }
    }
};

struct RenderTarget
{
    Microsoft::WRL::ComPtr<ID3D11Texture2D> texture;
    Microsoft::WRL::ComPtr<ID3D11RenderTargetView> view;
};

struct GraphicsInfo
{
    GraphicsInfo(const uint32_t width, const uint32_t height, const HWND window, const StreamHandle handle, const RSPixelFormat format)
        :m_width(width), m_height(height), m_window(window), m_streamHandle(handle), m_format(format)
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
    Graphics() = delete;
    Graphics(const dx11device& dxDevice, const GraphicsInfo& info);

    void initDx(const GraphicsInfo& info);
    void render(const DirectX::XMMATRIX& matFinal, const int indexCount);

    Microsoft::WRL::ComPtr<IDXGISwapChain> getSwapChain() const { return swapchain; };
    RenderTarget renderstreamTarget;

private:
    DXGI_FORMAT toDxgiFormat(RSPixelFormat rsFormat);

    dx11device dxDevice;
    Microsoft::WRL::ComPtr<IDXGISwapChain> swapchain;
    Microsoft::WRL::ComPtr<ID3D11RenderTargetView> backbuffer;
    Microsoft::WRL::ComPtr<ID3D11DepthStencilView> depthStencilView;
    Microsoft::WRL::ComPtr<ID3D11Buffer> pCBuffer;
};
