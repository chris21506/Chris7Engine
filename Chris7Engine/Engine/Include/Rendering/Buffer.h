##pragma once

#include <d3d11.h>
#include <cstdint>

enum class BufferType {
    Vertex,
    Index,
    Constant
};

class Buffer final {
public:
    Buffer() noexcept;
    ~Buffer() noexcept;

    Buffer(const Buffer&) = delete;
    Buffer& operator=(const Buffer&) = delete;

    Buffer(Buffer&&) noexcept;
    Buffer& operator=(Buffer&&) noexcept;

    bool Create(
        ID3D11Device* device,
        BufferType type,
        const void* data,
        std::uint32_t sizeInBytes,
        std::uint32_t stride = 0
    ) noexcept;

    void Update(ID3D11DeviceContext* context, const void* data, std::uint32_t sizeInBytes) noexcept;
    void Release() noexcept;

    [[nodiscard]] ID3D11Buffer* GetBuffer() const noexcept;
    [[nodiscard]] std::uint32_t GetSize() const noexcept;
    [[nodiscard]] std::uint32_t GetStride() const noexcept;

private:
    struct Implementation;
    Implementation* m_implementation = nullptr;
};
