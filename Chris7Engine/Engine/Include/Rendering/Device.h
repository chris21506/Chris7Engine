#pragma once

#include <d3d11.h>
#include <cstdint>

class Device final {
public:
    Device() noexcept;
    ~Device() noexcept;

    Device(const Device&) = delete;
    Device& operator=(const Device&) = delete;

    Device(Device&&) noexcept;
    Device& operator=(Device&&) noexcept;

    bool Initialize(HWND windowHandle, std::uint32_t width, std::uint32_t height) noexcept;
    void ClearRenderTarget(float r, float g, float b, float a) noexcept;
    void Present(bool vsync = true) noexcept;
    void Resize(std::uint32_t width, std::uint32_t height) noexcept;
    void Shutdown() noexcept;

    [[nodiscard]] ID3D11Device* GetDevice() const noexcept;
    [[nodiscard]] ID3D11DeviceContext* GetContext() const noexcept;
    [[nodiscard]] ID3D11RenderTargetView* GetRenderTargetView() const noexcept;

private:
    struct Implementation;
    Implementation* m_implementation = nullptr;
};
