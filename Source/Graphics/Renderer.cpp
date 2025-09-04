#include "Renderer.h"
#include <d3dcompiler.h>
#include <iostream>

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3dcompiler.lib")

Renderer::Renderer()
    : m_backgroundColor(0.0f, 0.0f, 0.0f, 1.0f), m_width(0), m_height(0)
{
}

Renderer::~Renderer()
{
    Shutdown();
}

bool Renderer::Initialize(HWND hWnd, int width, int height)
{
    m_width = width;
    m_height = height;

    if (!CreateDeviceAndSwapChain(hWnd, width, height))
        return false;

    if (!CreateRenderTargetView())
        return false;

    if (!CreateDepthStencilBuffer(width, height))
        return false;

    if (!LoadShaders())
        return false;

    if (!CreateInputLayout())
        return false;

    if (!CreateConstantBuffer())
        return false;

    // Setup viewport
    m_viewport.Width = static_cast<float>(width);
    m_viewport.Height = static_cast<float>(height);
    m_viewport.MinDepth = 0.0f;
    m_viewport.MaxDepth = 1.0f;
    m_viewport.TopLeftX = 0;
    m_viewport.TopLeftY = 0;

    m_context->RSSetViewports(1, &m_viewport);

    return true;
}

bool Renderer::CreateDeviceAndSwapChain(HWND hWnd, int width, int height)
{
    DXGI_SWAP_CHAIN_DESC swapChainDesc = {};
    swapChainDesc.BufferCount = 1;
    swapChainDesc.BufferDesc.Width = width;
    swapChainDesc.BufferDesc.Height = height;
    swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    swapChainDesc.BufferDesc.RefreshRate.Numerator = 60;
    swapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
    swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapChainDesc.OutputWindow = hWnd;
    swapChainDesc.SampleDesc.Count = 1;
    swapChainDesc.SampleDesc.Quality = 0;
    swapChainDesc.Windowed = TRUE;

    D3D_FEATURE_LEVEL featureLevel;
    HRESULT result = D3D11CreateDeviceAndSwapChain(
        nullptr,
        D3D_DRIVER_TYPE_HARDWARE,
        nullptr,
        0,
        nullptr,
        0,
        D3D11_SDK_VERSION,
        &swapChainDesc,
        &m_swapChain,
        &m_device,
        &featureLevel,
        &m_context
    );

    return SUCCEEDED(result);
}

bool Renderer::CreateRenderTargetView()
{
    ComPtr<ID3D11Texture2D> backBuffer;
    HRESULT result = m_swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&backBuffer);
    if (FAILED(result))
        return false;

    result = m_device->CreateRenderTargetView(backBuffer.Get(), nullptr, &m_renderTargetView);
    return SUCCEEDED(result);
}

bool Renderer::CreateDepthStencilBuffer(int width, int height)
{
    D3D11_TEXTURE2D_DESC depthBufferDesc = {};
    depthBufferDesc.Width = width;
    depthBufferDesc.Height = height;
    depthBufferDesc.MipLevels = 1;
    depthBufferDesc.ArraySize = 1;
    depthBufferDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    depthBufferDesc.SampleDesc.Count = 1;
    depthBufferDesc.SampleDesc.Quality = 0;
    depthBufferDesc.Usage = D3D11_USAGE_DEFAULT;
    depthBufferDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;

    HRESULT result = m_device->CreateTexture2D(&depthBufferDesc, nullptr, &m_depthStencilBuffer);
    if (FAILED(result))
        return false;

    result = m_device->CreateDepthStencilView(m_depthStencilBuffer.Get(), nullptr, &m_depthStencilView);
    return SUCCEEDED(result);
}

bool Renderer::LoadShaders()
{
    const char* vertexShaderSource =
        "cbuffer ConstantBuffer : register(b0) {"
        "    matrix World;"
        "    matrix View;"
        "    matrix Projection;"
        "    float4 Color;"
        "    float Time;"
        "    float3 Padding;"
        "}"
        "struct VS_INPUT {"
        "    float3 Pos : POSITION;"
        "    float2 Tex : TEXCOORD0;"
        "};"
        "struct VS_OUTPUT {"
        "    float4 Pos : SV_POSITION;"
        "    float2 Tex : TEXCOORD0;"
        "    float4 Color : COLOR;"
        "};"
        "VS_OUTPUT main(VS_INPUT input) {"
        "    VS_OUTPUT output = (VS_OUTPUT)0;"
        "    float4 worldPos = mul(float4(input.Pos, 1.0f), World);"
        "    float4 viewPos = mul(worldPos, View);"
        "    output.Pos = mul(viewPos, Projection);"
        "    output.Tex = input.Tex;"
        "    output.Color = Color;"
        "    return output;"
        "}";

    const char* pixelShaderSource =
        "struct PS_INPUT {"
        "    float4 Pos : SV_POSITION;"
        "    float2 Tex : TEXCOORD0;"
        "    float4 Color : COLOR;"
        "};"
        "float4 main(PS_INPUT input) : SV_Target {"
        "    return float4(1.0f, 1.0f, 1.0f, 1.0f);"
        "}";

    ComPtr<ID3DBlob> vsBlob, psBlob, errorBlob;

    // Compile vertex shader
    HRESULT result = D3DCompile(
        vertexShaderSource, strlen(vertexShaderSource), nullptr, nullptr, nullptr,
        "main", "vs_4_0", 0, 0, &vsBlob, &errorBlob
    );

    if (FAILED(result))
    {
        if (errorBlob)
        {
            std::cout << "Vertex shader compilation error: " << (char*)errorBlob->GetBufferPointer() << std::endl;
        }
        return false;
    }

    result = m_device->CreateVertexShader(vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), nullptr, &m_vertexShader);
    if (FAILED(result))
        return false;

    // Compile pixel shader
    result = D3DCompile(
        pixelShaderSource, strlen(pixelShaderSource), nullptr, nullptr, nullptr,
        "main", "ps_4_0", 0, 0, &psBlob, &errorBlob
    );

    if (FAILED(result))
    {
        if (errorBlob)
        {
            std::cout << "Pixel shader compilation error: " << (char*)errorBlob->GetBufferPointer() << std::endl;
        }
        return false;
    }

    result = m_device->CreatePixelShader(psBlob->GetBufferPointer(), psBlob->GetBufferSize(), nullptr, &m_pixelShader);
    return SUCCEEDED(result);
}

bool Renderer::CreateInputLayout()
{
    D3D11_INPUT_ELEMENT_DESC layout[] = {
        {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0}
    };

    const char* vertexShaderSource =
        "cbuffer ConstantBuffer : register(b0) {"
        "    matrix World; matrix View; matrix Projection; float4 Color; float Time; float3 Padding;"
        "}"
        "struct VS_INPUT { float3 Pos : POSITION; float2 Tex : TEXCOORD0; };"
        "struct VS_OUTPUT { float4 Pos : SV_POSITION; float2 Tex : TEXCOORD0; float4 Color : COLOR; };"
        "VS_OUTPUT main(VS_INPUT input) {"
        "    VS_OUTPUT output = (VS_OUTPUT)0;"
        "    output.Pos = mul(mul(mul(float4(input.Pos, 1.0f), World), View), Projection);"
        "    output.Tex = input.Tex; output.Color = Color; return output;"
        "}";

    ComPtr<ID3DBlob> vsBlob;
    HRESULT result = D3DCompile(vertexShaderSource, strlen(vertexShaderSource), nullptr, nullptr, nullptr,
        "main", "vs_4_0", 0, 0, &vsBlob, nullptr);

    if (FAILED(result))
        return false;

    result = m_device->CreateInputLayout(layout, 2, vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), &m_inputLayout);
    return SUCCEEDED(result);
}

bool Renderer::CreateConstantBuffer()
{
    D3D11_BUFFER_DESC bufferDesc = {};
    bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
    bufferDesc.ByteWidth = sizeof(ConstantBuffer);
    bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

    HRESULT result = m_device->CreateBuffer(&bufferDesc, nullptr, &m_constantBuffer);
    return SUCCEEDED(result);
}

void Renderer::BeginFrame()
{
    float clearColor[4] = { m_backgroundColor.x, m_backgroundColor.y, m_backgroundColor.z, m_backgroundColor.w };
    m_context->ClearRenderTargetView(m_renderTargetView.Get(), clearColor);
    m_context->ClearDepthStencilView(m_depthStencilView.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0);

    m_context->OMSetRenderTargets(1, m_renderTargetView.GetAddressOf(), m_depthStencilView.Get());
    m_context->RSSetViewports(1, &m_viewport);

    m_context->VSSetShader(m_vertexShader.Get(), nullptr, 0);
    m_context->PSSetShader(m_pixelShader.Get(), nullptr, 0);
    m_context->IASetInputLayout(m_inputLayout.Get());
}

void Renderer::EndFrame()
{
    m_swapChain->Present(1, 0);
}

void Renderer::SetBackgroundColor(float r, float g, float b)
{
    m_backgroundColor = XMFLOAT4(r, g, b, 1.0f);
}

void Renderer::DrawLines(const std::vector<Vertex>& vertices, const XMFLOAT4& color)
{
    if (vertices.empty()) return;

    // Update constant buffer with color
    ConstantBuffer cb = {};
    cb.world = XMMatrixIdentity();
    cb.view = XMMatrixIdentity();
    cb.projection = XMMatrixOrthographicLH(2.0f, 2.0f, 0.1f, 100.0f);
    cb.color = color;
    cb.time = 0.0f;

    UpdateConstantBuffer(cb);

    // Create/update vertex buffer
    D3D11_BUFFER_DESC bufferDesc = {};
    bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
    bufferDesc.ByteWidth = static_cast<UINT>(sizeof(Vertex) * vertices.size());
    bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

    ComPtr<ID3D11Buffer> vertexBuffer;
    HRESULT result = m_device->CreateBuffer(&bufferDesc, nullptr, &vertexBuffer);
    if (FAILED(result)) return;

    D3D11_MAPPED_SUBRESOURCE mappedResource;
    result = m_context->Map(vertexBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
    if (SUCCEEDED(result))
    {
        memcpy(mappedResource.pData, vertices.data(), sizeof(Vertex) * vertices.size());
        m_context->Unmap(vertexBuffer.Get(), 0);
    }

    UINT stride = sizeof(Vertex);
    UINT offset = 0;
    m_context->IASetVertexBuffers(0, 1, vertexBuffer.GetAddressOf(), &stride, &offset);
    m_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);
    m_context->Draw(static_cast<UINT>(vertices.size()), 0);
}

void Renderer::DrawLineStrip(const std::vector<Vertex>& vertices, const XMFLOAT4& color)
{
    if (vertices.empty()) return;

    ConstantBuffer cb = {};
    cb.world = XMMatrixIdentity();
    cb.view = XMMatrixIdentity();
    cb.projection = XMMatrixOrthographicLH(2.0f, 2.0f, 0.1f, 100.0f);
    cb.color = color;
    cb.time = 0.0f;

    UpdateConstantBuffer(cb);

    D3D11_BUFFER_DESC bufferDesc = {};
    bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
    bufferDesc.ByteWidth = static_cast<UINT>(sizeof(Vertex) * vertices.size());
    bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

    ComPtr<ID3D11Buffer> vertexBuffer;
    HRESULT result = m_device->CreateBuffer(&bufferDesc, nullptr, &vertexBuffer);
    if (FAILED(result)) return;

    D3D11_MAPPED_SUBRESOURCE mappedResource;
    result = m_context->Map(vertexBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
    if (SUCCEEDED(result))
    {
        memcpy(mappedResource.pData, vertices.data(), sizeof(Vertex) * vertices.size());
        m_context->Unmap(vertexBuffer.Get(), 0);
    }

    UINT stride = sizeof(Vertex);
    UINT offset = 0;
    m_context->IASetVertexBuffers(0, 1, vertexBuffer.GetAddressOf(), &stride, &offset);
    m_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP);
    m_context->Draw(static_cast<UINT>(vertices.size()), 0);
}

void Renderer::UpdateConstantBuffer(const ConstantBuffer& cb)
{
    D3D11_MAPPED_SUBRESOURCE mappedResource;
    HRESULT result = m_context->Map(m_constantBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
    if (SUCCEEDED(result))
    {
        memcpy(mappedResource.pData, &cb, sizeof(ConstantBuffer));
        m_context->Unmap(m_constantBuffer.Get(), 0);
    }

    m_context->VSSetConstantBuffers(0, 1, m_constantBuffer.GetAddressOf());
    m_context->PSSetConstantBuffers(0, 1, m_constantBuffer.GetAddressOf());
}

void Renderer::Shutdown()
{
    if (m_context) m_context->ClearState();
}