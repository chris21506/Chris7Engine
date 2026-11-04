#include <Engine/Engine.h>
#include <Window.h>

#include <d3d11.h>
#include <d3dcompiler.h>

#include <cstddef>
#include <cstdint>
#include <new>
#include <sstream>
#include <string>

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "d3dcompiler.lib")

// ============================================================================
// MACROS CORREGIDAS
// ============================================================================
#define SAFE_RELEASE(x) if ((x) != nullptr) { (x)->Release(); (x) = nullptr; }

#define LOG_MESSAGE(classObj, method, state)                      \
{                                                                 \
    std::wostringstream os_;                                      \
    os_ << classObj << L"::" << method << L" : "                 \
        << L"[CREATION OF RESOURCE : " << state << L"]\n";        \
    OutputDebugStringW(os_.str().c_str());                        \
}

#define LOG_ERROR(classObj, method, errorMSG)                     \
{                                                                 \
    try {                                                         \
        std::wostringstream os_;                                  \
        os_ << L"ERROR : " << classObj << L"::" << method          \
           << L" : " << errorMSG << L"\n";                       \
        OutputDebugStringW(os_.str().c_str());                    \
    } catch (...) {                                               \
        OutputDebugStringW(L"Failed to log error message.\n");    \
    }                                                             \
}

// ============================================================================
// ESTRUCTURA DE IMPLEMENTACIÓN (Pimpl)
// ============================================================================
struct Engine::Implementation {
    struct Vertex {
        float position[3];
        float color[4];
    };

    HWND window = nullptr;
    std::uint32_t width = 0;
    std::uint32_t height = 0;

    // Recursos de DirectX 11
    ID3D11Device* device = nullptr;
    ID3D11DeviceContext* context = nullptr;
    IDXGISwapChain* swapChain = nullptr;
    ID3D11RenderTargetView* renderTarget = nullptr;

    // Visual Pipeline
    ID3D11VertexShader* vertexShader = nullptr;
    ID3D11PixelShader* pixelShader = nullptr;
    ID3D11InputLayout* inputLayout = nullptr;
    ID3D11Buffer* vertexBuffer = nullptr;
};

// Instancia global estática para el C-API Wrapper
static Engine g_engineInstance;

// ============================================================================
// MÉTODOS DE LA CLASE Engine
// ============================================================================
Engine::Engine() noexcept
    : m_implementation(new(std::nothrow) Implementation()) {
}

Engine::~Engine() noexcept {
    Shutdown();
    delete m_implementation;
    m_implementation = nullptr;
}

bool Engine::Initialize(void* nativeWindow, std::uint32_t width, std::uint32_t height) noexcept {
    if (!m_implementation) return false;

    m_implementation->window = static_cast<HWND>(nativeWindow);
    m_implementation->width = width;
    m_implementation->height = height;

    if (!m_implementation->window) {
        LOG_ERROR(L"Engine", L"Initialize", L"HWND nulo proporcionado.");
        return false;
    }

    // 1. Crear SwapChain, Device y Context
    DXGI_SWAP_CHAIN_DESC scd = {};
    scd.BufferCount = 1;
    scd.BufferDesc.Width = width;
    scd.BufferDesc.Height = height;
    scd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    scd.BufferAquí tienes el archivo** `Engine.cpp`** corregido y completamente funcional en C++11 / DirectX 11.

        Se arreglaron varios errores de sintaxis y tipado presentes en el fragmento original :
    1. * *Tipos e indicadores de punteros en DirectX : **Se agregaron las estrellas `*` faltantes a los punteros de COM(`ID3D11Device*`, `IDXGISwapChain*`, etc.).
        2. * *Macros de depuración : **Se corrigió la macro `SAFE_RELEASE` (había un `IF` en mayúsculas y faltaba un bloque `{
}`) y la macro `MESSAGE` (que intentaba usar `std::_Add_pointer` en lugar de `std::wostringstream`).
    3. * *Carga y compilación de shaders : **Se implementó la compilación runtime de `shaders\Triangle.hlsl` para generar el triángulo que el `wWinMain` espera validar.
    4. * *Bucle de renderizado Direct3D 11:**Implementación completa del * Viewport*, * ClearRenderTargetView*, * Draw* y* Present* .

    ```cpp
#include <Engine/Engine.h>
#include <Window.h>

#include <d3d11.h>
#include <d3dcompiler.h>

#include <cstddef>
#include <cstdint>
#include <new>
#include <sstream>
#include <string>

    // Vinculación de librerías de DirectX 11
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "d3dcompiler.lib")

// ============================================================================
// MACROS DE UTILIDAD Y REGISTRO DE ERRORES
// ============================================================================
#define SAFE_RELEASE(x) if ((x) != nullptr) { (x)->Release(); (x) = nullptr; }

#define LOG_MESSAGE(classObj, method, state)                          \
{                                                                     \
    std::wostringstream os_;                                          \
    os_ << classObj << L"::" << method << L" : [RESOURCE "            \
       << state << L"]\n";                                            \
    OutputDebugStringW(os_.str().c_str());                            \
}

#define LOG_ERROR(classObj, method, errorMSG)                         \
{                                                                     \
    try {                                                             \
        std::wostringstream os_;                                      \
        os_ << L"ERROR : " << classObj << L"::" << method             \
           << L" : " << errorMSG << L"\n";                            \
        OutputDebugStringW(os_.str().c_str());                        \
    } catch (...) {                                                   \
        OutputDebugStringW(L"Failed to log error message.\n");        \
    }                                                                 \
}

// ============================================================================
// ESTRUCTURA DE IMPLEMENTACIÓN PRIVADA (PIMPL)
// ============================================================================
struct Engine::Implementation {
        struct Vertex {
            float position[3];
            float color[4];
        };

        HWND window = nullptr;
        std::uint32_t width = 0;
        std::uint32_t height = 0;

        // Recursos Principales de Direct3D 11
        ID3D11Device* device = nullptr;
        ID3D11DeviceContext* context = nullptr;
        IDXGISwapChain* swapChain = nullptr;
        ID3D11RenderTargetView* renderTarget = nullptr;

        // Pipeline de Renderizado
        ID3D11VertexShader* vertexShader = nullptr;
        ID3D11PixelShader* pixelShader = nullptr;
        ID3D11InputLayout* inputLayout = nullptr;
        ID3D11Buffer* vertexBuffer = nullptr;

        bool isInitialized = false;

        // Método auxiliar para compilar y cargar el shader HLSL
        bool InitializePipeline() noexcept {
            ID3DBlob* vsBlob = nullptr;
            ID3DBlob* psBlob = nullptr;
            ID3DBlob* errorBlob = nullptr;

            // 1. Compilar Vertex Shader desde shaders/Triangle.hlsl
            HRESULT hr = D3DCompileFromFile(
                L"shaders\\Triangle.hlsl",
                nullptr,
                D3D_COMPILE_STANDARD_INCLUDES,
                "VSMain",
                "vs_5_0",
                0, 0,
                &vsBlob,
                &errorBlob
            );

            if (FAILED(hr)) {
                if (errorBlob) {
                    OutputDebugStringA(static_cast<char*>(errorBlob->GetBufferPointer()));
                    SAFE_RELEASE(errorBlob);
                }
                LOG_ERROR(L"Engine", L"InitializePipeline", L"Fallo al compilar Vertex Shader (shaders\\Triangle.hlsl)");
                return false;
            }

            // 2. Crear Vertex Shader
            hr = device->CreateVertexShader(vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), nullptr, &vertexShader);
            if (FAILED(hr)) {
                SAFE_RELEASE(vsBlob);
                LOG_ERROR(L"Engine", L"InitializePipeline", L"Fallo al crear Vertex Shader");
                return false;
            }

            // 3. Compilar Pixel Shader
            hr = D3DCompileFromFile(
                L"shaders\\Triangle.hlsl",
                nullptr,
                D3D_COMPILE_STANDARD_INCLUDES,
                "PSMain",
                "ps_5_0",
                0, 0,
                &psBlob,
                &errorBlob
            );

            if (FAILED(hr)) {
                if (errorBlob) {
                    OutputDebugStringA(static_cast<char*>(errorBlob->GetBufferPointer()));
                    SAFE_RELEASE(errorBlob);
                }
                SAFE_RELEASE(vsBlob);
                LOG_ERROR(L"Engine", L"InitializePipeline", L"Fallo al compilar Pixel Shader");
                return false;
            }

            // 4. Crear Pixel Shader
            hr = device->CreatePixelShader(psBlob->GetBufferPointer(), psBlob->GetBufferSize(), nullptr, &pixelShader);
            SAFE_RELEASE(psBlob);
            if (FAILED(hr)) {
                SAFE_RELEASE(vsBlob);
                LOG_ERROR(L"Engine", L"InitializePipeline", L"Fallo al crear Pixel Shader");
                return false;
            }

            // 5. Definir e instanciar Input Layout
            D3D11_INPUT_ELEMENT_DESC layoutDesc[] = {
                { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(Vertex, position), D3D11_INPUT_PER_VERTEX_DATA, 0 },
                { "COLOR",    0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, offsetof(Vertex, color),    D3D11_INPUT_PER_VERTEX_DATA, 0 }
            };

            hr = device->CreateInputLayout(
                layoutDesc,
                ARRAYSIZE(layoutDesc),
                vsBlob->GetBufferPointer(),
                vsBlob->GetBufferSize(),
                &inputLayout
            );
            SAFE_RELEASE(vsBlob);

            if (FAILED(hr)) {
                LOG_ERROR(L"Engine", L"InitializePipeline", L"Fallo al crear Input Layout");
                return false;
            }

            // 6. Geometría de un Triángulo
            Vertex vertices[] = {
                { {  0.0f,  0.5f, 0.0f }, { 1.0f, 0.0f, 0.0f, 1.0f } }, // Arriba (Rojo)
                { {  0.5f, -0.5f, 0.0f }, { 0.0f, 1.0f, 0.0f, 1.0f } }, // Derecha (Verde)
                { { -0.5f, -0.5f, 0.0f }, { 0.0f, 0.0f, 1.0f, 1.0f } }  // Izquierda (Azul)
            };

            D3D11_BUFFER_DESC bufferDesc{};
            bufferDesc.Usage = D3D11_USAGE_DEFAULT;
            bufferDesc.ByteWidth = sizeof(vertices);
            bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

            D3D11_SUBRESOURCE_DATA initData{};
            initData.pSysMem = vertices;

            hr = device->CreateBuffer(&bufferDesc, &initData, &vertexBuffer);
            if (FAILED(hr)) {
                LOG_ERROR(L"Engine", L"InitializePipeline", L"Fallo al crear Vertex Buffer");
                return false;
            }

            return true;
        }
    };

    // Instancia global estática para el soporte C-API si se requiere exportar funciones extern "C"
    static Engine g_engineInstance;

    // ============================================================================
    // MÉTODOS DE LA CLASE ENGINE
    // ============================================================================
    Engine::Engine() noexcept
        : m_implementation(new(std::nothrow) Implementation()) {
    }

    Engine::~Engine() noexcept {
        Shutdown();
        delete m_implementation;
        m_implementation = nullptr;
    }

    bool Engine::Initialize(void* nativeWindow, std::uint32_t width, std::uint32_t height) noexcept {
        if (!m_implementation) {
            return false;
        }

        if (m_implementation->isInitialized) {
            return true;
        }

        m_implementation->window = static_cast<HWND>(nativeWindow);
        m_implementation->width = width;
        m_implementation->height = height;

        if (!m_implementation->window) {
            LOG_ERROR(L"Engine", L"Initialize", L"HWND nulo proporcionado.");
            return false;
        }

        // Configuración del Swap Chain
        DXGI_SWAP_CHAIN_DESC scd{};
        scd.BufferCount = 1;
        scd.BufferDesc.Width = m_implementation->width;
        scd.BufferDesc.Height = m_implementation->height;
        scd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        scd.BufferDesc.RefreshRate.Numerator = 60;
        scd.BufferDesc.RefreshRate.Denominator = 1;
        scd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        scd.OutputWindow = m_implementation->window;
        scd.SampleDesc.Count = 1;
        scd.SampleDesc.Quality = 0;
        scd.Windowed = TRUE;

        UINT createDeviceFlags = 0;
#if defined(_DEBUG)
        createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

        // Crear Dispositivo y SwapChain de Direct3D
        HRESULT hr = D3D11CreateDeviceAndSwapChain(
            nullptr,
            D3D_DRIVER_TYPE_HARDWARE,
            nullptr,
            createDeviceFlags,
            nullptr,
            0,
            D3D11_SDK_VERSION,
            &scd,
            &m_implementation->swapChain,
            &m_implementation->device,
            nullptr,
            &m_implementation->context
        );

        if (FAILED(hr)) {
            LOG_ERROR(L"Engine", L"Initialize", L"Error al crear D3D11 Device y SwapChain.");
            return false;
        }

        // Crear Render Target View
        ID3D11Texture2D* backBuffer = nullptr;
        hr = m_implementation->swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&backBuffer));
        if (FAILED(hr)) {
            LOG_ERROR(L"Engine", L"Initialize", L"Fallo al obtener el BackBuffer.");
            return false;
        }

        hr = m_implementation->device->CreateRenderTargetView(backBuffer, nullptr, &m_implementation->renderTarget);
        SAFE_RELEASE(backBuffer);

        if (FAILED(hr)) {
            LOG_ERROR(L"Engine", L"Initialize", L"Fallo al crear el Render Target View.");
            return false;
        }

        // Configurar el Viewport
        D3D11_VIEWPORT viewport{};
        viewport.TopLeftX = 0.0f;
        viewport.TopLeftY = 0.0f;
        viewport.Width = static_cast<float>(m_implementation->width);
        viewport.Height = static_cast<float>(m_implementation->height);
        viewport.MinDepth = 0.0f;
        viewport.MaxDepth = 1.0f;

        m_implementation->context->RSSetViewports(1, &viewport);

        // Cargar pipeline y shaders (shaders\Triangle.hlsl)
        if (!m_implementation->InitializePipeline()) {
            Shutdown();
            return false;
        }

        m_implementation->isInitialized = true;
        LOG_MESSAGE(L"Engine", L"Initialize", L"EXITOSO");
        return true;
    }

    void Engine::Render() noexcept {
        if (!m_implementation || !m_implementation->isInitialized) {
            return;
        }

        // Color de fondo azul oscuro
        const float clearColor[4] = { 0.1f, 0.2f, 0.4f, 1.0f };

        m_implementation->context->ClearRenderTargetView(m_implementation->renderTarget, clearColor);
        m_implementation->context->OMSetRenderTargets(1, &m_implementation->renderTarget, nullptr);

        // Configurar estado del pipeline
        UINT stride = sizeof(Implementation::Vertex);
        UINT offset = 0;

        m_implementation->context->IASetVertexBuffers(0, 1, &m_implementation->vertexBuffer, &stride, &offset);
        m_implementation->context->IASetInputLayout(m_implementation->inputLayout);
        m_implementation->context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

        m_implementation->context->VSSetShader(m_implementation->vertexShader, nullptr, 0);
        m_implementation->context->PSSetShader(m_implementation->pixelShader, nullptr, 0);

        // Dibujar el triángulo
        m_implementation->context->Draw(3, 0);

        // Presentar cuadro (VSync habilitado)
        m_implementation->swapChain->Present(1, 0);
    }

    void Engine::Shutdown() noexcept {
        if (!m_implementation) {
            return;
        }

        // Liberar recursos de Shaders y Buffers
        SAFE_RELEASE(m_implementation->vertexBuffer);
        SAFE_RELEASE(m_implementation->inputLayout);
        SAFE_RELEASE(m_implementation->pixelShader);
        SAFE_RELEASE(m_implementation->vertexShader);

        // Liberar recursos de Direct3D
        SAFE_RELEASE(m_implementation->renderTarget);
        SAFE_RELEASE(m_implementation->swapChain);

        if (m_implementation->context) {
            m_implementation->context->ClearState();
        }
        SAFE_RELEASE(m_implementation->context);
        SAFE_RELEASE(m_implementation->device);

        m_implementation->window = nullptr;
        m_implementation->width = 0;
        m_implementation->height = 0;
        m_implementation->isInitialized = false;

        LOG_MESSAGE(L"Engine", L"Shutdown", L"EXITOSO");
    }

    // ============================================================================
    // EXPORTACIÓN C-API WRAPPERS (Opcional, en caso de exportar a C/DLL)
    // ============================================================================
    extern "C" {

        ENGINE_API bool Engine_Initialize(HWND hwnd, int width, int height) noexcept {
            if (width < 0 || height < 0) return false;
            return g_engineInstance.Initialize(
                static_cast<void*>(hwnd),
                static_cast<std::uint32_t>(width),
                static_cast<std::uint32_t>(height)
            );
        }

        ENGINE_API void Engine_Update() noexcept {
            // Espacio reservado para lógica de actualización (DeltaTime, Input, etc.)
        }

        ENGINE_API void Engine_Render() noexcept {
            g_engineInstance.Render();
        }

        ENGINE_API void Engine_Shutdown() noexcept {
            g_engineInstance.Shutdown();
        }

    } // extern "C"