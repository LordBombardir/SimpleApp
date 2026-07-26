#pragma once

#include <memory>
#include <string>

#ifdef IsMinimized
#undef IsMinimized
#endif
#ifdef IsMaximized
#undef IsMaximized
#endif

namespace app {

class App;
class INativeWindow;

struct TitleBarRect {
    int left;
    int top;
    int right;
    int bottom;

    [[nodiscard]] constexpr bool Contains(int x, int y) const noexcept {
        return x >= left && x < right && y >= top && y < bottom;
    }
};

struct TitleBarLayout {
    static constexpr int Height      = 48;
    static constexpr int ButtonWidth = 46;

    [[nodiscard]] static constexpr TitleBarRect SettingsButton() noexcept { return {6, 6, 42, 42}; }
    [[nodiscard]] static constexpr TitleBarRect MinimizeButton(int windowWidth) noexcept {
        return {windowWidth - 138, 0, windowWidth - 92, Height};
    }
    [[nodiscard]] static constexpr TitleBarRect MaximizeButton(int windowWidth) noexcept {
        return {windowWidth - 92, 0, windowWidth - 46, Height};
    }
    [[nodiscard]] static constexpr TitleBarRect CloseButton(int windowWidth) noexcept {
        return {windowWidth - 46, 0, windowWidth, Height};
    }
};

class Window {
public:
    using Impl = INativeWindow;

    Window(const std::string& title, int width, int height, int minWidth, int minHeight);
    ~Window();

    bool ProcessMessages();
    void Present();

    void GetSize(int& width, int& height) const;
    void GetMinSize(int& minWidth, int& minHeight) const;
    void SetMinSize(int minWidth, int minHeight);

    int GetHoveredButton() const;
    int GetPressedButton() const;

    bool IsMaximizeEnabled() const;
    void SetMaximizeEnabled(bool enabled);
    bool IsResizeEnabled() const;
    void SetResizeEnabled(bool enabled);
    bool IsMaximized() const;
    bool IsMinimized() const;
    void Minimize();
    void MaximizeOrRestore();
    void Close();
    void Show();
    void StartDragging();
    void UpdateDragging();

    void                 SetApp(std::shared_ptr<App> app);
    std::shared_ptr<App> GetApp() const;

    INativeWindow* GetImpl() const { return m_impl.get(); }

private:
    std::unique_ptr<INativeWindow> m_impl;
};

} // namespace app
