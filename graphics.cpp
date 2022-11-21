#include "include/graphics.hpp"
#include "include/widget.hpp"
#include <stdexcept>
#include <fstream>
#include <sstream>
#include <string>
#include <WICTextureLoader.h>
#include <shobjidl.h> 

#include "Generated_Code/VertexShader.h"
#include "Generated_Code/PixelShader.h"
#include "Generated_Code/TexVertexShader.h"
#include "Generated_Code/TexPixelShader.h"

auto failureReport = [](const char* msg)
{
    ErrorLogger::popMessageBox(msg);
    return -1;
};

// TODO: currently texture on plane does NOT work
int PlaneScene::loadScene()
{
    //Step7/8:create shaders base off the shader.h's binary blob 
    if (FAILED(dxDeviceScene.dev->CreateVertexShader(TexVertexShaderBlob, std::size(TexVertexShaderBlob), NULL, pVS.GetAddressOf())))  
        return failureReport("Failed to create vertex shader.");
    if (FAILED(dxDeviceScene.dev->CreatePixelShader(TexPixelShaderBlob, std::size(TexPixelShaderBlob), NULL, pPS.GetAddressOf())))
        return failureReport("Failed to create pixel shader.");

    //Step9:create the input layout
    D3D11_INPUT_ELEMENT_DESC ied[] =
    {
        {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0},
    };

    if (FAILED(dxDeviceScene.dev->CreateInputLayout(ied, 2, VertexShaderBlob, std::size(VertexShaderBlob), &pLayout)))
        return failureReport("Failed to create input layout.");

    D3D11_BUFFER_DESC bd;
    ZeroMemory(&bd, sizeof(bd));

    bd.Usage = D3D11_USAGE_DYNAMIC;
    bd.ByteWidth = sizeof(texVERTEX) * ARRAYSIZE(planeVertices);
    bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

    if (FAILED(dxDeviceScene.dev->CreateBuffer(&bd, NULL, pVBuffer.GetAddressOf())))
        return failureReport("Failed to create vertex buffer.");

    // copy the vertices into the buffer
    D3D11_MAPPED_SUBRESOURCE ms;
    dxDeviceScene.devcon->Map(pVBuffer.Get(), NULL, D3D11_MAP_WRITE_DISCARD, NULL, &ms);    // map the buffer
    memcpy(ms.pData, planeVertices, sizeof(planeVertices));                 // copy the data
    dxDeviceScene.devcon->Unmap(pVBuffer.Get(), NULL);

    // create the index buffer
    bd.Usage = D3D11_USAGE_DYNAMIC;
    bd.ByteWidth = sizeof(planeIndices);
    bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
    bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    //The resource is to be mappable so that the CPU can change its content
    //Resources created with this flag cannot be set as outputs of the pipeline.
    bd.MiscFlags = 0;

    if (FAILED(dxDeviceScene.dev->CreateBuffer(&bd, NULL, pIBuffer.GetAddressOf())))
        return failureReport("Failed to create index shader.");

    dxDeviceScene.devcon->Map(pIBuffer.Get(), NULL, D3D11_MAP_WRITE_DISCARD, NULL, &ms);    // map the buffer
    memcpy(ms.pData, planeIndices, sizeof(planeIndices));                   // copy the data
    dxDeviceScene.devcon->Unmap(pIBuffer.Get(), NULL);

    D3D11_SAMPLER_DESC sampDesc;
    ZeroMemory(&sampDesc, sizeof(sampDesc));
    sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
    sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
    sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
    sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
    sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
    sampDesc.MinLOD = 0;
    sampDesc.MaxLOD = D3D11_FLOAT32_MAX;

    if (FAILED(dxDeviceScene.dev->CreateSamplerState(&sampDesc, samplerState.GetAddressOf())))
        return failureReport("Failed to create sampler state.");

    // TODO: currently scene is created every frame
    // therefore can not prompt selection every frame.
    // Need to figure out where to call selectTexture();

    //selectTexture();
    //if (FAILED(DirectX::CreateWICTextureFromFile(dxDeviceScene.dev.Get(), texturePath, nullptr, preloadTexture.GetAddressOf())))
    //    return failureReport("Failed to create texture from file.");

    UINT stride = sizeof(texVERTEX);
    UINT offset = 0;
    dxDeviceScene.devcon->IASetVertexBuffers(0, 1, pVBuffer.GetAddressOf(), &stride, &offset);
    dxDeviceScene.devcon->IASetIndexBuffer(pIBuffer.Get(), DXGI_FORMAT_R16_UINT, 0);
    dxDeviceScene.devcon->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    dxDeviceScene.devcon->IASetInputLayout(pLayout.Get());
    dxDeviceScene.devcon->VSSetShader(pVS.Get(), nullptr, 0);
    dxDeviceScene.devcon->PSSetShader(pPS.Get(), nullptr, 0);
    dxDeviceScene.devcon->PSSetShaderResources(0, 1, preloadTexture.GetAddressOf());
    return sizeof(planeIndices)/sizeof(uint16_t);
}

void PlaneScene::selectTexture()
{
    HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED |
        COINIT_DISABLE_OLE1DDE);
    if (SUCCEEDED(hr))
    {
        IFileOpenDialog* pFileOpen;
        hr = CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_ALL,
            IID_IFileOpenDialog, reinterpret_cast<void**>(&pFileOpen));
        if (SUCCEEDED(hr))
        {
            hr = pFileOpen->Show(NULL);
            if (SUCCEEDED(hr))
            {
                IShellItem* pItem;
                hr = pFileOpen->GetResult(&pItem);
                if (SUCCEEDED(hr))
                {
                    LPWSTR pszFilePath;
                    hr = pItem->GetDisplayName(SIGDN_FILESYSPATH, &pszFilePath);
                    if (SUCCEEDED(hr))
                    {
                        texturePath = pszFilePath;
                    }
                    pItem->Release();
                }
            }
            else if (hr == HRESULT_FROM_WIN32(ERROR_CANCELLED))
            {
                MessageBox(NULL, L"No texture for plane.", L"Error", MB_OK);
            }
            pFileOpen->Release();
        }
        CoUninitialize();
    }
}

int CubeScene::loadScene()
{
    if (FAILED(dxDeviceScene.dev->CreateVertexShader(VertexShaderBlob, std::size(VertexShaderBlob), NULL, pVS.GetAddressOf())))
        return failureReport("Failed to create vertex shader.");
    if (FAILED(dxDeviceScene.dev->CreatePixelShader(PixelShaderBlob, std::size(PixelShaderBlob), NULL, pPS.GetAddressOf())))
        return failureReport("Failed to create pixel shader.");

    D3D11_INPUT_ELEMENT_DESC ied[] =
    {
        {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0},
    };

    if (FAILED(dxDeviceScene.dev->CreateInputLayout(ied, 2, VertexShaderBlob, std::size(VertexShaderBlob), &pLayout)))
        return failureReport("Failed to create input lay out.");

    D3D11_BUFFER_DESC bd;
    ZeroMemory(&bd, sizeof(bd));

    bd.Usage = D3D11_USAGE_DYNAMIC;
    bd.ByteWidth = sizeof(VERTEX) * ARRAYSIZE(cubeVertices);
    bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

    if (FAILED(dxDeviceScene.dev->CreateBuffer(&bd, NULL, pVBuffer.GetAddressOf())))
        return failureReport("Failed to create vertex buffer.");

    // copy the vertices into the buffer
    D3D11_MAPPED_SUBRESOURCE ms;
    dxDeviceScene.devcon->Map(pVBuffer.Get(), NULL, D3D11_MAP_WRITE_DISCARD, NULL, &ms);    // map the buffer
    memcpy(ms.pData, cubeVertices, sizeof(cubeVertices));                 // copy the data
    dxDeviceScene.devcon->Unmap(pVBuffer.Get(), NULL);

    // create the index buffer
    bd.Usage = D3D11_USAGE_DYNAMIC;
    bd.ByteWidth = sizeof(uint16_t) * ARRAYSIZE(cubeIndices);
    bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
    bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    //The resource is to be mappable so that the CPU can change its content
    //Resources created with this flag cannot be set as outputs of the pipeline.
    bd.MiscFlags = 0;

    if (FAILED(dxDeviceScene.dev->CreateBuffer(&bd, NULL, pIBuffer.GetAddressOf())))
        return failureReport("Failed to create index shader.");

    dxDeviceScene.devcon->Map(pIBuffer.Get(), NULL, D3D11_MAP_WRITE_DISCARD, NULL, &ms);    // map the buffer
    memcpy(ms.pData, cubeIndices, sizeof(cubeIndices));                   // copy the data
    dxDeviceScene.devcon->Unmap(pIBuffer.Get(), NULL);

    UINT stride = sizeof(DirectX::XMFLOAT3);
    UINT offset = 0;
    dxDeviceScene.devcon->IASetVertexBuffers(0, 1, pVBuffer.GetAddressOf(), &stride, &offset);
    dxDeviceScene.devcon->IASetIndexBuffer(pIBuffer.Get(), DXGI_FORMAT_R16_UINT, 0);
    dxDeviceScene.devcon->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    dxDeviceScene.devcon->IASetInputLayout(pLayout.Get());
    dxDeviceScene.devcon->VSSetShader(pVS.Get(), nullptr, 0);
    dxDeviceScene.devcon->PSSetShader(pPS.Get(), nullptr, 0);
    return sizeof(cubeIndices) / sizeof(uint16_t);
}

Graphics::Graphics(const dx11device& dxObject, const GraphicsInfo& info)
{
    //copy the dx11device, increment ref count of dev and devcon
    dxDevice = dxObject;
    initDx(info);
}

void Graphics::initDx(const GraphicsInfo& info)
{
    // make description for a 2dTexture
    D3D11_TEXTURE2D_DESC rtDesc;
    ZeroMemory(&rtDesc, sizeof(D3D11_TEXTURE2D_DESC));

    //populate the 2d texture's description from d3
    rtDesc.Width = info.m_width;
    rtDesc.Height = info.m_height;
    // miplevel specfces the subresource from a 1d texutre to use in a shader-resource view
    // mipmap is a sequence of textures, each of which is a progressively lower resolution representation of the same image
    // the height and width of each image, or level, in the mipmap is a power of two smaller than the previous level. 

    rtDesc.MipLevels = 1;
    rtDesc.ArraySize = 1;
    rtDesc.Format = toDxgiFormat(info.m_format);
    rtDesc.SampleDesc.Count = 1;
    rtDesc.Usage = D3D11_USAGE_DEFAULT;
    rtDesc.BindFlags = D3D11_BIND_RENDER_TARGET;
    rtDesc.CPUAccessFlags = 0;
    rtDesc.MiscFlags = D3D11_RESOURCE_MISC_SHARED;

    if (FAILED(dxDevice.dev->CreateTexture2D(&rtDesc, nullptr, renderstreamTarget.texture.GetAddressOf())))
    {
        ErrorLogger::popMessageBox("Failed to create renderstream target texture.");
        return;
    }

    D3D11_RENDER_TARGET_VIEW_DESC rtvDesc;
    ZeroMemory(&rtvDesc, sizeof(D3D11_RENDER_TARGET_VIEW_DESC));
    rtvDesc.Format = rtDesc.Format;
    rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
    if (FAILED(dxDevice.dev->CreateRenderTargetView(renderstreamTarget.texture.Get(), &rtvDesc, renderstreamTarget.view.GetAddressOf())))
    {
        ErrorLogger::popMessageBox("Failed to create renderstream target texture view.");
        return;
    }

    //Step2: create swapchain
    DXGI_SWAP_CHAIN_DESC scd;
    ZeroMemory(&scd, sizeof(DXGI_SWAP_CHAIN_DESC));

    scd.BufferCount = 2;                                   // 2 for tradtional double-buffered behavior to avoid tearing. Set it to 3 if graphic content takes more  than one monitor refresh cycle to render single frame (60hz for example)
    scd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;    // use 32-bit color
    scd.BufferDesc.Width = info.m_width;                   // set the back buffer width
    scd.BufferDesc.Height = info.m_height;                 // set the back buffer height
    scd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;     // indicates that the swap chain will be a drawing surface, allowing to use it as a D3d render-target
    scd.SwapEffect = DXGI_SWAP_EFFECT_SEQUENTIAL;
    scd.OutputWindow = info.m_window;                               // the window to be used
    scd.SampleDesc.Count = 1;                              // how many multisamples
    scd.Windowed = TRUE;                                   // windowed/full-screen mode
    scd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;    // allow full-screen switching

    Microsoft::WRL::ComPtr<IDXGIDevice1> dxgiDevice;
    dxDevice.dev.As(&dxgiDevice);
    Microsoft::WRL::ComPtr<IDXGIAdapter> dxgiAdapter;
    dxgiDevice->GetAdapter(dxgiAdapter.GetAddressOf());
    Microsoft::WRL::ComPtr<IDXGIFactory1> dxgiFactory;
    dxgiAdapter->GetParent(__uuidof(IDXGIFactory1), &dxgiFactory);

    HRESULT hr = dxgiFactory->CreateSwapChain(
        dxDevice.dev.Get(),
        &scd,
        &swapchain
    );
    dxgiDevice = nullptr;
    dxgiAdapter = nullptr;
    dxgiFactory = nullptr;
    if (FAILED(hr))
    {
        ErrorLogger::popMessageBox("Failed to create swapchain.");
        return;
    }

    //Step3:create the back buffer for drawing the pixel to 
    ID3D11Texture2D* pBackBuffer;
    if (FAILED(swapchain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&pBackBuffer)))
    {
        ErrorLogger::popMessageBox("Failed to create back buffer.");
        return;
    }

    //Step4:creat renderTargetView, which is a way to access resource, bounded to the buffer resource
    if (FAILED(dxDevice.dev->CreateRenderTargetView(pBackBuffer, NULL, backbuffer.GetAddressOf())))
    {
        ErrorLogger::popMessageBox("Failed to create back buffer rendertarget view.");
        return;
    }
    //note: once renderTargetView is created, the true backBuffer is no longer in use, release it. 
    pBackBuffer->Release();

    //Step5:create the bepth buffer for writing the depth info of each pixel to
    D3D11_TEXTURE2D_DESC texd;
    ZeroMemory(&texd, sizeof(texd));

    texd.Width = info.m_width;
    texd.Height = info.m_height;
    texd.ArraySize = 1;
    texd.MipLevels = 1;
    texd.SampleDesc.Count = 1;
    texd.Format = DXGI_FORMAT_D32_FLOAT;
    texd.BindFlags = D3D11_BIND_DEPTH_STENCIL;

    ID3D11Texture2D* pDepthBuffer;
    if (FAILED(dxDevice.dev->CreateTexture2D(&texd, NULL, &pDepthBuffer)))
    {
        ErrorLogger::popMessageBox("Failed to create depth buffer.");
        return;
    }

    //Step6:create the depth stencil 
    D3D11_DEPTH_STENCIL_VIEW_DESC dsvd;
    ZeroMemory(&dsvd, sizeof(dsvd));
    dsvd.Format = DXGI_FORMAT_D32_FLOAT;
    dsvd.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DMS;
    if (FAILED(dxDevice.dev->CreateDepthStencilView(pDepthBuffer, &dsvd, depthStencilView.GetAddressOf())))
    {
        ErrorLogger::popMessageBox("Failed to create depth stencil view.");
        return;
    }
    pDepthBuffer->Release();

    D3D11_VIEWPORT viewport;
    ZeroMemory(&viewport, sizeof(D3D11_VIEWPORT));
    viewport.Width = static_cast<float>(info.m_width);
    viewport.Height = static_cast<float>(info.m_height);
    viewport.MinDepth = 0;
    viewport.MaxDepth = 1;
    dxDevice.devcon->RSSetViewports(1, &viewport);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    ImGui_ImplWin32_Init(info.m_window);
    ImGui_ImplDX11_Init(dxDevice.dev.Get(), dxDevice.devcon.Get());
    ImGui::StyleColorsDark();
}

void Graphics::render(const DirectX::XMMATRIX &matFinal, const int sceneNum)
{
    //TODO: currently hard-coded to load scene till schema is incorporated
    auto scene = SceneFactory::createScene(1, dxDevice);
    int indexCount = scene->loadScene();

    ID3D11RenderTargetView* renderTargetList[2] = { backbuffer.Get(), renderstreamTarget.view.Get() };
    dxDevice.devcon->OMSetRenderTargets(2, renderTargetList, nullptr);

    const float clearColour[4] = { 0.f, 0.f, 0.f, 0.f };
    for (int i = 0; i < 2; i++)
    {
        dxDevice.devcon->ClearRenderTargetView(renderTargetList[i], clearColour);
    }
    // set up constant buffer
    D3D11_BUFFER_DESC bd;
    ZeroMemory(&bd, sizeof(bd));

    bd.Usage = D3D11_USAGE_DEFAULT;
    bd.ByteWidth = 64;
    bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;

    dxDevice.dev->CreateBuffer(&bd, NULL, pCBuffer.GetAddressOf());
    dxDevice.devcon->UpdateSubresource(pCBuffer.Get(), 0, nullptr, &matFinal, 0, 0);
    dxDevice.devcon->VSSetConstantBuffers(0, 1, pCBuffer.GetAddressOf());

    dxDevice.devcon->DrawIndexed(indexCount, 0, 0);
}

DXGI_FORMAT Graphics::toDxgiFormat(RSPixelFormat rsFormat)
{
    switch (rsFormat)
    {
    case RS_FMT_BGRA8:
    case RS_FMT_BGRX8:
        return DXGI_FORMAT_R8G8B8A8_UNORM;
    case RS_FMT_RGBA32F:
        return DXGI_FORMAT_R32G32B32A32_FLOAT;
    case RS_FMT_RGBA16:
        return DXGI_FORMAT_R16G16B16A16_UNORM;
    default:
        throw std::runtime_error("bad pixel format");
    }
}