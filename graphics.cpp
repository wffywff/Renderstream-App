#include "include/graphics.hpp"
#include "include/widget.hpp"
#include <stdexcept>
#include <fstream>
#include <sstream>
#include <string>

#include "Generated_Code/VertexShader.h"
#include "Generated_Code/PixelShader.h"
#include "Generated_Code/TexVertexShader.h"
#include "Generated_Code/TexPixelShader.h"

int Scene0::loadMesh()
{
    //Step7/8:create shaders base off the shader.h's binary blob 
    if (FAILED(dev->CreateVertexShader(TexVertexShaderBlob, std::size(TexVertexShaderBlob), NULL, pVS.GetAddressOf())))  return 87;
    if (FAILED(dev->CreatePixelShader(TexPixelShaderBlob, std::size(TexPixelShaderBlob), NULL, pPS.GetAddressOf())))  return 88;

    //Step9:create the input layout objectzz
    D3D11_INPUT_ELEMENT_DESC ied[] =
    {
        {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0},
    };

    if (FAILED(dev->CreateInputLayout(ied, 2, VertexShaderBlob, std::size(VertexShaderBlob), &pLayout))) return 89;

    D3D11_SAMPLER_DESC sampDesc;
    ZeroMemory(&sampDesc, sizeof(sampDesc));
    sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
    sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
    sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
    sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
    sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
    sampDesc.MinLOD = 0;
    sampDesc.MaxLOD = D3D11_FLOAT32_MAX;

    if (FAILED(dev->CreateSamplerState(&sampDesc, samplerState.GetAddressOf()))) return 87;

    D3D11_BUFFER_DESC bd;
    ZeroMemory(&bd, sizeof(bd));

    bd.Usage = D3D11_USAGE_DYNAMIC;
    bd.ByteWidth = sizeof(planeVertices);
    bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

    if (FAILED(dev->CreateBuffer(&bd, NULL, pVBuffer.GetAddressOf()))) return 90;

    // copy the vertices into the buffer
    D3D11_MAPPED_SUBRESOURCE ms;
    devcon->Map(pVBuffer.Get(), NULL, D3D11_MAP_WRITE_DISCARD, NULL, &ms);    // map the buffer
    memcpy(ms.pData, planeVertices, sizeof(planeVertices));                 // copy the data
    devcon->Unmap(pVBuffer.Get(), NULL);

    // create the index buffer
    bd.Usage = D3D11_USAGE_DYNAMIC;
    bd.ByteWidth = sizeof(planeIndices);
    bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
    bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    //The resource is to be mappable so that the CPU can change its content
    //Resources created with this flag cannot be set as outputs of the pipeline.
    bd.MiscFlags = 0;

    if (FAILED(dev->CreateBuffer(&bd, NULL, pIBuffer.GetAddressOf()))) return 91;

    devcon->Map(pIBuffer.Get(), NULL, D3D11_MAP_WRITE_DISCARD, NULL, &ms);    // map the buffer
    memcpy(ms.pData, planeIndices, sizeof(planeIndices));                   // copy the data
    devcon->Unmap(pIBuffer.Get(), NULL);

    // set up constant buffer
    ZeroMemory(&bd, sizeof(bd));

    bd.Usage = D3D11_USAGE_DEFAULT;
    bd.ByteWidth = 64;
    bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;

    if (FAILED(dev->CreateBuffer(&bd, NULL, pCBuffer.GetAddressOf()))) return 92;
    return 0;
}

int Scene1::loadMesh()
{
    if (FAILED(dev->CreateVertexShader(VertexShaderBlob, std::size(VertexShaderBlob), NULL, pVS.GetAddressOf())))  return 87;
    if (FAILED(dev->CreatePixelShader(PixelShaderBlob, std::size(PixelShaderBlob), NULL, pPS.GetAddressOf())))  return 88;

    D3D11_INPUT_ELEMENT_DESC ied[] =
    {
        {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0},
    };

    if (FAILED(dev->CreateInputLayout(ied, 2, VertexShaderBlob, std::size(VertexShaderBlob), &pLayout))) return 89;

    D3D11_SAMPLER_DESC sampDesc;
    ZeroMemory(&sampDesc, sizeof(sampDesc));
    sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
    sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
    sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
    sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
    sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
    sampDesc.MinLOD = 0;
    sampDesc.MaxLOD = D3D11_FLOAT32_MAX;

    if (FAILED(dev->CreateSamplerState(&sampDesc, samplerState.GetAddressOf()))) return 87;

    D3D11_BUFFER_DESC bd;
    ZeroMemory(&bd, sizeof(bd));

    bd.Usage = D3D11_USAGE_DYNAMIC;
    bd.ByteWidth = sizeof(cubeVertices);
    bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

    if (FAILED(dev->CreateBuffer(&bd, NULL, pVBuffer.GetAddressOf()))) return 90;

    // copy the vertices into the buffer
    D3D11_MAPPED_SUBRESOURCE ms;
    devcon->Map(pVBuffer.Get(), NULL, D3D11_MAP_WRITE_DISCARD, NULL, &ms);    // map the buffer
    memcpy(ms.pData, cubeVertices, sizeof(cubeVertices));                 // copy the data
    devcon->Unmap(pVBuffer.Get(), NULL);

    // create the index buffer
    bd.Usage = D3D11_USAGE_DYNAMIC;
    bd.ByteWidth = sizeof(cubeIndices);
    bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
    bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    //The resource is to be mappable so that the CPU can change its content
    //Resources created with this flag cannot be set as outputs of the pipeline.
    bd.MiscFlags = 0;

    if (FAILED(dev->CreateBuffer(&bd, NULL, pIBuffer.GetAddressOf()))) return 91;

    devcon->Map(pIBuffer.Get(), NULL, D3D11_MAP_WRITE_DISCARD, NULL, &ms);    // map the buffer
    memcpy(ms.pData, cubeIndices, sizeof(cubeIndices));                   // copy the data
    devcon->Unmap(pIBuffer.Get(), NULL);

    // set up constant buffer
    ZeroMemory(&bd, sizeof(bd));

    bd.Usage = D3D11_USAGE_DEFAULT;
    bd.ByteWidth = 64;
    bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;

    if (FAILED(dev->CreateBuffer(&bd, NULL, pCBuffer.GetAddressOf()))) return 92;
    return 0;
}

Graphics::Graphics(const dx11device& dxObject, const GraphicsInfo& info)
{
    dev = dxObject.dev;
    devcon = dxObject.devcon;
    initDx(info);
    // should we also implement the class invariant here?
}

int Graphics::initDx(const GraphicsInfo& info)
{
    m_sHandle = info.m_streamHandle;

    // populate the handle as the key
    // setup a reference to target struct so we can create them 
    RenderTarget& target = renderstreamTarget[m_sHandle];

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

    if (FAILED(dev->CreateTexture2D(&rtDesc, nullptr, target.texture.GetAddressOf())))
        return 80;

    D3D11_RENDER_TARGET_VIEW_DESC rtvDesc;
    ZeroMemory(&rtvDesc, sizeof(D3D11_RENDER_TARGET_VIEW_DESC));
    rtvDesc.Format = rtDesc.Format;
    rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
    if (FAILED(dev->CreateRenderTargetView(target.texture.Get(), &rtvDesc, target.view.GetAddressOf())))
        return 81;

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
    dev.As(&dxgiDevice);
    Microsoft::WRL::ComPtr<IDXGIAdapter> dxgiAdapter;
    dxgiDevice->GetAdapter(dxgiAdapter.GetAddressOf());
    Microsoft::WRL::ComPtr<IDXGIFactory1> dxgiFactory;
    dxgiAdapter->GetParent(__uuidof(IDXGIFactory1), &dxgiFactory);

    HRESULT hr = dxgiFactory->CreateSwapChain(
        dev.Get(),
        &scd,
        &swapchain
    );
    dxgiDevice = nullptr;
    dxgiAdapter = nullptr;
    dxgiFactory = nullptr;
    if (FAILED(hr)) return 82;

    //Step3:create the back buffer for drawing the pixel to 
    ID3D11Texture2D* pBackBuffer;
    if (FAILED(swapchain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&pBackBuffer))) return 83;

    //Step4:creat renderTargetView, which is a way to access resource, bounded to the buffer resource
    if (FAILED(dev->CreateRenderTargetView(pBackBuffer, NULL, backbuffer.GetAddressOf()))) return 84;
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
    if (FAILED(dev->CreateTexture2D(&texd, NULL, &pDepthBuffer))) return 85;

    //Step6:create the depth stencil 
    D3D11_DEPTH_STENCIL_VIEW_DESC dsvd;
    ZeroMemory(&dsvd, sizeof(dsvd));
    dsvd.Format = DXGI_FORMAT_D32_FLOAT;
    dsvd.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DMS;
    if (FAILED(dev->CreateDepthStencilView(pDepthBuffer, &dsvd, depthStencilView.GetAddressOf()))) return 86;
    pDepthBuffer->Release();

    D3D11_VIEWPORT viewport;
    ZeroMemory(&viewport, sizeof(D3D11_VIEWPORT));
    viewport.Width = static_cast<float>(info.m_width);
    viewport.Height = static_cast<float>(info.m_height);
    viewport.MinDepth = 0;
    viewport.MaxDepth = 1;
    devcon->RSSetViewports(1, &viewport);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    ImGui_ImplWin32_Init(info.m_window);
    ImGui_ImplDX11_Init(dev.Get(), devcon.Get());
    ImGui::StyleColorsDark();

    return 0;
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

void Graphics::render(const DirectX::XMMATRIX matFinal)
{
    //depends on scene number, scene->render()
    ID3D11RenderTargetView* renderTargetList[2] = { backbuffer.Get(), renderstreamTarget[m_sHandle].view.Get() };
    devcon->OMSetRenderTargets(2, renderTargetList, nullptr);
    //devcon->OMSetRenderTargets(1, target.view.GetAddressOf(), nullptr);
    const float clearColour[4] = { 0.f, 0.f, 0.f, 0.f };
    for (int i = 0; i < 2; i++)
    {
        devcon->ClearRenderTargetView(renderTargetList[i], clearColour);
    }

    devcon->UpdateSubresource(pCBuffer.Get(), 0, nullptr, &matFinal, 0, 0);

    // Draw cube
    UINT stride = sizeof(DirectX::XMFLOAT3);
    UINT offset = 0;
    devcon->IASetVertexBuffers(0, 1, pVBuffer.GetAddressOf(), &stride, &offset);
    devcon->IASetIndexBuffer(pIBuffer.Get(), DXGI_FORMAT_R16_UINT, 0);
    devcon->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    devcon->IASetInputLayout(pLayout.Get());
    devcon->VSSetShader(pVS.Get(), nullptr, 0);
    devcon->VSSetConstantBuffers(0, 1, pCBuffer.GetAddressOf());
    devcon->PSSetShader(pPS.Get(), nullptr, 0);
    devcon->DrawIndexed(m_indiceCount, 0, 0);
}