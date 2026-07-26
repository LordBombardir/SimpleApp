#pragma once

#include "graphics/grid_bg.h"
#include "imgui.h"
#include <memory>
#include <string>
#include <unordered_map>

namespace app {

enum class FontType { Regular, Title, TitleSmall };
enum class ModalType { None, Settings };

class Window;

class App {
public:
    App(std::shared_ptr<Window> window);
    ~App();

    void Update(float deltaTime);
    void Render();

    ImFont* GetFont(FontType type) const;

    bool HasActiveModal() const { return m_activeModal != ModalType::None; }
    bool IsModalOpen(ModalType type) const { return m_activeModal == type; }
    void OpenModal(ModalType type) { m_activeModal = type; }
    void CloseModal() { m_activeModal = ModalType::None; }
    void ToggleModal(ModalType type) { m_activeModal = (m_activeModal == type) ? ModalType::None : type; }

private:
    void InitImGui();
    void LoadFonts();
    void DrawContent(float width, float height, bool isInteractive);

    std::shared_ptr<Window> m_window = nullptr;
    GridBackground          m_gridBackground;

    std::unordered_map<FontType, ImFont*> m_fonts;

    ModalType m_activeModal = ModalType::None;

    float m_animTime        = 0.0f;
    float m_scrollY         = 0.0f;
    float m_targetScrollY   = 0.0f;
    int   m_targetPageIndex = 0;
    bool  m_wasMinimized    = false;
};

} // namespace app
