#pragma once

#include "imgui.h"

namespace app::ui {

class SettingsModal {
public:
    SettingsModal() = delete;

    /**
     * @brief Renders the settings overlay modal.
     */
    static void Draw(
        ImDrawList* drawList,
        float       width,
        float       height,
        ImFont*     fontRegular,
        bool&       showSettings
    );
};

} // namespace app::ui
