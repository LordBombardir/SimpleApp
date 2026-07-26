#include "window/window.h"
#include "window/native_window.h"
#include <stdexcept>

namespace app {

Window::Window(const std::string& title, int width, int height, int minWidth, int minHeight)
: m_impl(INativeWindow::Create(this, title, width, height, minWidth, minHeight)) {
    if (!m_impl) {
        throw std::runtime_error("Failed to create native window.");
    }
}

Window::~Window() = default;

bool Window::ProcessMessages() { return m_impl ? m_impl->ProcessMessages() : false; }

void Window::Present() {
    if (m_impl) m_impl->Present();
}

void Window::GetSize(int& width, int& height) const {
    if (m_impl) {
        m_impl->GetSize(width, height);
    } else {
        width  = 0;
        height = 0;
    }
}

void Window::GetMinSize(int& minWidth, int& minHeight) const {
    if (m_impl) {
        m_impl->GetMinSize(minWidth, minHeight);
    } else {
        minWidth  = 0;
        minHeight = 0;
    }
}

void Window::SetMinSize(int minWidth, int minHeight) {
    if (m_impl) {
        m_impl->SetMinSize(minWidth, minHeight);
    }
}

int Window::GetHoveredButton() const { return m_impl ? m_impl->GetHoveredButton() : 0; }

int Window::GetPressedButton() const { return m_impl ? m_impl->GetPressedButton() : 0; }

bool Window::IsMaximizeEnabled() const { return m_impl ? m_impl->IsMaximizeEnabled() : false; }

void Window::SetMaximizeEnabled(bool enabled) {
    if (m_impl) m_impl->SetMaximizeEnabled(enabled);
}

bool Window::IsResizeEnabled() const { return m_impl ? m_impl->IsResizeEnabled() : false; }

void Window::SetResizeEnabled(bool enabled) {
    if (m_impl) m_impl->SetResizeEnabled(enabled);
}

bool Window::IsMaximized() const { return m_impl ? m_impl->IsMaximized() : false; }

bool Window::IsMinimized() const { return m_impl ? m_impl->IsMinimized() : false; }

void Window::Minimize() {
    if (m_impl) m_impl->Minimize();
}

void Window::MaximizeOrRestore() {
    if (m_impl) m_impl->MaximizeOrRestore();
}

void Window::Close() {
    if (m_impl) m_impl->Close();
}

void Window::Show() {
    if (m_impl) m_impl->Show();
}

void Window::StartDragging() {
    if (m_impl) m_impl->StartDragging();
}

void Window::UpdateDragging() {
    if (m_impl) m_impl->UpdateDragging();
}

void Window::SetApp(std::shared_ptr<App> app) {
    if (m_impl) m_impl->SetApp(app);
}

std::shared_ptr<App> Window::GetApp() const { return m_impl ? m_impl->GetApp() : nullptr; }

} // namespace app
