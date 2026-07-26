#include "ui/dashboard_cards.h"
#include "core/text_manager.h"
#include "system/system_metrics.h"
#include <cfloat>
#include <format>
#include <string>
#include <vector>

namespace app::ui {

// clang-format off
static const std::vector<std::string> s_techBadges = {
    "C++", "C# (.NET 10.0)", "Java", "Go", "Node.JS", "PHP",
    "Boost", "protobuf", "cpr", "spdlog", "fmt", "nlohmann_json", "magic_enum",
    "Win32 API", "WinDivert", "OpenSSL", "Botan", "CryptoPP", "LeviLamina",
    "EndStone", "libhat", "imgui", "OpenGL", "DirectX 11", "CMake", "XMake", "vcpkg"
};
// clang-format on

float DashboardCards::GetTechStackCardHeight(float width, ImFont* fontRegular) {
    float contentWidth   = width - 36.0f;
    int   numCols        = (std::max)(1, (std::min)(3, static_cast<int>(contentWidth / 220.0f)));
    bool  isSingleColumn = (numCols < 2);

    float curX    = 18.0f;
    float curY    = 54.0f;
    int   curLine = 0;

    for (const auto& badge : s_techBadges) {
        float badgeW =
            fontRegular ? (fontRegular->CalcTextSizeA(16.0f, FLT_MAX, 0.0f, badge.c_str()).x + 20.0f) : 80.0f;
        float badgeH = 28.0f;

        if (curX + badgeW > width - 18.0f) {
            curX = 18.0f;
            curLine++;
            if (isSingleColumn && curLine >= 2) {
                break;
            }
            curY += badgeH + 8.0f;
        }

        curX += badgeW + 8.0f;
    }

    return (std::max)(125.0f, curY + 28.0f + 16.0f);
}

float DashboardCards::GetSystemMetricsCardHeight(float width) {
    float contentWidth = width - 36.0f;
    float minColWidth  = 220.0f;

    int numCols = static_cast<int>(contentWidth / minColWidth);
    numCols     = (std::max)(1, (std::min)(3, numCols));

    int totalItems = 12;
    if (numCols < 2) {
        totalItems = 8;
    }

    int   rowsPerCol = (totalItems + numCols - 1) / numCols;
    float rowHeight  = 44.0f;

    return 54.0f + rowsPerCol * rowHeight + 14.0f;
}

float DashboardCards::DrawTechStackCard(
    ImDrawList* drawList,
    ImVec2      pos,
    float       width,
    ImFont*     fontRegular,
    float       fadeProgress,
    bool        isInteractive
) {
    std::string techTitle = TextManager::Instance().Get("cards.tech_stack", "My Technology Stack");

    float contentWidth   = width - 36.0f;
    int   numCols        = (std::max)(1, (std::min)(3, static_cast<int>(contentWidth / 220.0f)));
    bool  isSingleColumn = (numCols < 2);

    float  startX   = pos.x + 18.0f;
    float  curX     = startX;
    float  curY     = pos.y + 54.0f;
    int    curLine  = 0;
    ImVec2 mousePos = ImGui::GetMousePos();

    std::vector<std::string> activeBadges;

    for (const auto& badge : s_techBadges) {
        float badgeW = fontRegular->CalcTextSizeA(16.0f, FLT_MAX, 0.0f, badge.c_str()).x + 20.0f;
        float badgeH = 28.0f;

        if (curX + badgeW > pos.x + width - 18.0f) {
            curX = startX;
            curLine++;
            if (isSingleColumn && curLine >= 2) {
                break;
            }
            curY += badgeH + 8.0f;
        }

        activeBadges.push_back(badge);
        curX += badgeW + 8.0f;
    }

    float cardHeight = GetTechStackCardHeight(width, fontRegular);

    ImVec2 cardMin = pos;
    ImVec2 cardMax(pos.x + width, pos.y + cardHeight);

    drawList
        ->AddRectFilled(cardMin, cardMax, ImGui::GetColorU32(ImVec4(0.07f, 0.07f, 0.09f, 0.75f * fadeProgress)), 10.0f);
    drawList->AddRect(
        cardMin,
        cardMax,
        ImGui::GetColorU32(ImVec4(0.22f, 0.22f, 0.25f, 0.50f * fadeProgress)),
        10.0f,
        ImDrawFlags_None,
        1.2f
    );

    drawList->AddText(
        fontRegular,
        19.0f,
        ImVec2(cardMin.x + 18.0f, cardMin.y + 14.0f),
        ImGui::GetColorU32(ImVec4(0.85f, 0.85f, 0.88f, fadeProgress)),
        techTitle.c_str()
    );
    drawList->AddLine(
        ImVec2(cardMin.x + 18.0f, cardMin.y + 42.0f),
        ImVec2(cardMax.x - 18.0f, cardMin.y + 42.0f),
        ImGui::GetColorU32(ImVec4(0.18f, 0.18f, 0.20f, 0.40f * fadeProgress)),
        1.0f
    );

    curX = cardMin.x + 18.0f;
    curY = cardMin.y + 54.0f;

    for (const auto& badge : activeBadges) {
        float badgeW = fontRegular->CalcTextSizeA(16.0f, FLT_MAX, 0.0f, badge.c_str()).x + 20.0f;
        float badgeH = 28.0f;

        if (curX + badgeW > cardMax.x - 18.0f) {
            curX  = cardMin.x + 18.0f;
            curY += badgeH + 8.0f;
        }

        ImVec2 bMin(curX, curY);
        ImVec2 bMax(curX + badgeW, curY + badgeH);
        bool   isBHovered =
            isInteractive
            && (mousePos.x >= bMin.x && mousePos.x <= bMax.x && mousePos.y >= bMin.y && mousePos.y <= bMax.y);

        ImU32 bBg     = isBHovered ? ImGui::GetColorU32(ImVec4(0.20f, 0.20f, 0.24f, 0.85f * fadeProgress))
                                   : ImGui::GetColorU32(ImVec4(0.13f, 0.13f, 0.15f, 0.60f * fadeProgress));
        ImU32 bBorder = isBHovered ? ImGui::GetColorU32(ImVec4(0.40f, 0.40f, 0.45f, 0.75f * fadeProgress))
                                   : ImGui::GetColorU32(ImVec4(0.24f, 0.24f, 0.28f, 0.45f * fadeProgress));

        drawList->AddRectFilled(bMin, bMax, bBg, 6.0f);
        drawList->AddRect(bMin, bMax, bBorder, 6.0f, ImDrawFlags_None, 1.0f);
        drawList->AddText(
            fontRegular,
            16.0f,
            ImVec2(curX + 10.0f, curY + 4.0f),
            ImGui::GetColorU32(ImVec4(0.90f, 0.90f, 0.93f, fadeProgress)),
            badge.c_str()
        );

        curX += badgeW + 8.0f;
    }

    return cardHeight;
}

float DashboardCards::DrawSystemMetricsCard(
    ImDrawList* drawList,
    ImVec2      pos,
    float       width,
    float       windowWidth,
    float       windowHeight,
    ImFont*     fontRegular,
    float       fadeProgress
) {
    std::string sysTitle = TextManager::Instance().Get("cards.system_info", "System Metrics of Your Computer");

    auto&       collector  = ::app::system::SystemMetricsCollector::Instance();
    const auto& staticInfo = collector.GetStaticInfo();
    auto        perfSnap   = collector.GetPerformanceSnapshot();

    float fps = ImGui::GetIO().Framerate;

    struct MetricEntry {
        std::string label;
        std::string value;
    };

    std::string cpuUsageStr = std::format("{:.1f} %", perfSnap.totalCpuUsage);
    std::string procCpuStr  = std::format("{:.1f} %", perfSnap.processCpuUsage);
    std::string sysRamStr   = std::format("{} / {} MB", perfSnap.systemRamUsedMB, perfSnap.systemRamTotalMB);
    std::string procRamStr  = std::format("{} MB", perfSnap.processRamUsedMB);
    std::string resStr      = std::format("{:.0f} x {:.0f}", windowWidth, windowHeight);
    std::string fpsStr      = std::format("{:.0f} FPS", fps);

    std::vector<MetricEntry> metrics = {
        {TextManager::Instance().Get("metrics.os",          "OS / Platform"),    staticInfo.osName      },
        {TextManager::Instance().Get("metrics.arch",        "Architecture"),     staticInfo.architecture},
        {TextManager::Instance().Get("metrics.resolution",  "Resolution"),       resStr                 },
        {TextManager::Instance().Get("metrics.fps",         "Frame Rate"),       fpsStr                 },

        {TextManager::Instance().Get("metrics.cpu",         "CPU"),              staticInfo.cpuModel    },
        {TextManager::Instance().Get("metrics.gpu",         "GPU"),              staticInfo.gpuModel    },
        {TextManager::Instance().Get("metrics.disk",        "Disk"),             staticInfo.diskInfo    },
        {TextManager::Instance().Get("metrics.motherboard", "Motherboard"),      staticInfo.motherboard },

        {TextManager::Instance().Get("metrics.cpu_usage",   "CPU Usage (%)"),    cpuUsageStr            },
        {TextManager::Instance().Get("metrics.process_cpu", "Process CPU (%)"),  procCpuStr             },
        {TextManager::Instance().Get("metrics.system_ram",  "System RAM (MB)"),  sysRamStr              },
        {TextManager::Instance().Get("metrics.process_ram", "Process RAM (MB)"), procRamStr             }
    };

    float contentWidth = width - 36.0f;
    float minColWidth  = 220.0f;

    int numCols = static_cast<int>(contentWidth / minColWidth);
    numCols     = (std::max)(1, (std::min)(3, numCols));

    if (numCols < 2 && metrics.size() > 8) {
        metrics.resize(8);
    }

    int   totalItems = static_cast<int>(metrics.size());
    int   rowsPerCol = (totalItems + numCols - 1) / numCols;
    float colWidth   = contentWidth / static_cast<float>(numCols);
    float rowHeight  = 44.0f;

    float cardHeight = 54.0f + rowsPerCol * rowHeight + 14.0f;

    ImVec2 cardMin = pos;
    ImVec2 cardMax(pos.x + width, pos.y + cardHeight);

    drawList
        ->AddRectFilled(cardMin, cardMax, ImGui::GetColorU32(ImVec4(0.07f, 0.07f, 0.09f, 0.85f * fadeProgress)), 10.0f);
    drawList->AddRect(
        cardMin,
        cardMax,
        ImGui::GetColorU32(ImVec4(0.22f, 0.22f, 0.25f, 0.55f * fadeProgress)),
        10.0f,
        ImDrawFlags_None,
        1.2f
    );

    drawList->AddText(
        fontRegular,
        19.0f,
        ImVec2(cardMin.x + 18.0f, cardMin.y + 14.0f),
        ImGui::GetColorU32(ImVec4(0.85f, 0.85f, 0.88f, fadeProgress)),
        sysTitle.c_str()
    );
    drawList->AddLine(
        ImVec2(cardMin.x + 18.0f, cardMin.y + 42.0f),
        ImVec2(cardMax.x - 18.0f, cardMin.y + 42.0f),
        ImGui::GetColorU32(ImVec4(0.18f, 0.18f, 0.20f, 0.40f * fadeProgress)),
        1.0f
    );

    float startX = cardMin.x + 18.0f;
    float startY = cardMin.y + 52.0f;

    for (int i = 0; i < totalItems; ++i) {
        int col = i / rowsPerCol;
        int row = i % rowsPerCol;

        float cellX = startX + col * colWidth;
        float cellY = startY + row * rowHeight;

        drawList->PushClipRect(ImVec2(cellX, cellY), ImVec2(cellX + colWidth - 10.0f, cellY + rowHeight), true);

        drawList->AddText(
            fontRegular,
            14.0f,
            ImVec2(cellX, cellY),
            ImGui::GetColorU32(ImVec4(0.60f, 0.60f, 0.65f, 0.90f * fadeProgress)),
            metrics[i].label.c_str()
        );

        drawList->AddText(
            fontRegular,
            16.0f,
            ImVec2(cellX, cellY + 18.0f),
            ImGui::GetColorU32(ImVec4(0.92f, 0.94f, 0.98f, 1.0f * fadeProgress)),
            metrics[i].value.c_str()
        );

        drawList->PopClipRect();
    }

    return cardHeight;
}

} // namespace app::ui
