#pragma once

#include <d3d11.h>
#include <cstdint>

class DeviceContext final {
public:
    DeviceContext() noexcept;
    ~DeviceContext() noexcept;

    DeviceContext(const DeviceContext&) = delete;
    DeviceContext& operator=(const DeviceContext&) = delete;

    DeviceContext(DeviceContext&&) noexcept;
    DeviceContext& operator=(DeviceContext&&) noexcept;

    bool Initialize(ID3D11DeviceContext* context) noexcept;
    void Shutdown() noexcept;

    // Viewport
    void SetViewport(float width, float height, float x = 0.0f, float y = 0.0f) noexcept;

    // Output Merger
    void SetRenderTargets(UINT numViews, ID3D11RenderTargetView* const* renderTargetViews, ID3D11DepthStencilView* depthStencilView = nullptr) noexcept;
    void ClearRenderTarget(ID3D11RenderTargetView* renderTargetView, const float colorRGBA[4]) noexcept;

    // Input Assembler
    void SetVertexBuffer(ID3D11Buffer* buffer, UINT stride, UINT offset = 0) noexcept;
    void SetIndexBuffer(ID3D11Buffer* buffer, DXGI_FORMAT format = DXGI_FORMAT_R32_UINT, UINT offset = 0) noexcept;
    void SetInputLayout(ID3D11InputLayout* layout) noexcept;
    void SetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY topology) noexcept;

    // Shaders
    void SetVertexShader(ID3D11VertexShader* shader) noexcept;
    void SetPixelShader(ID3D11PixelShader* shader) noexcept;
    void SetConstantBufferVS(UINT startSlot, ID3D11Buffer* buffer) noexcept;
    void SetConstantBufferPS(UINT startSlot, ID3D11Buffer* buffer) noexcept;

    // Drawing
    void Draw(UINT vertexCount, UINT startVertexLocation = 0) noexcept;
    void DrawIndexed(UINT indexCount, UINT startIndexLocation = 0, INT baseVertexLocation = 0) noexcept;

    [[nodiscard]] ID3D11DeviceContext* GetNativeContext() const noexcept;

private:
    struct Implementation;
    Implementation* m_implementation = nullptr;
};
