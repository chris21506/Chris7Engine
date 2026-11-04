#include "Device.h"
#include <new>
#include <sstream>

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")

#define SAFE_RELEASE(x) if ((x) != nullptr) { (x)->Release(); (x) = nullptr; }

// ============================================================================
// ESTRUCTURA DE IMPLEMENTACIÓN PRIVADA (PIMPL)
// ============================================================================
struct Device::Implementation {
    HWND windowHandle = nullptr;
    std::uint32_t width = 0;
    std::uint32_t height = 0;

    ID3D11Device* device = nullptr;
    ID3D11DeviceContext* context = nullptr;
    IDXGISwapChain* swapChain = nullptr;
    ID3D11RenderTargetView* renderTargetView = nullptr;

    bool CreateRenderTarget() noexcept {
        ID3D11Texture2D* backBuffer = nullptr;
        HRESULT hr = swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&backBuffer));
        if (FAILED(hr)) {
            return false;
        }

        hr = device->CreateRenderTargetView(backBuffer, nullptr, &renderTargetView);
        SAFE_RELEASE(backBuffer);

        return SUCCEEDED(hr);
    }

    void SetViewport(std::uint32_t newWidth, std::uint32_t newHeight) noexcept {
        width = newWidth;
        height = newHeight;

        D3D11_VIEWPORT vp{};
        vp.TopLeftX = 0.0f;
        vp.TopLeftY = 0.0f;
        vp.Width = static_cast<float>(width);
        vp.Height = static_cast<float>(height);
        vp.MinDepth = 0.0f;
        vp.MaxDepth = 1.0f;

        if (context) {
            context->RSSetViewports(1, &vp);
        }
    }
};

// ============================================================================
// CONSTRUCTORES Y DESTRUCTOR
// ============================================================================
Device::Device() noexcept
    : m_implementation(new(std::nothrow) Implementation()) {
}

Device::~Device() noexcept {
    Shutdown();
    delete m_implementation;
    m_implementation = nullptr;
}

Device::Device(Device&& other) noexcept
    : m_implementation(other.m_implementation) {
    other.m_implementation = nullptr;
}

Device& Device::operator=(Device&& other) noexcept {
    if (this != &other) {
        Shutdown();
        delete m_implementation;

        m_implementation = other.m_implementation;
        other.m_implementation = nullptr;
    }
    return *this;
}

// ============================================================================
// MÉTODOS DE INICIALIZACIÓN Y GESTIÓN
// ============================================================================
bool Device::Initialize(HWND windowHandle, std::uint32_t width, std::uint32_t height) noexcept {
    if (!m_implementation || !windowHandle || width == 0 || height == 0) {
        return false;
    }

    m_implementation->windowHandle = windowHandle;
    m_implementation->width = width;
    m_implementation->height = height;

    DXGI_SWAP_CHAIN_DESC scd{};
    scd.BufferCount = 1;
    scd.BufferDesc.Width = width;
    scd.BufferDesc.Height = height;
    scd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    scd.BufferDesc.RefreshRate.Numerator = 60;
    scd.BufferDesc.RefreshRate.Denominator = 1;
    scd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    scd.OutputWindow = windowHandle;
    scd.SampleDesc.Count = 1;
    scd.SampleDesc.Quality = 0;
    scd.Windowed = TRUE;

    UINT flags = 0;
#if defined(_DEBUG)
    flags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

    D3D_FEATURE_LEVEL featureLevels[] = {
        D3D_FEATURE_LEVEL_11_1,
        D3D_FEATURE_LEVEL_11_0,
    };
    D3D_FEATURE_LEVEL selectedFeatureLevel;

    HRESULT hr = D3D11CreateDeviceAndSwapChain(
        nullptr,
        D3D_DRIVER_TYPE_HARDWARE,
        nullptr,
        flags,
        featureLevels,
        ARRAYSIZE(featureLevels),
        D3D11_SDK_VERSION,
        &scd,
        &m_implementation->swapChain,
        &m_implementation->device,
        &selectedFeatureLevel,
        &m_implementation->context
    );

    if (FAILED(hr)) {
        return false;
    }

    if (!m_implementation->CreateRenderTarget()) {
        Shutdown();
        return false;
    }

    m_implementation->SetViewport(width, height);
    return true;
}

void Device::ClearRenderTarget(float r, float g, float b, float a) noexcept {
    if (!m_implementation || !m_implementation->context || !m_implementation->renderTargetView) {
        return;
    }

    const float color[4] = { r, g, b, a };
    m_implementation->context->ClearRenderTargetView(m_implementation->renderTargetView, color);
    m_implementation->context->OMSetRenderTargets(1, &m_implementation->renderTargetView, nullptr);
}

void Device::Present(bool vsync) noexcept {
    if (m_implementation && m_implementation->swapChain) {
        m_implementation->swapChain->Present(vsync ? 1 : 0, 0);
    }
}

void Device::Resize(std::uint32_t width, std::uint32_t height) noexcept {
    if (!m_implementation || !m_implementation->swapChain || width == 0 || height == 0) {
        return;
    }

    m_implementation->context->OMSetRenderTargets(0, nullptr, nullptr);
    SAFE_RELEASE(m_implementation->renderTargetView);

    HRESULT hr = m_implementation->swapChain->ResizeBuffers(0, width, height, DXGI_FORMAT_UNKNOWN, 0);
    if (SUCCEEDED(hr)) {
        m_implementation->CreateRenderTarget();
        m_implementation->SetViewport(width, height);
    }
}

void Device::Shutdown() noexcept {
    if (!m_implementation) {
        return;
    }

    SAFE_RELEASE(m_implementation->renderTargetView);
    SAFE_RELEASE(m_implementation->swapChain);

    if (m_implementation->context) {
        m_implementation->context->ClearState();
    }
    SAFE_RELEASE(m_implementation->context);
    SAFE_RELEASE(m_implementation->device);

    m_implementation->windowHandle = nullptr;
    m_implementation->width = 0;
    m_implementation->height = 0;
}

// ============================================================================
// GETTERS
// ============================================================================
ID3D11Device* Device::GetDevice() const noexcept {
    return m_implementation ? m_implementation->device : nullptr;
}

ID3D11DeviceContext* Device::GetContext() const noexcept {
    return m_implementation ? m_implementation->context : nullptr;
}

ID3D11RenderTargetView* Device::GetRenderTargetView() const noexcept {
    return m_implementation ? m_implementation->renderTargetView : nullptr;
}