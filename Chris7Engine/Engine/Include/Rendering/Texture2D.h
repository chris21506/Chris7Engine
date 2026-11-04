#pragma once

#include <d3d11.h>
#include <cstdint>

class Texture2D final {
public:
    Texture2D() noexcept;
    ~Texture2D() noexcept;

    Texture2D(const Texture2D&) = delete;
    Texture2D& operator=(const Texture2D&) = delete;

    Texture2D(Texture2D&&) noexcept;
    Texture2D& operator=(Texture2D&&) noexcept;

    // Crear una textura desde memoria de píxeles en bruto (RGBA)
    bool Create(
        ID3D11Device* device,
        std::uint32_t width,
        std::uint32_t height,
        const void* pixelData = nullptr,
        DXGI_FORMAT format = DXGI_FORMAT_R8G8B8A8_UNORM
    ) noexcept;

    // Liberar recursos
    void Release() noexcept;

    [[nodiscard]] ID3D11Texture2D* GetTexture() const noexcept;
    [[nodiscard]] ID3D11ShaderResourceView* GetShaderResourceView() const noexcept;
    [[nodiscard]] std::uint32_t GetWidth() const noexcept;
    [[nodiscard]] std::uint32_t GetHeight() const noexcept;

private:
    struct Implementation;
    Implementation* m_implementation = nullptr;
};
