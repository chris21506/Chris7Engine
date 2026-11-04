#include <Engine/Engine.h>
#include "Window.h"
#include "Device.h"
#include "DeviceContext.h"
#include "SwapChain.h"
#include "MeshComponent.h"
#include "Texture2D.h"

#include <d3d11.h>
#include <d3dcompiler.h>

#include <cstddef>
#include <cstdint>
#include <new>
#include <sstream>
#include <vector>

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
    HWND window = nullptr;
    std::uint32_t width = 0;
    std::uint32_t height = 0;
    bool isInitialized = false;

    // Abstracciones modulares del Engine
    Device device;
    DeviceContext context;
    SwapChain swapChain;
    MeshComponent triangleMesh;
    Texture2D defaultTexture;

    // Recursos del Pipeline Visual
    ID3D11VertexShader* vertexShader = nullptr;
    ID3D11PixelShader* pixelShader = nullptr;
    ID3D11InputLayout* inputLayout = nullptr;

    bool InitializeShaders() noexcept {
        ID3DBlob* vsBlob = nullptr;
        ID3DBlob* psBlob = nullptr;
        ID3DBlob* errorBlob = nullptr;

        ID3D11Device* d3dDevice = device.GetDevice();
        if (!d3dDevice) return false;

        // 1. Compilar Vertex Shader desde shaders\Triangle.hlsl
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
            LOG_ERROR(L"Engine", L"InitializeShaders", L"Fallo al compilar Vertex Shader (shaders\\Triangle.hlsl)");
            return false;
        }

        hr = d3dDevice->CreateVertexShader(vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), nullptr, &vertexShader);
        if (FAILED(hr)) {
            SAFE_RELEASE(vsBlob);
            LOG_ERROR(L"Engine", L"InitializeShaders", L"Fallo al crear Vertex Shader");
            return false;
        }

        // 2. Compilar Pixel Shader
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
            LOG_ERROR(L"Engine", L"InitializeShaders", L"Fallo al compilar Pixel Shader");
            return false;
        }

        hr = d3dDevice->CreatePixelShader(psBlob->GetBufferPointer(), psBlob->GetBufferSize(), nullptr, &pixelShader);
        SAFE_RELEASE(psBlob);
        if (FAILED(hr)) {
            SAFE_RELEASE(vsBlob);
            LOG_ERROR(L"Engine", L"InitializeShaders", L"Fallo al crear Pixel Shader");
            return false;
        }

        // 3. Crear Input Layout compatible con MeshComponent::Vertex
        D3D11_INPUT_ELEMENT_DESC layoutDesc[] = {
            { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,    0, offsetof(MeshComponent::Vertex, position), D3D11_INPUT_PER_VERTEX_DATA, 0 },
            { "COLOR",    0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, offsetof(MeshComponent::Vertex, color),    D3D11_INPUT_PER_VERTEX_DATA, 0 },
            { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,       0, offsetof(MeshComponent::Vertex, texCoord), D3D11_INPUT_PER_VERTEX_DATA, 0 },
            { "NORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT,    0, offsetof(MeshComponent::Vertex, normal),   D3D11_INPUT_PER_VERTEX_DATA, 0 }
        };

        hr = d3dDevice->CreateInputLayout(
            layoutDesc,
            ARRAYSIZE(layoutDesc),
            vsBlob->GetBufferPointer(),
            vsBlob->GetBufferSize(),
            &inputLayout
        );
        SAFE_RELEASE(vsBlob);

        if (FAILED(hr)) {
            LOG_ERROR(L"Engine", L"InitializeShaders", L"Fallo al crear Input Layout");
            return false;
        }

        return true;
    }
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
    if (m_implementation->isInitialized) return true;

    m_implementation->window = static_cast<HWND>(nativeWindow);
    m_implementation->width = width;
    m_implementation->height = height;

    if (!m_implementation->window) {
        LOG_ERROR(L"Engine", L"Initialize", L"HWND nulo proporcionado.");
        return false;
    }

    // 1. Inicializar Dispositivo (Device)
    if (!m_implementation->device.Initialize(m_implementation->window, width, height)) {
        LOG_ERROR(L"Engine", L"Initialize", L"Fallo al inicializar Device.");
        return false;
    }

    // 2. Inicializar Contexto (DeviceContext)
    if (!m_implementation->context.Initialize(m_implementation->device.GetContext())) {
        LOG_ERROR(L"Engine", L"Initialize", L"Fallo al inicializar DeviceContext.");
        return false;
    }

    // 3. Inicializar Cadena de Intercambio (SwapChain)
    if (!m_implementation->swapChain.Initialize(m_implementation->device.GetDevice(), m_implementation->window, width, height)) {
        LOG_ERROR(L"Engine", L"Initialize", L"Fallo al inicializar SwapChain.");
        return false;
    }

    // 4. Compilar Shaders e Input Layout
    if (!m_implementation->InitializeShaders()) {
        Shutdown();
        return false;
    }

    // 5. Configurar geometría usando el componente modular MeshComponent
    std::vector<MeshComponent::Vertex> vertices = {
        { {  0.0f,  0.5f, 0.0f }, { 1.0f, 0.0f, 0.0f, 1.0f }, { 0.5f, 0.0f }, { 0.0f, 0.0f, -1.0f } },
        { {  0.5f, -0.5f, 0.0f }, { 0.0f, 1.0f, 0.0f, 1.0f }, { 1.0f, 1.0f }, { 0.0f, 0.0f, -1.0f } },
        { { -0.5f, -0.5f, 0.0f }, { 0.0f, 0.0f, 1.0f, 1.0f }, { 0.0f, 1.0f }, { 0.0f, 0.0f, -1.0f } }
    };

    if (!m_implementation->triangleMesh.Initialize(m_implementation->device.GetDevice(), vertices)) {
        LOG_ERROR(L"Engine", L"Initialize", L"Fallo al inicializar MeshComponent.");
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

    ID3D11RenderTargetView* rtv = m_implementation->swapChain.GetRenderTargetView();
    if (!rtv) return;

    // 1. Limpieza de pantalla (Fondo azul oscuro)
    const float clearColor[4] = { 0.1f, 0.2f, 0.4f, 1.0f };
    m_implementation->context.ClearRenderTarget(rtv, clearColor);
    m_implementation->context.SetRenderTargets(1, &rtv, nullptr);
    m_implementation->context.SetViewport(
        static_cast<float>(m_implementation->width),
        static_cast<float>(m_implementation->height)
    );

    // 2. Configuración de Pipeline
    m_implementation->context.SetInputLayout(m_implementation->inputLayout);
    m_implementation->context.SetVertexShader(m_implementation->vertexShader);
    m_implementation->context.SetPixelShader(m_implementation->pixelShader);

    // 3. Renderizar Malla
    m_implementation->triangleMesh.Render(m_implementation->context.GetNativeContext());

    // 4. Presentar Búfer en pantalla (VSync activado)
    m_implementation->swapChain.Present(true);
}

void Engine::Shutdown() noexcept {
    if (!m_implementation) {
        return;
    }

    // Liberar Malla y Shaders
    m_implementation->triangleMesh.Shutdown();
    m_implementation->defaultTexture.Release();

    SAFE_RELEASE(m_implementation->inputLayout);
    SAFE_RELEASE(m_implementation->pixelShader);
    SAFE_RELEASE(m_implementation->vertexShader);

    // Liberar Subsistemas DX11
    m_implementation->swapChain.Shutdown();
    m_implementation->context.Shutdown();
    m_implementation->device.Shutdown();

    m_implementation->window = nullptr;
    m_implementation->width = 0;
    m_implementation->height = 0;
    m_implementation->isInitialized = false;

    LOG_MESSAGE(L"Engine", L"Shutdown", L"EXITOSO");
}

// ============================================================================
// EXPORTACIÓN C-API WRAPPERS
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
        // Espacio reservado para actualización de frame (DeltaTime, Input, etc.)
    }

    ENGINE_API void Engine_Render() noexcept {
        g_engineInstance.Render();
    }

    ENGINE_API void Engine_Shutdown() noexcept {
        g_engineInstance.Shutdown();
    }

} // extern "C"