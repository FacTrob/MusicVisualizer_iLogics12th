#pragma once
#include <d3d11.h>
#include <DirectXMath.h>
#include <wrl/client.h>
#include <vector>

using namespace DirectX;
using Microsoft::WRL::ComPtr;

struct Vertex
{
    XMFLOAT3 position;
    XMFLOAT2 texCoord;
};

struct ConstantBuffer
{
    XMMATRIX world;
    XMMATRIX view;
    XMMATRIX projection;
    XMFLOAT4 color;
    float time;
    XMFLOAT3 padding;
};

class Renderer
{
public:
    Renderer();
    ~Renderer();

    bool Initialize(HWND hWnd, int width, int height);
    void Shutdown();

    void BeginFrame();
    void EndFrame();

    void SetBackgroundColor(float r, float g, float b);
    void DrawLines(const std::vector<Vertex>& vertices, const XMFLOAT4& color);
    void DrawLineStrip(const std::vector<Vertex>& vertices, const XMFLOAT4& color);
    void UpdateConstantBuffer(const ConstantBuffer& cb);

    ID3D11Device* GetDevice() const { return m_device.Get(); }
    ID3D11DeviceContext* GetContext() const { return m_context.Get(); }

private:
    bool CreateDeviceAndSwapChain(HWND hWnd, int width, int height);
    bool CreateRenderTargetView();
    bool CreateDepthStencilBuffer(int width, int height);
    bool LoadShaders();
    bool CreateConstantBuffer();
    bool CreateInputLayout();

    ComPtr<ID3D11Device> m_device;
    ComPtr<ID3D11DeviceContext> m_context;
    ComPtr<IDXGISwapChain> m_swapChain;
    ComPtr<ID3D11RenderTargetView> m_renderTargetView;
    ComPtr<ID3D11DepthStencilView> m_depthStencilView;
    ComPtr<ID3D11Texture2D> m_depthStencilBuffer;

    ComPtr<ID3D11VertexShader> m_vertexShader;
    ComPtr<ID3D11PixelShader> m_pixelShader;
    ComPtr<ID3D11InputLayout> m_inputLayout;
    ComPtr<ID3D11Buffer> m_constantBuffer;
    ComPtr<ID3D11Buffer> m_vertexBuffer;

    D3D11_VIEWPORT m_viewport;
    XMFLOAT4 m_backgroundColor;
    int m_width;
    int m_height;
};