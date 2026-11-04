#include "DeviceContext.h"
#include <new>

#define SAFE_RELEASE(x) if ((x) != nullptr) { (x)->Release(); (x) = nullptr; }

// ============================================================================
// ESTRUCTURA DE IMPLEMENTACIÓN PRIVADA (PIMPL)
// ============================================================================
struct DeviceContext::Implementation {
    ID3D11DeviceContext* context = nullptr;
};

// ============================================================================
// CONSTRUCTORES Y DESTRUCTOR
// ============================================================================
DeviceContext::DeviceContext() noexcept
    : m_implementation(new(std::nothrow) Implementation()) {
}

DeviceContext::~DeviceContext() noexcept {
    Shutdown();
    delete m_implementation;
    m_implementation = nullptr;
}

DeviceContext::DeviceContext(DeviceContext&& other) noexcept
    : m_implementation(other.m_implementation) {
    other.m_implementation = nullptr;
}

DeviceContext& DeviceContext::operator=(DeviceContext&& other) noexcept {
    if (this != &other) {
        Shutdown();
        delete m_implementation;

        m_implementation = other.m_implementation;
        other.m_implementation = nullptr;
    }
    return *this;
}

// ============================================================================
// INICIALIZACIÓN Y GESTIÓN
// ============================================================================
bool DeviceContext::Initialize(ID3D11DeviceContext* context) noexcept {
    if (!m_implementation || !context) {
        return false;
    }

    Shutdown();

    m_implementation->context = context;
    m_implementation->context->AddRef(); // Incrementamos referencia COM
    return true;
}

void DeviceContext::Shutdown() noexcept {
    if (m_implementation && m_implementation->context) {
        m_implementation->context->ClearState();
        SAFE_RELEASE(m_implementation->context);
    }
}

// ============================================================================
// PIPELINE Y COMANDOS DE RENDER
// ============================================================================
void DeviceContext::SetViewport(float width, float height, float x, float y) noexcept {
    if (!m_implementation || !m_implementation->context) return;

    D3D11_VIEWPORT vp{};
    vp.TopLeftX = x;
    vp.TopLeftY = y;
    vp.Width = width;
    vp.Height = height;
    vp.MinDepth = 0.0f;
    vp.MaxDepth = 1.0f;

    m_implementation->context->RSSetViewports(1, &vp);
}

void DeviceContext::SetRenderTargets(UINT numViews, ID3D11RenderTargetView* const* renderTargetViews, ID3D11DepthStencilView* depthStencilView) noexcept {
    if (m_implementation && m_implementation->context) {
        m_implementation->context->OMSetRenderTargets(numViews, renderTargetViews, depthStencilView);
    }
}

void DeviceContext::ClearRenderTarget(ID3D11RenderTargetView* renderTargetView, const float colorRGBA[4]) noexcept {
    if (m_implementation && m_implementation->context && renderTargetView) {
        m_implementation->context->ClearRenderTargetView(renderTargetView, colorRGBA);
    }
}

void DeviceContext::SetVertexBuffer(ID3D11Buffer* buffer, UINT stride, UINT offset) noexcept {
    if (m_implementation && m_implementation->context) {
        m_implementation->context->IASetVertexBuffers(0, 1, &buffer, &stride, &offset);
    }
}

void DeviceContext::SetIndexBuffer(ID3D11Buffer* buffer, DXGI_FORMAT format, UINT offset) noexcept {
    if (m_implementation && m_implementation->context) {
        m_implementation->context->IASetIndexBuffer(buffer, format, offset);
    }
}

void DeviceContext::SetInputLayout(ID3D11InputLayout* layout) noexcept {
    if (m_implementation && m_implementation->context) {
        m_implementation->context->IASetInputLayout(layout);
    }
}

void DeviceContext::SetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY topology) noexcept {
    if (m_implementation && m_implementation->context) {
        m_implementation->context->IASetPrimitiveTopology(topology);
    }
}

void DeviceContext::SetVertexShader(ID3D11VertexShader* shader) noexcept {
    if (m_implementation && m_implementation->context) {
        m_implementation->context->VSSetShader(shader, nullptr, 0);
    }
}

void DeviceContext::SetPixelShader(ID3D11PixelShader* shader) noexcept {
    if (m_implementation && m_implementation->context) {
        m_implementation->context->PSSetShader(shader, nullptr, 0);
    }
}

void DeviceContext::SetConstantBufferVS(UINT startSlot, ID3D11Buffer* buffer) noexcept {
    if (m_implementation && m_implementation->context) {
        m_implementation->context->VSSetConstantBuffers(startSlot, 1, &buffer);
    }
}

void DeviceContext::SetConstantBufferPS(UINT startSlot, ID3D11Buffer* buffer) noexcept {
    if (m_implementation && m_implementation->context) {
        m_implementation->context->PSSetConstantBuffers(startSlot, 1, &buffer);
    }
}

void DeviceContext::Draw(UINT vertexCount, UINT startVertexLocation) noexcept {
    if (m_implementation && m_implementation->context) {
        m_implementation->context->Draw(vertexCount, startVertexLocation);
    }
}

void DeviceContext::DrawIndexed(UINT indexCount, UINT startIndexLocation, INT baseVertexLocation) noexcept {
    if (m_implementation && m_implementation->context) {
        m_implementation->context->DrawIndexed(indexCount, startIndexLocation, baseVertexLocation);
    }
}

// ============================================================================
// GETTERS
// ============================================================================
ID3D11DeviceContext* DeviceContext::GetNativeContext() const noexcept {
    return m_implementation ? m_implementation->context : nullptr;
}