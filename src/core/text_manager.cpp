#include "core/text_manager.h"
#include "resources/embedded_resources.h"
#include <cstdlib>
#include <nlohmann/json.hpp>

#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#endif

#include <algorithm>
#include <stdexcept>

namespace app {

TextManager& TextManager::Instance() {
    static TextManager instance;
    return instance;
}

void TextManager::Initialize() {
    LoadEmbeddedLanguages();
    std::string detected = DetectSystemLanguage();
    SetLanguage(detected);
}

void TextManager::SetLanguage(const std::string& langCode) {
    if (langCode == "Auto" || langCode.empty()) {
        m_currentLang = DetectSystemLanguage();
    } else {
        m_currentLang = langCode;
    }
}

std::string TextManager::GetCurrentLanguage() const { return m_currentLang; }

std::vector<std::string> TextManager::GetAvailableLanguages() const {
    if (m_languages.empty()) {
        throw std::runtime_error(
            "TextManager is not initialized! Call TextManager::Instance().Initialize() before accessing languages."
        );
    }

    std::vector<std::string> result;
    result.reserve(m_languages.size());
    for (const auto& [langCode, _] : m_languages) {
        result.push_back(langCode);
    }

    std::sort(result.begin(), result.end());
    return result;
}

std::string TextManager::GetForLanguage(
    const std::string& langCode,
    const std::string& key,
    const std::string& defaultValue
) const {
    if (m_languages.empty()) {
        throw std::runtime_error(
            "TextManager is not initialized! Call TextManager::Instance().Initialize() before accessing texts."
        );
    }

    auto langIt = m_languages.find(langCode);
    if (langIt != m_languages.end()) {
        auto keyIt = langIt->second.find(key);
        if (keyIt != langIt->second.end()) {
            return keyIt->second;
        }
    }

    return defaultValue;
}

std::string TextManager::DetectSystemLanguage() const {
#ifdef _WIN32
    static const std::unordered_map<std::string, std::string> s_langMap = {
        {"ru", "ru_RU"},
        {"en", "en_US"}
    };

    auto MatchLang = [&](const std::wstring& wtag) -> std::string {
        char tagBuf[32] = {0};
        WideCharToMultiByte(CP_UTF8, 0, wtag.c_str(), -1, tagBuf, sizeof(tagBuf), nullptr, nullptr);
        std::string tag(tagBuf);
        for (const auto& [prefix, targetLang] : s_langMap) {
            if (tag.rfind(prefix, 0) == 0) {
                return targetLang;
            }
        }
        return "";
    };

    ULONG count      = 0;
    ULONG bufferSize = 0;
    if (GetUserPreferredUILanguages(MUI_LANGUAGE_NAME, &count, nullptr, &bufferSize) && bufferSize > 0) {
        std::vector<wchar_t> buffer(bufferSize);
        if (GetUserPreferredUILanguages(MUI_LANGUAGE_NAME, &count, buffer.data(), &bufferSize)) {
            std::string match = MatchLang(buffer.data());
            if (!match.empty()) return match;
        }
    }

    wchar_t localeName[LOCALE_NAME_MAX_LENGTH] = {0};
    if (GetUserDefaultLocaleName(localeName, LOCALE_NAME_MAX_LENGTH) > 0) {
        std::string match = MatchLang(localeName);
        if (!match.empty()) return match;
    }

    return m_currentLang;
#else
    static const std::unordered_map<std::string, std::string> s_langMap = {
        {"ru", "ru_RU"},
        {"en", "en_US"}
    };

    const char* langEnv = std::getenv("LANG");
    if (!langEnv) langEnv = std::getenv("LC_ALL");
    if (langEnv) {
        std::string str(langEnv);
        for (const auto& pair : s_langMap) {
            if (str.find(pair.first) != std::string::npos) {
                return pair.second;
            }
        }
    }

    return m_currentLang;
#endif
}

void TextManager::LoadEmbeddedLanguages() {
    m_languages.clear();
    auto embeddedFiles = GetEmbeddedTextFiles();
    for (const auto& file : embeddedFiles) {
        if (file.data && file.size > 0) {
            std::string content(reinterpret_cast<const char*>(file.data), file.size);
            ParseJson(content, m_languages[file.name]);
        }
    }
}

std::string TextManager::Get(const std::string& key, const std::string& defaultValue) const {
    if (m_languages.empty()) {
        throw std::runtime_error(
            "TextManager is not initialized! Call TextManager::Instance().Initialize() before accessing texts."
        );
    }

    if (auto langIt = m_languages.find(m_currentLang); langIt != m_languages.end()) {
        if (auto keyIt = langIt->second.find(key); keyIt != langIt->second.end()) {
            return keyIt->second;
        }
    }

    if (m_currentLang != "en_US") {
        if (auto fallbackIt = m_languages.find("en_US"); fallbackIt != m_languages.end()) {
            if (auto keyIt = fallbackIt->second.find(key); keyIt != fallbackIt->second.end()) {
                return keyIt->second;
            }
        }
    }

    return defaultValue;
}

void TextManager::ParseJson(const std::string& jsonContent, std::unordered_map<std::string, std::string>& outMap) {
    outMap.clear();

    auto parsedJson = nlohmann::json::parse(jsonContent, nullptr, false, true);
    if (!parsedJson.is_discarded() && parsedJson.is_object()) {
        for (const auto& [key, value] : parsedJson.items()) {
            if (value.is_string()) {
                outMap[key] = value.get<std::string>();
            }
        }
    }
}

} // namespace app
