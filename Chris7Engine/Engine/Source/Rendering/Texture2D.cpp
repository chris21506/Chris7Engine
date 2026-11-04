#include "Texture2D.h"
#include <new>

#define SAFE_RELEASE(x) if ((x) != nullptr) { (x)->Release(); (x) = nullptr; }

// ============================================================================
// ESTRUCTURA DE IMPLEMENTACIÓN PRIVADA (PIMPL)
// ============================================================================
struct Texture2D::Implementation {
    ID3D11Texture2D* texture = nullptr;
    ID3D11ShaderResourceView* srv = nullptr;
    std::uint32_t width = 0;
    std::uint32_t height = 0;
    DXGI_FORMAT format = DXGI_FORMAT_R8G8B8A8_UNORM;

    ~Implementation() {
        SAFE_RELEASE(srv);
        SAFE_RELEASE(texture);
    }
};

// ============================================================================
// CONSTRUCTORES Y DESTRUCTOR
// ============================================================================
Texture2D::Texture2D() noexcept
    : m_implementation(new(std::nothrow) Implementation()) {
}

Texture2D::~Texture2D() noexcept {
    Release();
    delete m_implementation;
    m_implementation = nullptr;
}

Texture2D::Texture2D(Texture2D&& other) noexcept
    : m_implementation(other.m_implementation) {
    other.m_implementation = nullptr;
}

Texture2D& Texture2D::operator=(Texture2D&& other) noexcept {
    if (this != &other) {
        Release();
        delete m_implementation;

        m_implementation = other.m_implementation;
        other.m_implementation = nullptr;
    }
    return *this;
}

// ============================================================================
// CREACIÓN Y GESTIÓN DE TEXTURA
// ============================================================================
bool Texture2D::Create(
    ID3D11Device* device,
    std::uint32_t width,
    std::uint32_t height,
    const void* pixelData,
    DXGI_FORMAT format
) noexcept {
    if (!m_implementation || !device || width == 0 || height == 0) {
        return false;
    }

    Release();

    m_implementation->width = width;
    m_implementation->height = height;
    m_implementation->format = format;

    // Configuración de la descripción de la textura
    D3D11_TEXTURE2D_DESC desc{};
    desc.Width = width;
    desc.Height = height;
    desc.MipLevels = 1;
    desc.ArraySize = 1;
    desc.Format = format;
    desc.SampleDesc.Count = 1;
    desc.SampleDesc.Quality = 0;
    desc.Usage = D3D11_USAGE_DEFAULT;
    desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
    desc.CPUAccessFlags = 0;
    desc.MiscFlags = 0;

    D3D11_SUBRESOURCE_DATA initData{};
    D3D11_SUBRESOURCE_DATA* pInitData = nullptr;

    if (pixelData != nullptr) {
        // Cálculo del pitch asumido para RGBA (4 bytes por píxel si es R8G8B8A8)
        std::uint32_t bytesPerPixel = 4;
        if (format == DXGI_FORMAT_R32G32B32A32_FLOAT) {
            bytesPerPixel = 16;
        }

        initData.pSysMem = pixelData;
        initData.SysMemPitch = width * bytesPerPixel;
        initData.SysMemSlicePitch = 0;
        pInitData = &initData;
    }

    // 1. Crear el objeto Texture2D de Direct3D
    HRESULT hr = device->CreateTexture2D(&desc, pInitData, &m_implementation->texture);
    if (FAILED(hr)) {
        return false;
    }

    // 2. Crear la vista del recurso de shader (Shader Resource View - SRV)
    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc{};
    srvDesc.Format = format;
    srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Texture2D.MostDetailedMip = 0;
    srvDesc.Texture2D.MipLevels = 1;

    hr = device->CreateShaderResourceView(m_implementation->texture, &srvDesc, &m_implementation->srv);
    if (FAILED(hr)) {
        Release();
        return false;
    }

    return true;
}

void Texture2D::Release() noexcept {
    if (m_implementation) {
        SAFE_RELEASE(m_implementation->srv);
        SAFE_RELEASE(m_implementation->texture);
        m_implementation->width = 0;
        m_implementation->height = 0;
    }
}

// ============================================================================
// GETTERS
// ============================================================================
ID3D11Texture2D* Texture2D::GetTexture() const noexcept {
    return m_implementation ? m_implementation->texture : nullptr;
}

ID3D11ShaderResourceView* Texture2D::GetShaderResourceView() const noexcept {
    return m_implementation ? m_implementation->srv : nullptr;
}

std::uint32_t Texture2D::GetWidth() const noexcept {
    return m_implementation ? m_implementation->width : 0;
}

std::uint32_t Texture2D::GetHeight() const noexcept {
    return m_implementation ? m_implementation->height : 0;
}