#pragma once

#include "imgui.h"
#include <string>

namespace app::ui {

class TextRenderer {
public:
    TextRenderer() = delete;

    /**
     * @brief Renders justified text within the specified width.
     * @return The total height (in pixels) consumed by the rendered text.
     */
    static float RenderJustifiedText(
        ImDrawList*        drawList,
        const std::string& text,
        ImVec2             pos,
        float              width,
        ImFont*            font,
        float              fontSize,
        ImU32              color
    );
};

} // namespace app::ui
