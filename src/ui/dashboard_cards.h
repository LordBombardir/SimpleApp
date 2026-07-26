#pragma once

#include "imgui.h"

namespace app::ui {

class DashboardCards {
public:
    DashboardCards() = delete;

    /**
     * @brief Renders the Tech Stack card with tech badges grid.
     * @return The height (in pixels) of the card.
     */
    static float DrawTechStackCard(
        ImDrawList* drawList,
        ImVec2      pos,
        float       width,
        ImFont*     fontRegular,
        float       fadeProgress,
        bool        isInteractive = true
    );

    /**
     * @brief Calculates the exact height of the Tech Stack card.
     */
    static float GetTechStackCardHeight(float width, ImFont* fontRegular);

    /**
     * @brief Calculates the exact height of the System Metrics card.
     */
    static float GetSystemMetricsCardHeight(float width);

    /**
     * @brief Renders the System Metrics card with live FPS and hardware info.
     * @return The height (in pixels) of the card.
     */
    static float DrawSystemMetricsCard(
        ImDrawList* drawList,
        ImVec2      pos,
        float       width,
        float       windowWidth,
        float       windowHeight,
        ImFont*     fontRegular,
        float       fadeProgress
    );
};

} // namespace app::ui
