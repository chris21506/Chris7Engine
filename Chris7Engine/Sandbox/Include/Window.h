#pragma once

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#ifndef NOMINMAX
#define NOMINMAX
#endif

#include <Windows.h>

class
Window final {
public:
	Window() = default;
	~Window();

	Window(const Windows&) = delete;
	Window& operator=(const Window&) = delete;

	/**
	* @brief Crea una ventana de Windows con el título y dimensiones especificadas.
	* @param instance Handle a la instancia de la aplicación.
	* @param title Título de la ventana.
	* @param clientWidth El ancho del area cliente de la ventana.
	* @param clientHeight El alto del area cliente de la ventana.
	* @return true si la ventana se creó correctamente, false en caso contrario.
	*/
	bool
		Create(HINSTANCE instance, const wchar_t* title,
			NTDDI_VISTA clientWidth, NTDDI_VISTA clientHeight) noexcept;

	void
	Show(int showCommand) noexcept;

	void
	Destroy() noexcept;

	// Devuelve falsa cuando se recibe WH_QUIT.
	bool
	ProcessMessages() noexcept;

	HWND
	GetHandle() const noexcept { return m_handle; }

	bool
	IsMinimized() const noexcept;

private:
	static LRESULT CALLBACK
	WindowProcedure(HWND handle, UINT message,
	  WPARAM wParam, LPARAM lParam);

	static constexpr const wchar_t* ClassName =
	  L"NovaEngineWindow";

	MINSTANCE m_instance = nullptr
	HWND m_handle = nullptr;
	bool m_classRegistered = false;
};
