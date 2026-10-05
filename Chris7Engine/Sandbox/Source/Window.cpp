#include "Window.h"
#include <cstdint>

// Destructor
Window::~Window() {
    Destroy();
}

// Crea la ventana ajustando las dimensiones al área cliente deseada
bool Window::Create(HINSTANCE instance, const wchar_t* title, std::uint32_t clientWidth, std::uint32_t clientHeight) noexcept {
    m_instance = instance;

    // Registrar la clase de ventana una sola vez
    if (!m_classRegistered) {
        WNDCLASSEXW wc = {};
        wc.cbSize = sizeof(WNDCLASSEXW);
        wc.style = CS_HREDRAW | CS_VREDRAW;
        wc.lpfnWndProc = WindowProcedure;
        wc.hInstance = m_instance;
        wc.hIcon = LoadIcon(nullptr, IDI_APPLICATION);
        wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
        wc.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1);
        wc.lpszClassName = ClassName;
        wc.hIconSm = LoadIcon(nullptr, IDI_APPLICATION);

        if (!RegisterClassExW(&wc)) {
            return false;
        }

        m_classRegistered = true;
    }

    // Calcular el tamaño total de la ventana incluyendo marcos/título
    RECT windowRect = { 0, 0, static_cast<LONG>(clientWidth), static_cast<LONG>(clientHeight) };
    AdjustWindowRect(&windowRect, WS_OVERLAPPEDWINDOW, FALSE);

    // Crear la ventana
    m_handle = CreateWindowExW(
        0,
        ClassName,
        title,
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT,
        windowRect.right - windowRect.left,
        windowRect.bottom - windowRect.top,
        nullptr,
        nullptr,
        m_instance,
        this // Se pasa 'this' para asociarlo en WM_NCCREATE
    );

    return m_handle != nullptr;
}

void Window::Show(int showCommand) noexcept {
    if (m_handle) {
        ShowWindow(m_handle, showCommand);
        UpdateWindow(m_handle);
    }
}

void Window::Destroy() noexcept {
    if (m_handle) {
        DestroyWindow(m_handle);
        m_handle = nullptr;
    }

    if (m_classRegistered && m_instance) {
        UnregisterClassW(ClassName, m_instance);
        m_classRegistered = false;
    }
}

bool Window::ProcessMessages() noexcept {
    MSG msg = {};
    while (PeekMessageW(&msg, nullptr, 0, 0, PM_REMOVE)) {
        if (msg.message == WM_QUIT) {
            return false;
        }
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }
    return true;
}

bool Window::IsMinimized() const noexcept {
    if (!m_handle) {
        return false;
    }
    return IsIconic(m_handle) != FALSE;
}

// Callback de procedimiento de ventana (Static)
LRESULT CALLBACK Window::WindowProcedure(HWND handle, UINT message, WPARAM wParam, LPARAM lParam) {
    Window* pWindow = nullptr;

    if (message == WM_NCCREATE) {
        CREATESTRUCTW* pCreate = reinterpret_cast<CREATESTRUCTW*>(lParam);
        pWindow = reinterpret_cast<Window*>(pCreate->lpCreateParams);
        SetWindowLongPtrW(handle, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pWindow));
    }
    else {
        pWindow = reinterpret_cast<Window*>(GetWindowLongPtrW(handle, GWLP_USERDATA));
    }

    switch (message) {
    case WM_CLOSE: {
        DestroyWindow(handle);
        return 0;
    }
    case WM_DESTROY: {
        PostQuitMessage(0);
        return 0;
    }
    default:
        break;
    }

    return DefWindowProcW(handle, message, wParam, lParam);
}