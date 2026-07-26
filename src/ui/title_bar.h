#pragma once

#include "imgui.h"
#include <string>

namespace app {
class Window;
}

namespace app::ui {

class TitleBar {
public:
    TitleBar() = delete;

    /**
     * @brief Renders custom window titlebar.
     * @return true if the settings button was toggled.
     */
    static bool Draw(
        ImDrawList* drawList,
        Window*     window,
        float       width,
        ImFont*     fontRegular,
        bool        showSettingsActive,
        bool        disableMaximize = false
    );

private:
    static void DrawSettingsIcon(ImDrawList* drawList, ImVec2 center, float size, ImU32 color);
};

} // namespace app::ui
