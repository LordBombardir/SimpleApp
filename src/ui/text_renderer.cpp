#include "ui/text_renderer.h"
#include <cfloat>
#include <vector>

namespace app::ui {

float TextRenderer::RenderJustifiedText(
    ImDrawList*        drawList,
    const std::string& text,
    ImVec2             pos,
    float              width,
    ImFont*            font,
    float              fontSize,
    ImU32              color
) {
    if (text.empty()) return 0.0f;

    std::vector<std::string> paragraphs;
    size_t                   start = 0;
    size_t                   end   = text.find('\n');
    while (end != std::string::npos) {
        paragraphs.push_back(text.substr(start, end - start));
        start = end + 1;
        end   = text.find('\n', start);
    }
    paragraphs.push_back(text.substr(start));

    float yOffset    = 0.0f;
    float spaceWidth = font->CalcTextSizeA(fontSize, FLT_MAX, 0.0f, " ").x;

    for (const auto& para : paragraphs) {
        if (para.empty()) {
            yOffset += fontSize * 1.0f;
            continue;
        }

        std::vector<std::string> words;
        std::string              currentWord;
        size_t                   i   = 0;
        size_t                   len = para.length();

        while (i < len) {
            unsigned char c       = static_cast<unsigned char>(para[i]);
            int           charLen = 1;
            if (c >= 0xF0) charLen = 4;
            else if (c >= 0xE0) charLen = 3;
            else if (c >= 0xC0) charLen = 2;

            std::string utf8Char = para.substr(i, charLen);

            if (utf8Char == " ") {
                if (!currentWord.empty()) {
                    words.push_back(currentWord);
                    currentWord.clear();
                }
            } else {
                currentWord += utf8Char;
            }
            i += charLen;
        }
        if (!currentWord.empty()) {
            words.push_back(currentWord);
        }

        if (words.empty()) continue;

        std::vector<std::vector<std::string>> lines;
        std::vector<std::string>              currentLine;
        float                                 currentLineWidth = 0.0f;

        for (const auto& w : words) {
            float wordWidth = font->CalcTextSizeA(fontSize, FLT_MAX, 0.0f, w.c_str()).x;
            float spacing   = currentLine.empty() ? 0.0f : spaceWidth;

            if (currentLineWidth + spacing + wordWidth > width) {
                lines.push_back(currentLine);
                currentLine.clear();
                currentLine.push_back(w);
                currentLineWidth = wordWidth;
            } else {
                currentLine.push_back(w);
                currentLineWidth += spacing + wordWidth;
            }
        }
        if (!currentLine.empty()) {
            lines.push_back(currentLine);
        }

        for (size_t lineIdx = 0; lineIdx < lines.size(); ++lineIdx) {
            const auto& lineWords  = lines[lineIdx];
            bool        isLastLine = (lineIdx == lines.size() - 1);

            float totalWordsWidth = 0.0f;
            for (const auto& w : lineWords) {
                totalWordsWidth += font->CalcTextSizeA(fontSize, FLT_MAX, 0.0f, w.c_str()).x;
            }

            float currentX = pos.x;
            float currentY = pos.y + yOffset;

            if (isLastLine || lineWords.size() <= 1) {
                for (const auto& w : lineWords) {
                    drawList->AddText(font, fontSize, ImVec2(currentX, currentY), color, w.c_str());
                    currentX += font->CalcTextSizeA(fontSize, FLT_MAX, 0.0f, w.c_str()).x + spaceWidth;
                }
            } else {
                float remainingSpace = width - totalWordsWidth;
                float gapWidth       = remainingSpace / static_cast<float>(lineWords.size() - 1);

                for (size_t wordIdx = 0; wordIdx < lineWords.size(); ++wordIdx) {
                    const auto& w = lineWords[wordIdx];
                    drawList->AddText(font, fontSize, ImVec2(currentX, currentY), color, w.c_str());
                    currentX += font->CalcTextSizeA(fontSize, FLT_MAX, 0.0f, w.c_str()).x + gapWidth;
                }
            }

            yOffset += fontSize * 1.42f;
        }
        yOffset += fontSize * 0.35f;
    }

    return yOffset;
}

} // namespace app::ui
