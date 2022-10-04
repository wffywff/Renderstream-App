#include "graphics.hpp"
#include <stdexcept>
#include "gui.hpp"

HRESULT Graphics::makeDxDevice()
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
    HRESULT hr = D3D11CreateDevice(
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
    return hr;
}

int Graphics::initDx(HWND window, const StreamDescription& description)
{
    //Step2: create swapchain
    DXGI_SWAP_CHAIN_DESC scd;
    ZeroMemory(&scd, sizeof(DXGI_SWAP_CHAIN_DESC));

    scd.BufferCount = 2;                                   // 2 for tradtional double-buffered behavior to avoid tearing. Set it to 3 if graphic content takes more  than one monitor refresh cycle to render single frame (60hz for example)
    scd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;    // use 32-bit color
    scd.BufferDesc.Width = description.width;                   // set the back buffer width
    scd.BufferDesc.Height = description.height;                 // set the back buffer height
    scd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;     // indicates that the swap chain will be a drawing surface, allowing to use it as a D3d render-target
    scd.SwapEffect = DXGI_SWAP_EFFECT_SEQUENTIAL;
    scd.OutputWindow = window;                               // the window to be used
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

    texd.Width = description.width;
    texd.Height = description.height;
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

    //Step7/8:create shaders base off the shader.h's binary blob 
    if (FAILED(dev->CreateVertexShader(VertexShaderBlob, std::size(VertexShaderBlob), NULL, pVS.GetAddressOf())))  return 87;
    if (FAILED(dev->CreatePixelShader(PixelShaderBlob, std::size(PixelShaderBlob), NULL, pPS.GetAddressOf())))  return 88;

    //Step9:create the input layout objectzz
    D3D11_INPUT_ELEMENT_DESC ied[] =
    {
        {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0},
    };

    if (FAILED(dev->CreateInputLayout(ied, 2, VertexShaderBlob, std::size(VertexShaderBlob), &pLayout))) return 89;

    //Step10: set up Vertex data and Index data and constant buffer
    //create buffer for each and map them 

    //static constexpr so that it is accessible across program and processed in compile time
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

    // create the vertex buffer
    D3D11_BUFFER_DESC bd;
    ZeroMemory(&bd, sizeof(bd));

    bd.Usage = D3D11_USAGE_DYNAMIC;
    bd.ByteWidth = sizeof(VERTEX) * 8;
    bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

    if (FAILED(dev->CreateBuffer(&bd, NULL, pVBuffer.GetAddressOf()))) return 90;

    // copy the vertices into the buffer
    D3D11_MAPPED_SUBRESOURCE ms;
    devcon->Map(pVBuffer.Get(), NULL, D3D11_MAP_WRITE_DISCARD, NULL, &ms);    // map the buffer
    memcpy(ms.pData, cubeVertices, sizeof(cubeVertices));                 // copy the data
    devcon->Unmap(pVBuffer.Get(), NULL);

    // create the index buffer out of DWORDs
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

    // create the index buffer
    bd.Usage = D3D11_USAGE_DYNAMIC;
    bd.ByteWidth = sizeof(uint16_t) * 36;
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

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    ImGui_ImplWin32_Init(window);
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