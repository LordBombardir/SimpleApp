#include "window/native_window.h"
#include "window/window.h"
#include <GLFW/glfw3.h>
#include <stdexcept>

#ifdef IsMaximized
#undef IsMaximized
#endif
#ifdef IsMinimized
#undef IsMinimized
#endif

namespace app {

class GlfwNativeWindow : public INativeWindow {
public:
    explicit GlfwNativeWindow(Window* parent) : m_parent(parent) {}

    ~GlfwNativeWindow() override {
        if (m_window) {
            glfwDestroyWindow(m_window);
        }

        glfwTerminate();
    }

    bool Initialize(const std::string& title, int width, int height, int minWidth, int minHeight) {
        m_minWidth  = minWidth;
        m_minHeight = minHeight;
        if (!glfwInit()) return false;

        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif
        glfwWindowHint(GLFW_DECORATED, GLFW_FALSE);
        glfwWindowHint(GLFW_TRANSPARENT_FRAMEBUFFER, GLFW_TRUE);

        m_window = glfwCreateWindow(width, height, title.c_str(), nullptr, nullptr);
        if (!m_window) return false;

        glfwSetWindowSizeLimits(m_window, m_minWidth, m_minHeight, GLFW_DONT_CARE, GLFW_DONT_CARE);

        glfwMakeContextCurrent(m_window);
        glfwSwapInterval(1); // Enable VSync

        glfwSetFramebufferSizeCallback(m_window, [](GLFWwindow*, int, int) {});

        return true;
    }

    bool ProcessMessages() override {
        glfwPollEvents();
        UpdateBorderResizing();
        return !glfwWindowShouldClose(m_window);
    }

    void Present() override {
        if (m_window) {
            glfwSwapBuffers(m_window);
        }
    }

    void GetSize(int& width, int& height) const override {
        if (m_window) {
            glfwGetFramebufferSize(m_window, &width, &height);
        } else {
            width  = 0;
            height = 0;
        }
    }

    void GetMinSize(int& minWidth, int& minHeight) const override {
        minWidth  = m_minWidth;
        minHeight = m_minHeight;
    }

    void SetMinSize(int minWidth, int minHeight) override {
        m_minWidth  = minWidth;
        m_minHeight = minHeight;
        if (m_window) {
            glfwSetWindowSizeLimits(m_window, m_minWidth, m_minHeight, GLFW_DONT_CARE, GLFW_DONT_CARE);
        }
    }

    int GetHoveredButton() const override { return 0; }
    int GetPressedButton() const override { return 0; }

    bool IsMaximizeEnabled() const override { return m_maximizeEnabled; }
    void SetMaximizeEnabled(bool enabled) override { m_maximizeEnabled = enabled; }

    bool IsResizeEnabled() const override { return m_resizeEnabled; }
    void SetResizeEnabled(bool enabled) override {
        if (m_resizeEnabled == enabled) return;

        m_resizeEnabled = enabled;
        if (m_window) {
            glfwSetWindowAttrib(m_window, GLFW_RESIZABLE, enabled ? GLFW_TRUE : GLFW_FALSE);
        }
    }

    bool IsMaximized() const override {
        if (!m_window) return false;
        return glfwGetWindowAttrib(m_window, GLFW_MAXIMIZED) != 0;
    }

    bool IsMinimized() const override {
        if (!m_window) return false;
        return glfwGetWindowAttrib(m_window, GLFW_ICONIFIED) != 0;
    }

    void Minimize() override {
        if (m_window) {
            glfwIconifyWindow(m_window);
        }
    }

    void MaximizeOrRestore() override {
        if (!IsMaximizeEnabled()) return;
        if (m_window) {
            if (IsMaximized()) {
                glfwRestoreWindow(m_window);
            } else {
                glfwMaximizeWindow(m_window);
            }
        }
    }

    void Close() override {
        if (m_window) {
            glfwSetWindowShouldClose(m_window, GLFW_TRUE);
        }
    }

    void Show() override {
        if (m_window) {
            glfwShowWindow(m_window);
        }
    }

    void StartDragging() override {
        if (!m_window) return;

        if (IsMaximized()) {
            int winX = 0, winY = 0;
            glfwGetWindowPos(m_window, &winX, &winY);

            int maxW = 0, maxH = 0;
            GetSize(maxW, maxH);

            double cursorX = 0.0, cursorY = 0.0;
            glfwGetCursorPos(m_window, &cursorX, &cursorY);

            double screenMouseX = winX + cursorX;
            double screenMouseY = winY + cursorY;

            double ratio = (maxW > 0) ? (cursorX / static_cast<double>(maxW)) : 0.5;

            glfwRestoreWindow(m_window);

            int resW = 0, resH = 0;
            GetSize(resW, resH);

            m_dragClickX = resW * ratio;
            m_dragClickY = cursorY;

            int newWinX = static_cast<int>(screenMouseX - m_dragClickX);
            int newWinY = static_cast<int>(screenMouseY - m_dragClickY);

            glfwSetWindowPos(m_window, newWinX, newWinY);
            m_isDragging = true;
        } else {
            m_isDragging = true;
            glfwGetCursorPos(m_window, &m_dragClickX, &m_dragClickY);
        }
    }

    void UpdateDragging() override {
        if (!m_window) return;

        double cursorX = 0.0, cursorY = 0.0;
        glfwGetCursorPos(m_window, &cursorX, &cursorY);

        int winW = 0, winH = 0;
        GetSize(winW, winH);

        int winX = 0, winY = 0;
        glfwGetWindowPos(m_window, &winX, &winY);

        bool isLeftPressed = (glfwGetMouseButton(m_window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS);

        // Titlebar dragging
        if (m_isDragging) {
            if (!isLeftPressed) {
                m_isDragging = false;
                return;
            }

            int deltaX = static_cast<int>(cursorX - m_dragClickX);
            int deltaY = static_cast<int>(cursorY - m_dragClickY);

            if (deltaX != 0 || deltaY != 0) {
                glfwSetWindowPos(m_window, winX + deltaX, winY + deltaY);
            }
            return;
        }

        // Border resizing
        if (m_isResizing) {
            if (!isLeftPressed) {
                m_isResizing = false;
                m_resizeEdge = ResizeEdge::None;
                return;
            }

            double screenMouseX = winX + cursorX;
            double screenMouseY = winY + cursorY;

            int deltaX = static_cast<int>(screenMouseX - m_resizeStartMouseX);
            int deltaY = static_cast<int>(screenMouseY - m_resizeStartMouseY);

            int newX = m_resizeStartWinX;
            int newY = m_resizeStartWinY;
            int newW = m_resizeStartWinW;
            int newH = m_resizeStartWinH;

            if (m_resizeEdge == ResizeEdge::Left || m_resizeEdge == ResizeEdge::TopLeft
                || m_resizeEdge == ResizeEdge::BottomLeft) {
                newW = m_resizeStartWinW - deltaX;
                if (newW < m_minWidth) {
                    deltaX = m_resizeStartWinW - m_minWidth;
                    newW   = m_minWidth;
                }
                newX = m_resizeStartWinX + deltaX;
            } else if (
                m_resizeEdge == ResizeEdge::Right || m_resizeEdge == ResizeEdge::TopRight
                || m_resizeEdge == ResizeEdge::BottomRight
            ) {
                newW = m_resizeStartWinW + deltaX;
                if (newW < m_minWidth) newW = m_minWidth;
            }

            if (m_resizeEdge == ResizeEdge::Top || m_resizeEdge == ResizeEdge::TopLeft
                || m_resizeEdge == ResizeEdge::TopRight) {
                newH = m_resizeStartWinH - deltaY;
                if (newH < m_minHeight) {
                    deltaY = m_resizeStartWinH - m_minHeight;
                    newH   = m_minHeight;
                }
                newY = m_resizeStartWinY + deltaY;
            } else if (
                m_resizeEdge == ResizeEdge::Bottom || m_resizeEdge == ResizeEdge::BottomLeft
                || m_resizeEdge == ResizeEdge::BottomRight
            ) {
                newH = m_resizeStartWinH + deltaY;
                if (newH < m_minHeight) newH = m_minHeight;
            }

            glfwSetWindowPos(m_window, newX, newY);
            glfwSetWindowSize(m_window, newW, newH);
            return;
        }

        // Hover detection for border resizing
        if (!IsMaximized() && m_resizeEnabled) {
            ResizeEdge edge = GetResizeEdgeAt(cursorX, cursorY, winW, winH);
            if (edge != ResizeEdge::None && isLeftPressed && !m_isResizing) {
                m_isResizing        = true;
                m_resizeEdge        = edge;
                m_resizeStartMouseX = winX + cursorX;
                m_resizeStartMouseY = winY + cursorY;
                m_resizeStartWinX   = winX;
                m_resizeStartWinY   = winY;
                m_resizeStartWinW   = winW;
                m_resizeStartWinH   = winH;
            }
        }
    }

    void                 SetApp(std::shared_ptr<App> app) override { m_app = app; }
    std::shared_ptr<App> GetApp() const override { return m_app.lock(); }

#ifdef _WIN32
    HWND                    GetHWND() const override { return nullptr; }
    ID3D11Device*           GetD3DDevice() const override { return nullptr; }
    ID3D11DeviceContext*    GetD3DDeviceContext() const override { return nullptr; }
    ID3D11RenderTargetView* GetMainRenderTargetView() const override { return nullptr; }
#else
    GLFWwindow* GetGLFWWindow() const override { return m_window; }
#endif

private:
    enum class ResizeEdge { None, Left, Right, Top, Bottom, TopLeft, TopRight, BottomLeft, BottomRight };

    ResizeEdge GetResizeEdgeAt(double cursorX, double cursorY, int winW, int winH) const {
        const double MARGIN = 8.0;
        bool left   = cursorX >= 0.0 && cursorX <= MARGIN;
        bool right  = cursorX >= (winW - MARGIN) && cursorX <= winW;
        bool top    = cursorY >= 0.0 && cursorY <= MARGIN;
        bool bottom = cursorY >= (winH - MARGIN) && cursorY <= winH;

        if (top && left) return ResizeEdge::TopLeft;
        if (top && right) return ResizeEdge::TopRight;
        if (bottom && left) return ResizeEdge::BottomLeft;
        if (bottom && right) return ResizeEdge::BottomRight;
        if (left) return ResizeEdge::Left;
        if (right) return ResizeEdge::Right;
        if (top) return ResizeEdge::Top;
        if (bottom) return ResizeEdge::Bottom;

        return ResizeEdge::None;
    }

    GLFWwindow*        m_window              = nullptr;
    std::weak_ptr<App> m_app;
    Window*              m_parent              = nullptr;
    int                  m_minWidth            = 900;
    int                  m_minHeight           = 690;
    bool                 m_maximizeEnabled     = true;
    bool                 m_resizeEnabled       = true;
    bool                 m_isDragging          = false;
    double               m_dragClickX          = 0.0;
    double               m_dragClickY          = 0.0;
    bool                 m_isResizing          = false;
    ResizeEdge           m_resizeEdge          = ResizeEdge::None;
    double               m_resizeStartMouseX   = 0.0;
    double               m_resizeStartMouseY   = 0.0;
    int                  m_resizeStartWinX     = 0;
    int                  m_resizeStartWinY     = 0;
    int                  m_resizeStartWinW     = 0;
    int                  m_resizeStartWinH     = 0;
};

std::unique_ptr<INativeWindow>
INativeWindow::Create(Window* parent, const std::string& title, int width, int height, int minWidth, int minHeight) {
    auto win = std::make_unique<GlfwNativeWindow>(parent);
    if (!win->Initialize(title, width, height, minWidth, minHeight)) {
        return nullptr;
    }
    return win;
}

} // namespace app
