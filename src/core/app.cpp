#ifndef NOMINMAX
#define NOMINMAX
#endif

#include "core/app.h"
#include "core/text_manager.h"
#include "resources/embedded_resources.h"
#include "ui/dashboard_cards.h"
#include "ui/settings_modal.h"
#include "ui/text_renderer.h"
#include "ui/title_bar.h"
#include "utils/utf8_utils.h"
#include "window/native_window.h"
#include "window/window.h"

#ifdef _WIN32
#include "imgui_impl_dx11.h"
#include "imgui_impl_win32.h"
#include <d3d11.h>
#else
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <GLFW/glfw3.h>
#include <GL/gl.h>
#endif

#include <mimalloc.h>

#include <algorithm>
#include <cmath>

namespace app {

App::App(std::shared_ptr<Window> window) : m_window(window) {
    TextManager::Instance().Initialize();
    InitImGui();
    LoadFonts();
    mi_collect(true);
}

App::~App() {
#ifdef _WIN32
    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
#else
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
#endif
    ImGui::DestroyContext();
}

void App::InitImGui() {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.IniFilename  = nullptr;

    ImGui::StyleColorsDark();

    ImGuiStyle& style               = ImGui::GetStyle();
    style.WindowRounding            = 0.0f;
    style.WindowBorderSize          = 0.0f;
    style.WindowPadding             = ImVec2(0.0f, 0.0f);
    style.Colors[ImGuiCol_WindowBg] = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);

#ifdef _WIN32
    ImGui_ImplWin32_Init(m_window->GetImpl()->GetHWND());
    ImGui_ImplDX11_Init(m_window->GetImpl()->GetD3DDevice(), m_window->GetImpl()->GetD3DDeviceContext());
#else
    ImGui_ImplGlfw_InitForOpenGL(m_window->GetImpl()->GetGLFWWindow(), true);
    ImGui_ImplOpenGL3_Init("#version 330");
#endif
}

void App::LoadFonts() {
    ImGuiIO& io = ImGui::GetIO();

    // clang-format off
    static const ImWchar ranges[] = {
        0x0020, 0x00FF, // Basic Latin + Latin Supplement
        0x0400, 0x052F, // Cyrillic + Cyrillic Supplement
        0x2DE0, 0x2DFF, // Cyrillic Extended-A
        0xA640, 0xA69F, // Cyrillic Extended-B
        0,
    };
    // clang-format on

    ImFontConfig fontConfig;
    fontConfig.FontDataOwnedByAtlas = false;
    fontConfig.OversampleH          = 1;
    fontConfig.OversampleV          = 1;
    fontConfig.PixelSnapH           = true;

    const unsigned char* fontData     = nullptr;
    size_t               fontDataSize = 0;

    if (GetEmbeddedFontJetBrainsMono(fontData, fontDataSize) && fontData && fontDataSize > 0) {
        m_fonts[FontType::Regular] = io.Fonts->AddFontFromMemoryTTF(
            const_cast<unsigned char*>(fontData),
            static_cast<int>(fontDataSize),
            20.0f,
            &fontConfig,
            ranges
        );

        m_fonts[FontType::Title] = io.Fonts->AddFontFromMemoryTTF(
            const_cast<unsigned char*>(fontData),
            static_cast<int>(fontDataSize),
            56.0f,
            &fontConfig,
            ranges
        );

        m_fonts[FontType::TitleSmall] = io.Fonts->AddFontFromMemoryTTF(
            const_cast<unsigned char*>(fontData),
            static_cast<int>(fontDataSize),
            26.0f,
            &fontConfig,
            ranges
        );
    }

    if (m_fonts[FontType::Regular] == nullptr) {
        m_fonts[FontType::Regular] = io.Fonts->AddFontDefault();
    }
    if (m_fonts[FontType::Title] == nullptr) {
        m_fonts[FontType::Title] = m_fonts[FontType::Regular];
    }
    if (m_fonts[FontType::TitleSmall] == nullptr) {
        m_fonts[FontType::TitleSmall] = m_fonts[FontType::Regular];
    }

    io.Fonts->Build();
}

ImFont* App::GetFont(FontType type) const {
    auto it = m_fonts.find(type);
    if (it != m_fonts.end()) {
        return it->second;
    }

    return nullptr;
}

void App::Update(float deltaTime) {
    m_animTime += deltaTime;
    m_gridBackground.Update(deltaTime);

    if (m_window) {
        const float TEXT_ANIMATION_DURATION = 1.8f + 0.5f + 1.5f;

        bool isAnimationFinished = (m_animTime >= TEXT_ANIMATION_DURATION);
        m_window->SetMaximizeEnabled(isAnimationFinished);
        m_window->SetResizeEnabled(isAnimationFinished);
    }
}

void App::DrawContent(float width, float height, bool isInteractive) {
    ImDrawList* drawList = ImGui::GetWindowDrawList();

    drawList->PushClipRect(ImVec2(0.0f, 48.0f), ImVec2(width, height), true);

    const float TYPING_DURATION     = 1.8f;
    const float DELAY_DURATION      = 0.5f;
    const float MOVE_SCALE_DURATION = 1.5f;

    float time = m_animTime;

    std::string fullTitleText = TextManager::Instance().Get("app.title");
    std::string fullBodyText  = TextManager::Instance().Get("app.body");

    int  totalUtf8Chars = utils::Utf8Utils::GetCharCount(fullTitleText);
    int  visibleChars   = totalUtf8Chars;
    bool showCursor     = true;

    if (time < TYPING_DURATION) {
        float progress = time / TYPING_DURATION;
        visibleChars   = static_cast<int>(totalUtf8Chars * progress);
        showCursor     = (static_cast<int>(time * 5.0f) % 2 == 0);
    } else {
        showCursor = (static_cast<int>((time - TYPING_DURATION) * 2.0f) % 2 == 0);
    }

    std::string currentTitle = utils::Utf8Utils::GetPrefix(fullTitleText, visibleChars);
    if (showCursor && time < TYPING_DURATION + DELAY_DURATION + MOVE_SCALE_DURATION) {
        currentTitle += "|";
    }

    float topLimit     = 48.0f;
    float usableHeight = height - topLimit;

    float titleY     = height * 0.45f - 24.0f;
    float titleScale = 1.0f;

    float moveStartTime = TYPING_DURATION + DELAY_DURATION;
    float moveEndTime   = moveStartTime + MOVE_SCALE_DURATION;

    if (time >= moveStartTime) {
        float moveProgress = (time - moveStartTime) / MOVE_SCALE_DURATION;
        moveProgress       = (std::max)(0.0f, (std::min)(1.0f, moveProgress));

        float smoothT = moveProgress * moveProgress * (3.0f - 2.0f * moveProgress);

        titleY     = (height * 0.45f - 24.0f) * (1.0f - smoothT) + 85.0f * smoothT;
        titleScale = 1.0f * (1.0f - smoothT) + 0.45f * smoothT;
    }

    bool isWideScreen = (width >= 960.0f);
    int  maxPages     = isWideScreen ? 1 : 2;

    if (isWideScreen) {
        m_targetPageIndex = 0;
        m_targetScrollY   = 0.0f;
        m_scrollY         = 0.0f;
    } else {
        if (time >= moveEndTime) {
            float wheel = ImGui::GetIO().MouseWheel;
            if (wheel < -0.1f && isInteractive) {
                if (m_targetPageIndex < maxPages - 1) {
                    m_targetPageIndex++;
                }
            } else if (wheel > 0.1f && isInteractive) {
                if (m_targetPageIndex > 0) {
                    m_targetPageIndex--;
                }
            }
        } else {
            m_targetPageIndex = 0;
        }
    }

    ImFont* fontTitle      = GetFont(FontType::Title);
    ImFont* fontTitleSmall = GetFont(FontType::TitleSmall);

    ImFont* activeFont      = (titleScale < 0.99f && fontTitleSmall) ? fontTitleSmall : fontTitle;
    float   fontNativeSize  = (titleScale < 0.99f && fontTitleSmall) ? 26.0f : 56.0f;
    float   currentFontSize = fontNativeSize * (titleScale < 0.99f ? (56.0f * titleScale / 26.0f) : 1.0f);

    if (titleScale <= 0.46f && fontTitleSmall) {
        activeFont      = fontTitleSmall;
        currentFontSize = 26.0f;
    }

    ImGui::PushFont(activeFont);
    ImVec2 titleSize = activeFont->CalcTextSizeA(currentFontSize, FLT_MAX, 0.0f, currentTitle.c_str());
    ImGui::PopFont();

    ImFont* fontRegular = GetFont(FontType::Regular);

    float cardWidth = isWideScreen ? ((std::min)(1150.0f, width - 80.0f) * 0.60f) : (std::min)(780.0f, width - 40.0f);
    float bioHeight = fontRegular ? fontRegular->CalcTextSizeA(20.0f, cardWidth, 0.0f, fullBodyText.c_str()).y : 240.0f;

    float page2Offset = (std::max)(usableHeight, titleSize.y + bioHeight + 80.0f);
    if (!isWideScreen) {
        m_targetPageIndex = (std::max)(0, (std::min)(maxPages - 1, m_targetPageIndex));
        m_targetScrollY   = (m_targetPageIndex == 1) ? page2Offset : 0.0f;
        if (m_wasMinimized) {
            m_scrollY      = m_targetScrollY;
            m_wasMinimized = false;
        }
    } else {
        m_wasMinimized = false;
    }

    float deltaTime  = ImGui::GetIO().DeltaTime;
    float scrollDiff = m_targetScrollY - m_scrollY;
    if (std::abs(scrollDiff) < 0.01f) {
        m_scrollY = m_targetScrollY;
    } else {
        m_scrollY += scrollDiff * (std::min)(1.0f, deltaTime * 2.8f);
    }

    float totalContentWidth = (std::min)(1550.0f, width - 80.0f);
    float gap               = (std::min)(40.0f, totalContentWidth * 0.03f);
    float rightCardWidth    = isWideScreen ? (std::min)(780.0f, (totalContentWidth - gap) * 0.53f) : 0.0f;
    float leftCardWidth     = isWideScreen ? (totalContentWidth - gap - rightCardWidth) : 0.0f;
    float wideStartX        = isWideScreen ? ((width - totalContentWidth) * 0.5f) : 0.0f;

    float  scrolledTitleY = titleY - m_scrollY;
    float  titleX = (isWideScreen && time >= moveStartTime) ? (wideStartX + (leftCardWidth - titleSize.x) * 0.5f)
                                                            : ((width - titleSize.x) * 0.5f);
    ImVec2 titlePos(titleX, scrolledTitleY);

    ImGui::PushFont(activeFont);
    drawList->AddText(
        activeFont,
        currentFontSize,
        ImVec2(titlePos.x + 1, titlePos.y + 1),
        ImGui::GetColorU32(ImVec4(0, 0, 0, 0.5f)),
        currentTitle.c_str()
    );
    drawList->AddText(activeFont, currentFontSize, titlePos, ImGui::GetColorU32(ImGuiCol_Text), currentTitle.c_str());
    ImGui::PopFont();

    if (time >= moveEndTime) {
        float       fadeTime      = time - moveEndTime;
        const float FADE_DURATION = 1.2f;
        float       fadeProgress  = fadeTime / FADE_DURATION;
        fadeProgress              = (std::max)(0.0f, (std::min)(1.0f, fadeProgress));

        ImGui::PushFont(fontRegular);

        if (isWideScreen) {
            float startX = wideStartX;
            float startY = 85.0f;

            ImVec2 bodyPos(startX, startY + titleSize.y + 24.0f);
            ImU32  textCol = ImGui::GetColorU32(ImVec4(0.92f, 0.94f, 0.98f, fadeProgress));
            ui::TextRenderer::RenderJustifiedText(
                drawList,
                fullBodyText,
                bodyPos,
                leftCardWidth,
                fontRegular,
                20.0f,
                textCol
            );

            float rightX = startX + leftCardWidth + gap;
            float rightY = startY;

            float card1H = ui::DashboardCards::DrawTechStackCard(
                drawList,
                ImVec2(rightX, rightY),
                rightCardWidth,
                fontRegular,
                fadeProgress,
                isInteractive
            );
            ui::DashboardCards::DrawSystemMetricsCard(
                drawList,
                ImVec2(rightX, rightY + card1H + 16.0f),
                rightCardWidth,
                width,
                height,
                fontRegular,
                fadeProgress
            );

        } else {
            float startX = (width - cardWidth) * 0.5f;

            float  startY = 85.0f + titleSize.y + 24.0f - m_scrollY;
            ImVec2 bodyPos(startX, startY);
            ImU32  textCol = ImGui::GetColorU32(ImVec4(0.92f, 0.94f, 0.98f, fadeProgress));
            ui::TextRenderer::RenderJustifiedText(
                drawList,
                fullBodyText,
                bodyPos,
                cardWidth,
                fontRegular,
                20.0f,
                textCol
            );

            float card1H      = ui::DashboardCards::GetTechStackCardHeight(cardWidth, fontRegular);
            float card2H      = ui::DashboardCards::GetSystemMetricsCardHeight(cardWidth);
            float totalCardsH = card1H + 16.0f + card2H;

            float topPadding   = (std::max)(20.0f, (usableHeight - totalCardsH) * 0.5f);
            float page2CenterY = page2Offset + topLimit + topPadding;

            float card1Y = page2CenterY - m_scrollY;
            ui::DashboardCards::DrawTechStackCard(
                drawList,
                ImVec2(startX, card1Y),
                cardWidth,
                fontRegular,
                fadeProgress,
                isInteractive
            );

            float card2Y = card1Y + card1H + 16.0f;
            ui::DashboardCards::DrawSystemMetricsCard(
                drawList,
                ImVec2(startX, card2Y),
                cardWidth,
                width,
                height,
                fontRegular,
                fadeProgress
            );
        }

        ImGui::PopFont();
    }

    drawList->PopClipRect();
}

void App::Render() {
#ifdef _WIN32
    ImGui_ImplDX11_NewFrame();
    ImGui_ImplWin32_NewFrame();
#else
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
#endif
    ImGui::NewFrame();

    int width = 0, height = 0;
    if (m_window) {
        m_window->GetSize(width, height);
    }

    bool isWindowMinimized = (width <= 0 || height <= 0 || (m_window && m_window->IsMinimized()));
    if (isWindowMinimized) {
        m_wasMinimized = true;
        ImGui::EndFrame();
        return;
    }

    float fWidth  = static_cast<float>(width);
    float fHeight = static_cast<float>(height);

    m_gridBackground.Render(ImGui::GetBackgroundDrawList(), fWidth, fHeight);

    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGui::SetNextWindowSize(ImVec2(fWidth, fHeight));
    ImGui::Begin(
        "FullscreenOverlay",
        nullptr,
        ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse
            | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus
            | ImGuiWindowFlags_NoBackground
    );

    ImDrawList* drawList = ImGui::GetWindowDrawList();
    ImFont*     fontReg  = GetFont(FontType::Regular);

    bool isContentInteractive = !HasActiveModal();
    DrawContent(fWidth, fHeight, isContentInteractive);

    const float TEXT_ANIMATION_DURATION = 1.8f + 0.5f + 1.5f;
    bool        disableMaximize         = (m_animTime < TEXT_ANIMATION_DURATION);

    bool isSettingsActive = IsModalOpen(ModalType::Settings);
    if (ui::TitleBar::Draw(drawList, m_window.get(), fWidth, fontReg, isSettingsActive, disableMaximize)) {
        ToggleModal(ModalType::Settings);
    }

    bool showSettingsModal = IsModalOpen(ModalType::Settings);
    ui::SettingsModal::Draw(drawList, fWidth, fHeight, fontReg, showSettingsModal);
    if (!showSettingsModal && IsModalOpen(ModalType::Settings)) {
        CloseModal();
    }

    ImGui::End();

    ImGui::Render();

#ifdef _WIN32
    ID3D11DeviceContext*    context       = m_window->GetImpl()->GetD3DDeviceContext();
    ID3D11RenderTargetView* rtView        = m_window->GetImpl()->GetMainRenderTargetView();
    const float             clearColor[4] = {0.02f, 0.02f, 0.03f, 1.0f};
    context->OMSetRenderTargets(1, &rtView, nullptr);
    context->ClearRenderTargetView(rtView, clearColor);
    ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
#else
    int fbW = width, fbH = height;
    if (m_window && m_window->GetImpl() && m_window->GetImpl()->GetGLFWWindow()) {
        glfwGetFramebufferSize(m_window->GetImpl()->GetGLFWWindow(), &fbW, &fbH);
    }
    glViewport(0, 0, fbW, fbH);
    glClearColor(0.035f, 0.035f, 0.05f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    ImFontAtlas* atlas = ImGui::GetIO().Fonts;
    if (atlas && atlas->IsBuilt()) {
        ImTextureID fontTex = atlas->TexRef.GetTexID();
        if (fontTex != 0) {
            GLuint fontTexId = static_cast<GLuint>((uintptr_t)fontTex);
            glBindTexture(GL_TEXTURE_2D, fontTexId);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
#ifdef GL_TEXTURE_MAX_ANISOTROPY_EXT
            glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, 16.0f);
#endif
        }
    }

    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
#endif
}

} // namespace app
