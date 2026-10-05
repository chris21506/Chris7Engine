#include <Engine/Engine.h>
#include "Windows.h"

int WINAPI 
wWinMain(HINSTANCE instance, HINSTANCE previousInstance, PWSTR commandLine, int showCommand9) {
    UNREFERENCED_PARAMETER(previousInstance);
    UNREFERENCED_PARAMETER(commandLine);


    constexpr UINT CLIENT_WIDTH = 1280;
    constexpr UINT CLIENT_HEIGHT = 720;

    Window window;

    if (!window.Create(instance, L"Nova Engine", CLIENT_WIDTH, CLIENT_HEIGHT))
    {
        MessageBoxW(
            nullptr,
            L"No se pudo crear la ventana.",
            L"Window Error"
            MB_OK | MB_ICONERROR
        );
        return 1;
    }

    Engine engine;

    if (!engine.Initialize(window.GetHandle(), CLIENT_WIDTH, CLIENT_HEIGHT))
    {
        MessageBoxW(
            window.GetHandle(),
            L"No se puede inicializar el Engine.\n\n"
            L"Verifica que exista:\n"
            L"shaders\\Triangle.hlsl\n\n"
            L"Revisa tambien la ventana Output.",
            L"Engine Error",
            MB_OK | MB_ICONERROR
        );

        return 1;
    }

    window.Show(showCommand);

    while (window.ProcessMessages())
    {
        if (window.IsMinimized())
        {
            WaitMessage();
            continue;
        }

        engine.Render();
    }

    engine.Shutdown();
    window.Destroy();

    return 0;
}

