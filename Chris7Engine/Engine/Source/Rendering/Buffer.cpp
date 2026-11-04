#include "Buffer.h"
#include <new>

// Macro de utilidad para liberación segura de interfaces COM
#define SAFE_RELEASE(x) if ((x) != nullptr) { (x)->Release(); (x) = nullptr; }

// ============================================================================
// ESTRUCTURA PRIVADA (Pimpl)
// ============================================================================
struct Buffer::Implementation {
    ID3D11Buffer* buffer = nullptr;
    BufferType type = BufferType::Vertex;
    std::uint32_t sizeInBytes = 0;
    std::uint32_t stride = 0;

    ~Implementation() {
        SAFE_RELEASE(buffer);
    }
};

// ============================================================================
// CONSTRUCTORES Y DESTRUCTOR
// ============================================================================
Buffer::Buffer() noexcept
    : m_implementation(new(std::nothrow) Implementation()) {
}

Buffer::~Buffer() noexcept {
    Release();
    delete m_implementation;
    m_implementation = nullptr;
}

// Semántica de movimiento (Move Constructor)
Buffer::Buffer(Buffer&& other) noexcept
    : m_implementation(other.m_implementation) {
    other.m_implementation = nullptr;
}

Buffer& Buffer::operator=(Buffer&& other) noexcept {
    if (this != &other) {
        Release();
        delete m_implementation;

        m_implementation = other.m_implementation;
        other.m_implementation = nullptr;
    }
    return *this;
}

// ============================================================================
// MÉTODOS DE LA CLASE
// ============================================================================
bool Buffer::Create(
    ID3D11Device* device,
    BufferType type,
    const void* data,
    std::uint32_t sizeInBytes,
    std::uint32_t stride
) noexcept {
    if (!m_implementation || !device || sizeInBytes == 0) {
        return false;
    }

    // Asegurar que liberamos si re-creamos sobre el mismo objeto
    Release();

    m_implementation->type = type;
    m_implementation->sizeInBytes = sizeInBytes;
    m_implementation->stride = stride;

    D3D11_BUFFER_DESC desc{};
    desc.ByteWidth = sizeInBytes;

    // Asignación de banderas según el tipo de Buffer
    switch (type) {
    case BufferType::Vertex:
        desc.Usage = D3D11_USAGE_DEFAULT;
        desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
        desc.CPUAccessFlags = 0;
        break;

    case BufferType::Index:
        desc.Usage = D3D11_USAGE_DEFAULT;
        desc.BindFlags = D3D11_BIND_INDEX_BUFFER;
        desc.CPUAccessFlags = 0;
        break;

    case BufferType::Constant:
        // Los Constant Buffers requieren ser múltiplos de 16 bytes en DX11
        desc.ByteWidth = (sizeInBytes + 15) & ~15;
        desc.Usage = D3D11_USAGE_DYNAMIC;
        desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
        desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
        break;
    }

    D3D11_SUBRESOURCE_DATA initData{};
    D3D11_SUBRESOURCE_DATA* pInitData = nullptr;

    if (data != nullptr) {
        initData.pSysMem = data;
        pInitData = &initData;
    }

    HRESULT hr = device->CreateBuffer(&desc, pInitData, &m_implementation->buffer);
    return SUCCEEDED(hr);
}

void Buffer::Update(ID3D11DeviceContext* context, const void* data, std::uint32_t sizeInBytes) noexcept {
    if (!m_implementation || !m_implementation->buffer || !context || !data) {
        return;
    }

    if (m_implementation->type == BufferType::Constant) {
        // Actualización rápida en GPU mediante Map/Unmap para Constant Buffers
        D3D11_MAPPED_SUBRESOURCE mappedResource{};
        HRESULT hr = context->Map(m_implementation->buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
        if (SUCCEEDED(hr)) {
            const std::uint32_t bytesToCopy = (sizeInBytes < m_implementation->sizeInBytes)
                ? sizeInBytes
                : m_implementation->sizeInBytes;

            memcpy(mappedResource.pData, data, bytesToCopy);
            context->Unmap(m_implementation->buffer, 0);
        }
    }
    else {
        // Actualización para Vertex/Index buffers por UpdateSubresource
        context->UpdateSubresource(m_implementation->buffer, 0, nullptr, data, 0, 0);
    }
}

void Buffer::Release() noexcept {
    if (m_implementation && m_implementation->buffer) {
        SAFE_RELEASE(m_implementation->buffer);
        m_implementation->sizeInBytes = 0;
        m_implementation->stride = 0;
    }
}

// ============================================================================
// GETTERS
// ============================================================================
ID3D11Buffer* Buffer::GetBuffer() const noexcept {
    return m_implementation ? m_implementation->buffer : nullptr;
}

std::uint32_t Buffer::GetSize() const noexcept {
    return m_implementation ? m_implementation->sizeInBytes : 0;
}

std::uint32_t Buffer::GetStride() const noexcept {
    return m_implementation ? m_implementation->stride : 0;
}