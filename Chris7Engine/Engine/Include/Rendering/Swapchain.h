#pragma once

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#ifndef NOMINMAX
#define NOMINMAX
#endif

#include <Windows.h>
#include <d3d11.h>
#include <cstdint>

class SwapChain final {
public:
    SwapChain() noexcept;
    ~SwapChain() noexcept;

    // Prevenir copia
    SwapChain(const SwapChain&) = delete;
    SwapChain& operator=(const SwapChain&) = delete;

    // Permitir movimiento
    SwapChain(SwapChain&&) noexcept;
    SwapChain& operator=(SwapChain&&) noexcept;

    /**
     * @brief Inicializa la cadena de intercambio (Swap Chain) y crea su Render Target View principal.
     * @param device Dispositivo de Direct3D 11 con el cual crear la SwapChain.
     * @param windowHandle Identificador de la ventana de Windows (HWND).
     * @param width Ancho inicial del búfer.
     * @param height Alto inicial del búfer.
     * @return true si la creación fue exitosa, false en caso contrario.
     */
    bool Initialize(
        ID3D11Device* device,
        HWND windowHandle,
        std::uint32_t width,
        std::uint32_t height
    ) noexcept;

    /**
     * @brief Presenta el búfer trasero (Back Buffer) en la pantalla.
     * @param vsync true para sincronización vertical (60Hz), false para renderizado sin límite.
     */
    void Present(bool vsync = true) noexcept;

    /**
     * @brief Redimensiona los búferes internos cuando la ventana cambia de tamaño.
     * @param device Contexto o dispositivo de DX11 necesario para re-crear el Render Target View.
     * @param context Contexto de dispositivo para desvincular los render targets activos.
     * @param width Nuevo ancho.
     * @param height Nuevo alto.
     * @return true si el reajuste fue exitoso.
     */
    bool Resize(
        ID3D11Device* device,
        ID3D11DeviceContext* context,
        std::uint32_t width,
        std::uint32_t height
    ) noexcept;

    /**
     * @brief Libera los recursos nativos de DXGI y Direct3D 11.
     */
    void Shutdown() noexcept;

    // Getters
    [[nodiscard]] IDXGISwapChain* GetNativeSwapChain() const noexcept;
    [[nodiscard]] ID3D11RenderTargetView* GetRenderTargetView() const noexcept;
    [[nodiscard]] std::uint32_t GetWidth() const noexcept;
    [[nodiscard]] std::uint32_t GetHeight() const noexcept;

private:
    struct Implementation;
    Implementation* m_implementation = nullptr;
};
