#pragma once

#include "imgui.h"
#include <vector>

namespace app {

struct Point2D {
    ImVec2 pos;
    float  depth;
    bool   valid;
};

class GridBackground {
public:
    GridBackground();
    ~GridBackground() = default;

    void Update(float deltaTime);
    void Render(ImDrawList* drawList, float screenWidth, float screenHeight);

private:
    float m_time = 0.0f;

    static constexpr int   GRID_COLS    = 80;
    static constexpr int   GRID_ROWS    = 60;
    static constexpr float GRID_SPACING = 1.40f;

    float m_cameraPitch    = -0.55f;
    float m_cameraYaw      = 0.40f;
    float m_cameraDistance = 18.0f;
    float m_focalLength    = 800.0f;

    std::vector<Point2D> m_projectedGrid;
    std::vector<float>   m_gridHeights;
};

} // namespace app
