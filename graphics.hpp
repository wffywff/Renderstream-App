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

class Graphics
{
public:
    Graphics() {};
    HRESULT makeDxDevice();
    int initDx(HWND window, const StreamDescription& description);
    DXGI_FORMAT toDxgiFormat(RSPixelFormat rsFormat);
    Microsoft::WRL::ComPtr<ID3D11Device> getDxDevice() const { return dev; };
    Microsoft::WRL::ComPtr<IDXGISwapChain> getSwapChain() const { return swapchain; };

    void render(ID3D11RenderTargetView* renderstreamRenderTarget, const DirectX::XMMATRIX matFinal);

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
};