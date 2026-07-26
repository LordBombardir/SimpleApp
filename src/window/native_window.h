#pragma once

#include <memory>
#include <string>

#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <d3d11.h>
#include <windows.h>
#ifdef IsMaximized
#undef IsMaximized
#endif
#ifdef IsMinimized
#undef IsMinimized
#endif
#else
#include <GLFW/glfw3.h>
#endif

namespace app {

class Window;
class App;

class INativeWindow {
public:
    virtual ~INativeWindow() = default;

    virtual bool ProcessMessages() = 0;
    virtual void Present()         = 0;

    virtual void GetSize(int& width, int& height) const          = 0;
    virtual void GetMinSize(int& minWidth, int& minHeight) const = 0;
    virtual void SetMinSize(int minWidth, int minHeight)         = 0;

    virtual int GetHoveredButton() const { return 0; }
    virtual int GetPressedButton() const { return 0; }

    virtual bool IsMaximizeEnabled() const        = 0;
    virtual void SetMaximizeEnabled(bool enabled) = 0;
    virtual bool IsResizeEnabled() const          = 0;
    virtual void SetResizeEnabled(bool enabled)   = 0;

    virtual bool IsMaximized() const = 0;
    virtual bool IsMinimized() const = 0;
    virtual void Minimize()          = 0;
    virtual void MaximizeOrRestore() = 0;
    virtual void Close()             = 0;
    virtual void Show()              = 0;

    virtual void StartDragging() {}
    virtual void UpdateDragging() {}

    virtual void                 SetApp(std::shared_ptr<App> app) = 0;
    virtual std::shared_ptr<App> GetApp() const                   = 0;

#ifdef _WIN32
    virtual HWND                    GetHWND() const                 = 0;
    virtual ID3D11Device*           GetD3DDevice() const            = 0;
    virtual ID3D11DeviceContext*    GetD3DDeviceContext() const     = 0;
    virtual ID3D11RenderTargetView* GetMainRenderTargetView() const = 0;
#else
    virtual GLFWwindow* GetGLFWWindow() const = 0;
#endif

    static std::unique_ptr<INativeWindow>
    Create(Window* parent, const std::string& title, int width, int height, int minWidth, int minHeight);
};

} // namespace app
