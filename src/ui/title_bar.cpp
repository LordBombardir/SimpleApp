#include "ui/title_bar.h"
#include "core/text_manager.h"
#include "window/window.h"
#include <cmath>

namespace app::ui {

void TitleBar::DrawSettingsIcon(ImDrawList* drawList, ImVec2 center, float size, ImU32 color) {
    float scale = size / 24.0f;

    drawList->AddCircle(center, 3.0f * scale, color, 16, 1.5f * scale);

    const int numTeeth = 8;
    ImVec2    gearPts[numTeeth * 4];
    float     rOuter = 9.2f * scale;
    float     rInner = 6.8f * scale;

    for (int k = 0; k < numTeeth; ++k) {
        float angleCenter = k * (2.0f * 3.1415926535f / numTeeth);
        float dAngle      = 0.22f;

        float a0 = angleCenter - dAngle * 1.5f;
        float a1 = angleCenter - dAngle * 0.7f;
        float a2 = angleCenter + dAngle * 0.7f;
        float a3 = angleCenter + dAngle * 1.5f;

        gearPts[k * 4 + 0] = ImVec2(center.x + std::cos(a0) * rInner, center.y + std::sin(a0) * rInner);
        gearPts[k * 4 + 1] = ImVec2(center.x + std::cos(a1) * rOuter, center.y + std::sin(a1) * rOuter);
        gearPts[k * 4 + 2] = ImVec2(center.x + std::cos(a2) * rOuter, center.y + std::sin(a2) * rOuter);
        gearPts[k * 4 + 3] = ImVec2(center.x + std::cos(a3) * rInner, center.y + std::sin(a3) * rInner);
    }
    drawList->AddPolyline(gearPts, numTeeth * 4, color, ImDrawFlags_Closed, 1.5f * scale);
}

bool TitleBar::Draw(
    ImDrawList* drawList,
    Window*     window,
    float       width,
    ImFont*     fontRegular,
    bool        showSettingsActive,
    bool        disableMaximize
) {
    bool settingsToggled = false;

    drawList->AddRectFilled(ImVec2(0, 0), ImVec2(width, 48), ImGui::GetColorU32(ImVec4(0.04f, 0.04f, 0.05f, 1.0f)));
    drawList->AddLine(ImVec2(0, 48), ImVec2(width, 48), ImGui::GetColorU32(ImVec4(0.20f, 0.20f, 0.22f, 0.60f)), 1.0f);

    TitleBarRect settingsRect = TitleBarLayout::SettingsButton();
    ImVec2       btnMin(static_cast<float>(settingsRect.left), static_cast<float>(settingsRect.top));
    ImVec2       btnMax(static_cast<float>(settingsRect.right), static_cast<float>(settingsRect.bottom));
    ImVec2       mousePos          = ImGui::GetMousePos();
    bool         isHoveredSettings = settingsRect.Contains(static_cast<int>(mousePos.x), static_cast<int>(mousePos.y));

    if (isHoveredSettings) {
        drawList->AddRectFilled(btnMin, btnMax, ImGui::GetColorU32(ImVec4(0.18f, 0.18f, 0.22f, 0.65f)), 6.0f);
        drawList->AddRect(
            btnMin,
            btnMax,
            ImGui::GetColorU32(ImVec4(0.32f, 0.32f, 0.36f, 0.50f)),
            6.0f,
            ImDrawFlags_None,
            1.0f
        );
        if (ImGui::IsMouseClicked(0)) {
            settingsToggled = true;
        }
    }

    ImU32 iconColorSettings = isHoveredSettings ? ImGui::GetColorU32(ImVec4(0.95f, 0.95f, 0.98f, 1.0f))
                                                : ImGui::GetColorU32(ImVec4(0.70f, 0.70f, 0.75f, 0.85f));
    DrawSettingsIcon(drawList, ImVec2(24.0f, 24.0f), 20.0f, iconColorSettings);

    std::string titlebarText = TextManager::Instance().Get("titlebar.name");
    ImGui::PushFont(fontRegular);
    drawList
        ->AddText(fontRegular, 20.0f, ImVec2(54.0f, 14.0f), ImGui::GetColorU32(ImGuiCol_Text), titlebarText.c_str());
    ImGui::PopFont();

    if (window) {
        int   hover     = window->GetHoveredButton();
        int   press     = window->GetPressedButton();
        ImU32 iconColor = ImGui::GetColorU32(ImGuiCol_Text);

        if (disableMaximize) {
            if (hover == 2) hover = 0;
            if (press == 2) press = 0;
        }

#ifndef _WIN32
        bool isHoverMin     = TitleBarLayout::MinimizeButton(static_cast<int>(width))
                                  .Contains(static_cast<int>(mousePos.x), static_cast<int>(mousePos.y));
        bool isHoverMax     = TitleBarLayout::MaximizeButton(static_cast<int>(width))
                                  .Contains(static_cast<int>(mousePos.x), static_cast<int>(mousePos.y));
        bool isHoverClose   = TitleBarLayout::CloseButton(static_cast<int>(width))
                                  .Contains(static_cast<int>(mousePos.x), static_cast<int>(mousePos.y));
        bool isTitleBarArea = mousePos.y >= 0.0f && mousePos.y < 48.0f && !isHoveredSettings && !isHoverMin
                           && !isHoverMax && !isHoverClose;

        if (hover == 0) {
            if (isHoverMin) hover = 1;
            else if (isHoverMax && !disableMaximize) hover = 2;
            else if (isHoverClose) hover = 3;
        }

        if (press == 0 && ImGui::IsMouseDown(0)) {
            if (isHoverMin) press = 1;
            else if (isHoverMax && !disableMaximize) press = 2;
            else if (isHoverClose) press = 3;
        }

        if (ImGui::IsMouseDoubleClicked(0) && isTitleBarArea) {
            if (!disableMaximize) {
                window->MaximizeOrRestore();
            }
        } else if (ImGui::IsMouseClicked(0)) {
            if (isHoverMin) {
                window->Minimize();
            } else if (isHoverMax && !disableMaximize) {
                window->MaximizeOrRestore();
            } else if (isHoverClose) {
                window->Close();
            } else if (isTitleBarArea) {
                window->StartDragging();
            }
        }

        window->UpdateDragging();
#endif

        // Minimize Button
        ImVec2 minStart(width - 138, 0);
        ImVec2 minEnd(width - 92, 48);
        if (hover == 1) {
            drawList->AddRectFilled(
                minStart,
                minEnd,
                ImGui::GetColorU32(press == 1 ? ImVec4(0.24f, 0.28f, 0.38f, 0.6f) : ImVec4(0.2f, 0.22f, 0.28f, 0.4f))
            );
        }
        float minCx = width - 115.0f;
        float minCy = 24.0f;
        drawList->AddLine(ImVec2(minCx - 5.0f, minCy), ImVec2(minCx + 5.0f, minCy), iconColor, 1.5f);

        // Maximize/Restore Button
        ImVec2 maxStart(width - 92, 0);
        ImVec2 maxEnd(width - 46, 48);
        if (hover == 2 && !disableMaximize) {
            drawList->AddRectFilled(
                maxStart,
                maxEnd,
                ImGui::GetColorU32(press == 2 ? ImVec4(0.24f, 0.28f, 0.38f, 0.6f) : ImVec4(0.2f, 0.22f, 0.28f, 0.4f))
            );
        }
        float cx = width - 69.0f;
        float cy = 24.0f;

        ImU32 maxIconColor = disableMaximize ? ImGui::GetColorU32(ImVec4(0.40f, 0.40f, 0.45f, 0.35f)) : iconColor;

        if (window->IsMaximized()) {
            float scale = 0.50f;
            float ox    = cx - 12.0f * scale;
            float oy    = cy - 12.0f * scale;

            auto MapPt = [&](float x, float y) { return ImVec2(ox + x * scale, oy + y * scale); };

            ImVec2 pathPts[] = {
                MapPt(20.0f, 16.0f),
                MapPt(22.0f, 16.0f),
                MapPt(22.0f, 4.0f),
                MapPt(20.0f, 2.0f),
                MapPt(10.0f, 2.0f),
                MapPt(8.0f, 2.0f),
                MapPt(8.0f, 4.0f)
            };
            drawList->AddPolyline(pathPts, 7, maxIconColor, ImDrawFlags_None, 1.3f);

            ImVec2 rectMin = MapPt(2.0f, 8.0f);
            ImVec2 rectMax = MapPt(16.0f, 22.0f);

            ImU32 bgFill = (hover == 2 && !disableMaximize)
                             ? ImGui::GetColorU32(
                                   press == 2 ? ImVec4(0.24f, 0.28f, 0.38f, 1.0f) : ImVec4(0.18f, 0.20f, 0.26f, 1.0f)
                               )
                             : ImGui::GetColorU32(ImVec4(0.05f, 0.05f, 0.07f, 1.0f));

            drawList->AddRectFilled(rectMin, rectMax, bgFill, 1.5f);
            drawList->AddRect(rectMin, rectMax, maxIconColor, 1.5f, ImDrawFlags_None, 1.3f);
        } else {
            ImVec2 rectMin(cx - 5.0f, cy - 5.0f);
            ImVec2 rectMax(cx + 5.0f, cy + 5.0f);
            drawList->AddRect(rectMin, rectMax, maxIconColor, 1.5f, ImDrawFlags_None, 1.5f);
        }

        // Close Button
        ImVec2 closeStart(width - 46, 0);
        ImVec2 closeEnd(width, 48);
        if (hover == 3) {
            drawList->AddRectFilled(
                closeStart,
                closeEnd,
                ImGui::GetColorU32(press == 3 ? ImVec4(0.75f, 0.15f, 0.15f, 0.9f) : ImVec4(0.85f, 0.2f, 0.2f, 0.8f))
            );
        }
        float closeCx   = width - 23.0f;
        float closeCy   = 24.0f;
        ImU32 closeIcon = (hover == 3) ? ImGui::GetColorU32(ImVec4(1.0f, 1.0f, 1.0f, 1.0f)) : iconColor;
        drawList
            ->AddLine(ImVec2(closeCx - 5.0f, closeCy - 5.0f), ImVec2(closeCx + 5.0f, closeCy + 5.0f), closeIcon, 1.5f);
        drawList
            ->AddLine(ImVec2(closeCx - 5.0f, closeCy + 5.0f), ImVec2(closeCx + 5.0f, closeCy - 5.0f), closeIcon, 1.5f);
    }

    return settingsToggled;
}

} // namespace app::ui
