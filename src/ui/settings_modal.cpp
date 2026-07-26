#ifndef NOMINMAX
#define NOMINMAX
#endif

#include "ui/settings_modal.h"
#include "core/text_manager.h"
#include "utils/url_utils.h"
#include "window/window.h"
#include <algorithm>
#include <sstream>
#include <string>

namespace app::ui {

void SettingsModal::Draw(ImDrawList* drawList, float width, float height, ImFont* fontRegular, bool& showSettings) {
    if (!showSettings) return;

    drawList
        ->AddRectFilled(ImVec2(0, 48), ImVec2(width, height), ImGui::GetColorU32(ImVec4(0.02f, 0.02f, 0.03f, 0.70f)));

    float  cardW = (std::min)(480.0f, width - 40.0f);
    float  cardH = (std::min)(360.0f, height - 80.0f);
    ImVec2 cardMin((width - cardW) * 0.5f, 48.0f + (height - 48.0f - cardH) * 0.5f);
    ImVec2 cardMax(cardMin.x + cardW, cardMin.y + cardH);

    ImVec2 mousePos = ImGui::GetMousePos();

    if (ImGui::IsMouseClicked(0)) {
        if (mousePos.x < cardMin.x || mousePos.x > cardMax.x || mousePos.y < cardMin.y || mousePos.y > cardMax.y) {
            TitleBarRect settingsRect = TitleBarLayout::SettingsButton();
            if (!settingsRect.Contains(static_cast<int>(mousePos.x), static_cast<int>(mousePos.y))) {
                showSettings = false;
                return;
            }
        }
    }

    drawList->AddRectFilled(cardMin, cardMax, ImGui::GetColorU32(ImVec4(0.08f, 0.08f, 0.10f, 0.96f)), 12.0f);
    drawList->AddRect(
        cardMin,
        cardMax,
        ImGui::GetColorU32(ImVec4(0.25f, 0.25f, 0.28f, 0.65f)),
        12.0f,
        ImDrawFlags_None,
        1.5f
    );

    ImGui::PushFont(fontRegular);

    // Header Title
    std::string titleText = TextManager::Instance().Get("settings.title", "Settings");
    drawList->AddText(
        fontRegular,
        22.0f,
        ImVec2(cardMin.x + 24.0f, cardMin.y + 20.0f),
        ImGui::GetColorU32(ImVec4(0.95f, 0.95f, 0.96f, 1.0f)),
        titleText.c_str()
    );

    // Close Button
    ImVec2 closeMin(cardMax.x - 44.0f, cardMin.y + 16.0f);
    ImVec2 closeMax(cardMax.x - 16.0f, cardMin.y + 44.0f);
    bool   isCloseHovered =
        (mousePos.x >= closeMin.x && mousePos.x <= closeMax.x && mousePos.y >= closeMin.y && mousePos.y <= closeMax.y);

    if (isCloseHovered) {
        drawList->AddRectFilled(closeMin, closeMax, ImGui::GetColorU32(ImVec4(0.85f, 0.20f, 0.22f, 0.60f)), 6.0f);
        if (ImGui::IsMouseClicked(0)) {
            showSettings = false;
        }
    }
    float clsCx  = (closeMin.x + closeMax.x) * 0.5f;
    float clsCy  = (closeMin.y + closeMax.y) * 0.5f;
    ImU32 clsCol = isCloseHovered ? ImGui::GetColorU32(ImVec4(1.0f, 1.0f, 1.0f, 1.0f))
                                  : ImGui::GetColorU32(ImVec4(0.75f, 0.75f, 0.78f, 0.85f));
    drawList->AddLine(ImVec2(clsCx - 5.0f, clsCy - 5.0f), ImVec2(clsCx + 5.0f, clsCy + 5.0f), clsCol, 1.5f);
    drawList->AddLine(ImVec2(clsCx - 5.0f, clsCy + 5.0f), ImVec2(clsCx + 5.0f, clsCy - 5.0f), clsCol, 1.5f);

    // Separator line
    drawList->AddLine(
        ImVec2(cardMin.x + 24.0f, cardMin.y + 56.0f),
        ImVec2(cardMax.x - 24.0f, cardMin.y + 56.0f),
        ImGui::GetColorU32(ImVec4(0.20f, 0.20f, 0.22f, 0.60f)),
        1.0f
    );

    // Language Switcher
    std::string langLabel = TextManager::Instance().Get("settings.language", "Language");
    drawList->AddText(
        fontRegular,
        18.0f,
        ImVec2(cardMin.x + 24.0f, cardMin.y + 72.0f),
        ImGui::GetColorU32(ImVec4(0.75f, 0.75f, 0.78f, 1.0f)),
        langLabel.c_str()
    );

    std::string currentLang = TextManager::Instance().GetCurrentLanguage();
    auto        availLangs  = TextManager::Instance().GetAvailableLanguages();

    float startX    = cardMin.x + 24.0f;
    float currentY  = cardMin.y + 100.0f;
    float btnWidth  = 196.0f;
    float btnHeight = 38.0f;
    float btnGap    = 16.0f;
    int   colIndex  = 0;

    for (const auto& langCode : availLangs) {
        float xMin = startX + colIndex * (btnWidth + btnGap);
        float xMax = xMin + btnWidth;

        if (xMax > cardMax.x - 24.0f) {
            colIndex  = 0;
            currentY += btnHeight + 10.0f;
            xMin      = startX;
            xMax      = xMin + btnWidth;
        }

        ImVec2 btnMin(xMin, currentY);
        ImVec2 btnMax(xMax, currentY + btnHeight);

        bool isActive = (currentLang == langCode);
        bool isHover =
            (mousePos.x >= btnMin.x && mousePos.x <= btnMax.x && mousePos.y >= btnMin.y && mousePos.y <= btnMax.y);

        ImU32 bg     = isActive ? ImGui::GetColorU32(ImVec4(0.20f, 0.20f, 0.24f, 0.90f))
                                : (isHover ? ImGui::GetColorU32(ImVec4(0.15f, 0.15f, 0.18f, 0.70f))
                                           : ImGui::GetColorU32(ImVec4(0.11f, 0.11f, 0.13f, 0.50f)));
        ImU32 border = isActive ? ImGui::GetColorU32(ImVec4(0.50f, 0.50f, 0.55f, 0.85f))
                                : (isHover ? ImGui::GetColorU32(ImVec4(0.35f, 0.35f, 0.38f, 0.60f))
                                           : ImGui::GetColorU32(ImVec4(0.20f, 0.20f, 0.22f, 0.40f)));

        drawList->AddRectFilled(btnMin, btnMax, bg, 6.0f);
        drawList->AddRect(btnMin, btnMax, border, 6.0f, ImDrawFlags_None, 1.2f);

        std::string langName  = TextManager::Instance().GetForLanguage(langCode, "language.name", langCode);
        std::string labelText = langName;

        drawList->AddText(
            fontRegular,
            18.0f,
            ImVec2(btnMin.x + 14.0f, btnMin.y + 8.0f),
            isActive ? ImGui::GetColorU32(ImVec4(0.98f, 0.98f, 1.0f, 1.0f))
                     : ImGui::GetColorU32(ImVec4(0.70f, 0.70f, 0.75f, 1.0f)),
            labelText.c_str()
        );

        if (isHover && ImGui::IsMouseClicked(0)) {
            TextManager::Instance().SetLanguage(langCode);
        }

        colIndex++;
    }

    float authorSectionY = currentY + btnHeight + 22.0f;

    // Author Info
    std::string authorLabel = TextManager::Instance().Get("settings.author", "About Author");
    drawList->AddText(
        fontRegular,
        18.0f,
        ImVec2(cardMin.x + 24.0f, authorSectionY),
        ImGui::GetColorU32(ImVec4(0.75f, 0.75f, 0.78f, 1.0f)),
        authorLabel.c_str()
    );

    std::string        authorInfo = TextManager::Instance().Get("settings.author_info");
    std::istringstream stream(authorInfo);
    std::string        line;
    float              lineY    = authorSectionY + 30.0f;
    float              fontSize = 17.0f;

    while (std::getline(stream, line)) {
        std::string prefix, url;
        if (utils::UrlUtils::ExtractUrl(line, prefix, url)) {
            ImVec2 prefSize = fontRegular->CalcTextSizeA(fontSize, FLT_MAX, 0.0f, prefix.c_str());
            ImVec2 urlSize  = fontRegular->CalcTextSizeA(fontSize, FLT_MAX, 0.0f, url.c_str());

            float prefX = cardMin.x + 24.0f;
            float urlX  = prefX + prefSize.x;

            if (!prefix.empty()) {
                drawList->AddText(
                    fontRegular,
                    fontSize,
                    ImVec2(prefX, lineY),
                    ImGui::GetColorU32(ImVec4(0.85f, 0.85f, 0.88f, 0.85f)),
                    prefix.c_str()
                );
            }

            ImVec2 urlMin(urlX, lineY);
            ImVec2 urlMax(urlX + urlSize.x, lineY + fontSize + 2.0f);
            bool   isHover =
                (mousePos.x >= urlMin.x && mousePos.x <= urlMax.x && mousePos.y >= urlMin.y && mousePos.y <= urlMax.y);

            ImU32 urlColor = isHover ? ImGui::GetColorU32(ImVec4(0.45f, 0.75f, 1.0f, 1.0f))
                                     : ImGui::GetColorU32(ImVec4(0.35f, 0.65f, 0.95f, 0.90f));
            drawList->AddText(fontRegular, fontSize, ImVec2(urlX, lineY), urlColor, url.c_str());
            drawList->AddLine(
                ImVec2(urlX, lineY + fontSize - 1.0f),
                ImVec2(urlX + urlSize.x, lineY + fontSize - 1.0f),
                urlColor,
                1.0f
            );

            if (isHover) {
                ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);
                if (ImGui::IsMouseClicked(0)) {
                    utils::UrlUtils::OpenURL(url);
                }
            }
        } else {
            drawList->AddText(
                fontRegular,
                fontSize,
                ImVec2(cardMin.x + 24.0f, lineY),
                ImGui::GetColorU32(ImVec4(0.85f, 0.85f, 0.88f, 0.85f)),
                line.c_str()
            );
        }

        lineY += fontSize + 6.0f;
    }

    ImGui::PopFont();
}

} // namespace app::ui
