#include "core/app.h"
#include "window/window.h"

#include <chrono>
#include <iostream>
#include <memory>

#ifdef _WIN32
#include <windows.h>
int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow) {
#else
int main(int argc, char* argv[]) {
#endif
    try {
        auto window = std::make_shared<app::Window>("Who Am I?", 900, 700, 900, 700);

        auto application = std::make_shared<app::App>(window);
        window->SetApp(application);

        window->Show();

        auto lastTime = std::chrono::high_resolution_clock::now();

        bool running = true;
        while (running) {
            if (!window->ProcessMessages()) {
                running = false;
                break;
            }

            auto                         currentTime = std::chrono::high_resolution_clock::now();
            std::chrono::duration<float> elapsed     = currentTime - lastTime;
            lastTime                                 = currentTime;

            float deltaTime = elapsed.count();
            if (deltaTime > 0.1f) deltaTime = 0.1f;

            application->Update(deltaTime);

            application->Render();
            window->Present();
        }
    } catch (const std::exception& e) {
#ifdef _WIN32
        MessageBoxA(nullptr, e.what(), "Critical Error", MB_ICONERROR | MB_OK);
#else
        std::cerr << "Critical Error: " << e.what() << std::endl;
#endif
        return 1;
    }

    return 0;
}
