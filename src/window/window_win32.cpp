#ifndef NOMINMAX
#define NOMINMAX
#endif

#include "core/app.h"
#include "window/native_window.h"
#include "window/window.h"
#include <chrono>
#include <d3d11.h>
#include <dwmapi.h>
#include <stdexcept>
#include <windows.h>
#include <windowsx.h>

#ifdef IsMaximized
#undef IsMaximized
#endif
#ifdef IsMinimized
#undef IsMinimized
#endif

#ifndef DWMWA_WINDOW_CORNER_PREFERENCE
#define DWMWA_WINDOW_CORNER_PREFERENCE 33
#endif

#ifndef DWMWCP_ROUND
#define DWMWCP_ROUND 2
#endif

#pragma comment(lib, "dwmapi.lib")
#pragma comment(lib, "d3d11.lib")

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

namespace app {

class Win32NativeWindow : public INativeWindow {
public:
    explicit Win32NativeWindow(Window* parent) : m_parent(parent) {}

    ~Win32NativeWindow() override {
        CleanupRenderTarget();
        CleanupDeviceD3D();
        if (m_hWnd) {
            DestroyWindow(m_hWnd);
        }
    }

    bool Initialize(const std::string& title, int width, int height, int minWidth, int minHeight) {
        m_minWidth  = minWidth;
        m_minHeight = minHeight;

        if (!CreateWin32Window(title, width, height)) return false;
        if (!CreateDeviceD3D()) {
            CleanupDeviceD3D();
            return false;
        }

        CreateRenderTarget();
        return true;
    }

    bool ProcessMessages() override {
        MSG msg;
        while (PeekMessage(&msg, nullptr, 0U, 0U, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
            if (msg.message == WM_QUIT) {
                return false;
            }
        }

        return true;
    }

    void Present() override {
        if (m_pSwapChain) {
            m_pSwapChain->Present(1, 0);
        }
    }

    void GetSize(int& width, int& height) const override {
        RECT rect;
        GetClientRect(m_hWnd, &rect);
        width  = rect.right - rect.left;
        height = rect.bottom - rect.top;
    }

    void GetMinSize(int& minWidth, int& minHeight) const override {
        minWidth  = m_minWidth;
        minHeight = m_minHeight;
    }

    void SetMinSize(int minWidth, int minHeight) override {
        m_minWidth  = minWidth;
        m_minHeight = minHeight;
    }

    int GetHoveredButton() const override { return m_hoveredButton; }
    int GetPressedButton() const override { return m_pressedButton; }

    bool IsMaximizeEnabled() const override { return m_maximizeEnabled; }
    void SetMaximizeEnabled(bool enabled) override { m_maximizeEnabled = enabled; }
    bool IsResizeEnabled() const override { return m_resizeEnabled; }
    void SetResizeEnabled(bool enabled) override { m_resizeEnabled = enabled; }

    bool IsMaximized() const override {
        WINDOWPLACEMENT wp = {sizeof(wp)};
        if (GetWindowPlacement(m_hWnd, &wp)) {
            return wp.showCmd == SW_SHOWMAXIMIZED;
        }

        return false;
    }

    bool IsMinimized() const override { return IsIconic(m_hWnd) != FALSE; }

    void Minimize() override { ShowWindow(m_hWnd, SW_MINIMIZE); }

    void MaximizeOrRestore() override {
        if (!IsMaximizeEnabled()) return;
        if (IsMaximized()) {
            ShowWindow(m_hWnd, SW_RESTORE);
        } else {
            ShowWindow(m_hWnd, SW_MAXIMIZE);
        }
    }

    void Close() override { PostMessage(m_hWnd, WM_CLOSE, 0, 0); }

    void Show() override {
        ShowWindow(m_hWnd, SW_SHOWDEFAULT);
        UpdateWindow(m_hWnd);
    }

    void                 SetApp(std::shared_ptr<App> app) override { m_app = app; }
    std::shared_ptr<App> GetApp() const override { return m_app.lock(); }

    HWND                    GetHWND() const override { return m_hWnd; }
    ID3D11Device*           GetD3DDevice() const override { return m_pd3dDevice; }
    ID3D11DeviceContext*    GetD3DDeviceContext() const override { return m_pd3dDeviceContext; }
    ID3D11RenderTargetView* GetMainRenderTargetView() const override { return m_mainRenderTargetView; }

private:
    bool CreateWin32Window(const std::string& title, int width, int height) {
        HINSTANCE hInstance = GetModuleHandle(nullptr);

        WNDCLASSEXW wc   = {};
        wc.cbSize        = sizeof(wc);
        wc.style         = CS_CLASSDC | CS_HREDRAW | CS_VREDRAW;
        wc.lpfnWndProc   = WndProc;
        wc.hInstance     = hInstance;
        wc.hCursor       = LoadCursor(nullptr, IDC_ARROW);
        wc.lpszClassName = L"SimpleAppWindowClass";

        RegisterClassExW(&wc);

        SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);

        int screenWidth  = GetSystemMetrics(SM_CXSCREEN);
        int screenHeight = GetSystemMetrics(SM_CYSCREEN);
        int posX         = (screenWidth - width) / 2;
        int posY         = (screenHeight - height) / 2;

        std::wstring wTitle(title.begin(), title.end());

        m_hWnd = CreateWindowExW(
            WS_EX_APPWINDOW,
            wc.lpszClassName,
            wTitle.c_str(),
            WS_OVERLAPPEDWINDOW,
            posX,
            posY,
            width,
            height,
            nullptr,
            nullptr,
            hInstance,
            this
        );

        if (!m_hWnd) return false;

        BOOL darkMode = TRUE;
        DwmSetWindowAttribute(m_hWnd, DWMWA_USE_IMMERSIVE_DARK_MODE, &darkMode, sizeof(darkMode));

        DWORD cornerPref = DWMWCP_ROUND;
        DwmSetWindowAttribute(m_hWnd, DWMWA_WINDOW_CORNER_PREFERENCE, &cornerPref, sizeof(cornerPref));

        MARGINS margins = {1, 1, 1, 1};
        DwmExtendFrameIntoClientArea(m_hWnd, &margins);

        SetWindowPos(
            m_hWnd,
            nullptr,
            0,
            0,
            0,
            0,
            SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE | SWP_FRAMECHANGED
        );

        return true;
    }

    bool CreateDeviceD3D() {
        DXGI_SWAP_CHAIN_DESC sd               = {};
        sd.BufferCount                        = 2;
        sd.BufferDesc.Width                   = 0;
        sd.BufferDesc.Height                  = 0;
        sd.BufferDesc.Format                  = DXGI_FORMAT_R8G8B8A8_UNORM;
        sd.BufferDesc.RefreshRate.Numerator   = 60;
        sd.BufferDesc.RefreshRate.Denominator = 1;
        sd.Flags                              = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
        sd.BufferUsage                        = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        sd.OutputWindow                       = m_hWnd;
        sd.SampleDesc.Count                   = 1;
        sd.SampleDesc.Quality                 = 0;
        sd.Windowed                           = TRUE;
        sd.SwapEffect                         = DXGI_SWAP_EFFECT_DISCARD;

        UINT                    createDeviceFlags = 0;
        D3D_FEATURE_LEVEL       featureLevel;
        const D3D_FEATURE_LEVEL featureLevelArray[2] = {D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_0};

        HRESULT hr = D3D11CreateDeviceAndSwapChain(
            nullptr,
            D3D_DRIVER_TYPE_HARDWARE,
            nullptr,
            createDeviceFlags,
            featureLevelArray,
            2,
            D3D11_SDK_VERSION,
            &sd,
            &m_pSwapChain,
            &m_pd3dDevice,
            &featureLevel,
            &m_pd3dDeviceContext
        );

        if (SUCCEEDED(hr)) {
            D3D11_SAMPLER_DESC sampDesc = {};
            sampDesc.Filter             = D3D11_FILTER_ANISOTROPIC;
            sampDesc.AddressU           = D3D11_TEXTURE_ADDRESS_CLAMP;
            sampDesc.AddressV           = D3D11_TEXTURE_ADDRESS_CLAMP;
            sampDesc.AddressW           = D3D11_TEXTURE_ADDRESS_CLAMP;
            sampDesc.MaxAnisotropy      = 16;
            sampDesc.ComparisonFunc     = D3D11_COMPARISON_ALWAYS;
            sampDesc.MinLOD             = 0;
            sampDesc.MaxLOD             = D3D11_FLOAT32_MAX;
            m_pd3dDevice->CreateSamplerState(&sampDesc, &m_pSmoothSampler);
        }

        return SUCCEEDED(hr);
    }

    void CleanupDeviceD3D() {
        if (m_pSmoothSampler) {
            m_pSmoothSampler->Release();
            m_pSmoothSampler = nullptr;
        }
        if (m_pSwapChain) {
            m_pSwapChain->Release();
            m_pSwapChain = nullptr;
        }
        if (m_pd3dDeviceContext) {
            m_pd3dDeviceContext->Release();
            m_pd3dDeviceContext = nullptr;
        }
        if (m_pd3dDevice) {
            m_pd3dDevice->Release();
            m_pd3dDevice = nullptr;
        }
    }

    void CreateRenderTarget() {
        ID3D11Texture2D* pBackBuffer = nullptr;
        m_pSwapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));
        if (pBackBuffer) {
            m_pd3dDevice->CreateRenderTargetView(pBackBuffer, nullptr, &m_mainRenderTargetView);
            pBackBuffer->Release();
        }
    }

    void CleanupRenderTarget() {
        if (m_mainRenderTargetView) {
            m_mainRenderTargetView->Release();
            m_mainRenderTargetView = nullptr;
        }
    }

    void ResizeRenderTarget() {
        CleanupRenderTarget();
        m_pSwapChain->ResizeBuffers(0, 0, 0, DXGI_FORMAT_UNKNOWN, 0);
        CreateRenderTarget();
    }

    static LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
        if (msg == WM_NCCREATE) {
            CREATESTRUCT* pCreate = reinterpret_cast<CREATESTRUCT*>(lParam);
            SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pCreate->lpCreateParams));
        }

        Win32NativeWindow* pThis = reinterpret_cast<Win32NativeWindow*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));
        if (pThis) {
            return pThis->MemberWndProc(hWnd, msg, wParam, lParam);
        }

        return DefWindowProc(hWnd, msg, wParam, lParam);
    }

    LRESULT MemberWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
        if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam)) {
            return true;
        }

        switch (msg) {
        case WM_NCCALCSIZE: {
            if (wParam) {
                NCCALCSIZE_PARAMS* params = reinterpret_cast<NCCALCSIZE_PARAMS*>(lParam);
                if (IsMaximized()) {
                    HMONITOR    monitor     = MonitorFromWindow(hWnd, MONITOR_DEFAULTTONEAREST);
                    MONITORINFO monitorInfo = {sizeof(monitorInfo)};
                    if (GetMonitorInfoW(monitor, &monitorInfo)) {
                        params->rgrc[0] = monitorInfo.rcWork;
                    }
                }
                return 0;
            }

            break;
        }

        case WM_GETMINMAXINFO: {
            MINMAXINFO* mmi       = reinterpret_cast<MINMAXINFO*>(lParam);
            mmi->ptMinTrackSize.x = m_minWidth;
            mmi->ptMinTrackSize.y = m_minHeight;
            return 0;
        }

        case WM_NCPAINT:
            return 0;

        case WM_NCACTIVATE:
            return TRUE;

        case WM_NCHITTEST: {
            POINT cursor = {GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)};
            RECT  windowRect;
            GetWindowRect(hWnd, &windowRect);

            int borderSizeX = GetSystemMetrics(SM_CXSIZEFRAME) + GetSystemMetrics(SM_CXPADDEDBORDER);
            int borderSizeY = GetSystemMetrics(SM_CYSIZEFRAME) + GetSystemMetrics(SM_CXPADDEDBORDER);

            int x      = cursor.x - windowRect.left;
            int y      = cursor.y - windowRect.top;
            int width  = windowRect.right - windowRect.left;
            int height = windowRect.bottom - windowRect.top;

            if (!IsMaximized() && IsResizeEnabled()) {
                bool left   = (x < borderSizeX);
                bool right  = (x > width - borderSizeX);
                bool top    = (y < borderSizeY);
                bool bottom = (y > height - borderSizeY);

                if (left && top) return HTTOPLEFT;
                if (left && bottom) return HTBOTTOMLEFT;
                if (right && top) return HTTOPRIGHT;
                if (right && bottom) return HTBOTTOMRIGHT;
                if (left) return HTLEFT;
                if (right) return HTRIGHT;
                if (top) return HTTOP;
                if (bottom) return HTBOTTOM;
            }

            if (y < TitleBarLayout::Height) {
                if (TitleBarLayout::SettingsButton().Contains(x, y)) {
                    return HTCLIENT;
                }
                if (TitleBarLayout::CloseButton(width).Contains(x, y)) {
                    return HTCLOSE;
                }
                if (TitleBarLayout::MaximizeButton(width).Contains(x, y)) {
                    return IsMaximizeEnabled() ? HTMAXBUTTON : HTCLIENT;
                }
                if (TitleBarLayout::MinimizeButton(width).Contains(x, y)) {
                    return HTMINBUTTON;
                }

                return HTCAPTION;
            }

            return HTCLIENT;
        }

        case WM_NCMOUSEMOVE: {
            int code     = static_cast<int>(wParam);
            int newHover = 0;
            if (code == HTMINBUTTON) newHover = 1;
            else if (code == HTMAXBUTTON && IsMaximizeEnabled()) newHover = 2;
            else if (code == HTCLOSE) newHover = 3;

            if (m_hoveredButton != newHover) {
                m_hoveredButton = newHover;
            }

            TRACKMOUSEEVENT tme = {};
            tme.cbSize          = sizeof(TRACKMOUSEEVENT);
            tme.dwFlags         = TME_NONCLIENT | TME_LEAVE;
            tme.hwndTrack       = hWnd;
            TrackMouseEvent(&tme);
            break;
        }

        case WM_NCMOUSELEAVE: {
            m_hoveredButton = 0;
            break;
        }

        case WM_NCLBUTTONDBLCLK: {
            if (!IsMaximizeEnabled()) {
                return 0;
            }

            break;
        }

        case WM_NCLBUTTONDOWN: {
            int code = static_cast<int>(wParam);
            if (code == HTMINBUTTON) {
                m_pressedButton = 1;
                Minimize();
                return 0;
            }
            if (code == HTMAXBUTTON) {
                if (!IsMaximizeEnabled()) return 0;
                m_pressedButton = 2;
                MaximizeOrRestore();
                return 0;
            }
            if (code == HTCLOSE) {
                m_pressedButton = 3;
                Close();
                return 0;
            }

            break;
        }

        case WM_NCLBUTTONUP:
        case WM_LBUTTONUP: {
            m_pressedButton = 0;
            break;
        }

        case WM_ENTERSIZEMOVE: {
            m_lastTimerTime = std::chrono::high_resolution_clock::now();
            SetTimer(hWnd, 1, 10, nullptr);
            break;
        }

        case WM_EXITSIZEMOVE: {
            KillTimer(hWnd, 1);
            break;
        }

        case WM_TIMER: {
            if (wParam == 1 && m_pd3dDevice != nullptr) {
                if (auto app = m_app.lock()) {
                    auto                         currentTime = std::chrono::high_resolution_clock::now();
                    std::chrono::duration<float> elapsed     = currentTime - m_lastTimerTime;
                    m_lastTimerTime                          = currentTime;

                    float dt = elapsed.count();
                    if (dt > 0.1f) dt = 0.1f;

                    app->Update(dt);
                    app->Render();
                    Present();
                }
            }

            return 0;
        }

        case WM_SIZE:
        case WM_PAINT: {
            if (m_pd3dDevice != nullptr && wParam != SIZE_MINIMIZED) {
                ResizeRenderTarget();
                if (auto app = m_app.lock()) {
                    app->Render();
                    Present();
                }
            }
            if (msg == WM_PAINT) {
                ValidateRect(hWnd, nullptr);
                return 0;
            }

            return 0;
        }

        case WM_DESTROY: {
            KillTimer(hWnd, 1);
            PostQuitMessage(0);
            return 0;
        }
        }

        return DefWindowProc(hWnd, msg, wParam, lParam);
    }

    HWND                 m_hWnd            = nullptr;
    std::weak_ptr<App>   m_app;
    Window*              m_parent          = nullptr;
    bool                 m_maximizeEnabled = true;
    bool                 m_resizeEnabled   = true;
    int                  m_hoveredButton   = 0;
    int                  m_pressedButton   = 0;

    int                     m_minWidth             = 900;
    int                     m_minHeight            = 690;
    ID3D11Device*           m_pd3dDevice           = nullptr;
    ID3D11DeviceContext*    m_pd3dDeviceContext    = nullptr;
    IDXGISwapChain*         m_pSwapChain           = nullptr;
    ID3D11RenderTargetView* m_mainRenderTargetView = nullptr;
    ID3D11SamplerState*     m_pSmoothSampler       = nullptr;

    std::chrono::high_resolution_clock::time_point m_lastTimerTime;
};

std::unique_ptr<INativeWindow>
INativeWindow::Create(Window* parent, const std::string& title, int width, int height, int minWidth, int minHeight) {
    auto win = std::make_unique<Win32NativeWindow>(parent);
    if (!win->Initialize(title, width, height, minWidth, minHeight)) {
        return nullptr;
    }

    return win;
}

} // namespace app
