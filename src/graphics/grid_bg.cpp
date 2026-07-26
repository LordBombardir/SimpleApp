#include "grid_bg.h"
#include <algorithm>
#include <cmath>

namespace app {

GridBackground::GridBackground() {
    m_time = 0.0f;
    m_projectedGrid.resize(GRID_COLS * GRID_ROWS);
    m_gridHeights.resize(GRID_COLS * GRID_ROWS);
}

void GridBackground::Update(float deltaTime) { m_time += deltaTime; }

void GridBackground::Render(ImDrawList* drawList, float screenWidth, float screenHeight) {
    float yaw   = m_cameraYaw + std::sin(m_time * 0.06f) * 0.04f;
    float pitch = m_cameraPitch + std::cos(m_time * 0.04f) * 0.02f;

    float cosY = std::cos(yaw);
    float sinY = std::sin(yaw);
    float cosP = std::cos(pitch);
    float sinP = std::sin(pitch);

    auto GetPt = [&](int i, int j) -> Point2D& { return m_projectedGrid[i * GRID_ROWS + j]; };
    auto GetH  = [&](int i, int j) -> float& { return m_gridHeights[i * GRID_ROWS + j]; };

    for (int i = 0; i < GRID_COLS; ++i) {
        float x = (i - GRID_COLS / 2) * GRID_SPACING;
        for (int j = 0; j < GRID_ROWS; ++j) {
            float y = (j - GRID_ROWS / 2) * GRID_SPACING;

            float swell1 = std::sin((x * 0.707f + y * 0.707f) * 0.32f - m_time * 0.60f) * 0.70f;
            float swell2 = std::cos((x * 0.866f - y * 0.500f) * 0.25f + m_time * 0.45f) * 0.40f;

            float z    = swell1 + swell2;
            GetH(i, j) = z;

            float x1 = x * cosY - z * sinY;
            float z1 = x * sinY + z * cosY;

            float y2 = y * cosP - z1 * sinP;
            float z2 = y * sinP + z1 * cosP;

            float zView = z2 + m_cameraDistance;

            if (zView < 0.2f) {
                GetPt(i, j).valid = false;
            } else {
                GetPt(i, j).valid = true;
                float px          = screenWidth * 0.5f + (x1 * m_focalLength) / zView;
                float py          = screenHeight * 0.55f + (y2 * m_focalLength) / zView;

                GetPt(i, j).pos   = ImVec2(px, py);
                GetPt(i, j).depth = zView;
            }
        }
    }

    for (int i = 0; i < GRID_COLS; ++i) {
        for (int j = 0; j < GRID_ROWS; ++j) {
            const auto& ptA = GetPt(i, j);
            if (!ptA.valid) continue;

            ImVec2 pA      = ptA.pos;
            float  depthA  = ptA.depth;
            float  heightA = GetH(i, j);

            float minDepth    = m_cameraDistance - 14.0f;
            float maxDepth    = m_cameraDistance + 28.0f;
            float depthFactor = 1.0f - ((depthA - minDepth) / (maxDepth - minDepth));
            depthFactor       = std::max(0.0f, std::min(1.0f, depthFactor));

            if (depthFactor <= 0.005f) continue;

            float heightFactor = (heightA + 1.2f) / 2.4f;
            heightFactor       = std::max(0.0f, std::min(1.0f, heightFactor));

            float r     = (16.0f * (1.0f - heightFactor) + 110.0f * heightFactor) / 255.0f;
            float g     = (16.0f * (1.0f - heightFactor) + 125.0f * heightFactor) / 255.0f;
            float b     = (20.0f * (1.0f - heightFactor) + 145.0f * heightFactor) / 255.0f;
            float alpha = depthFactor * (0.06f + heightFactor * 0.18f);

            ImU32 color = ImGui::GetColorU32(ImVec4(r, g, b, alpha));

            if (i < GRID_COLS - 1) {
                const auto& ptRight = GetPt(i + 1, j);
                if (ptRight.valid) {
                    drawList->AddLine(pA, ptRight.pos, color, 1.0f);
                }
            }

            if (j < GRID_ROWS - 1) {
                const auto& ptBottom = GetPt(i, j + 1);
                if (ptBottom.valid) {
                    drawList->AddLine(pA, ptBottom.pos, color, 1.0f);
                }
            }

            if (heightA > 0.40f) {
                float pointAlpha = depthFactor * (heightA - 0.40f) * 0.90f;
                pointAlpha       = std::min(0.60f, pointAlpha);
                ImU32 pointCol   = ImGui::GetColorU32(ImVec4(0.85f, 0.90f, 0.98f, pointAlpha));
                drawList->AddCircleFilled(pA, 1.2f * (0.5f + depthFactor * 0.5f), pointCol, 6);
            }
        }
    }
}

} // namespace app
