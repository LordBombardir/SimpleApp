#pragma once

#include <string>
#include <unordered_map>
#include <vector>

namespace app {

class TextManager {
public:
    static TextManager& Instance();

    void                     Initialize();
    void                     SetLanguage(const std::string& langCode);
    std::string              GetCurrentLanguage() const;
    std::vector<std::string> GetAvailableLanguages() const;

    std::string Get(const std::string& key, const std::string& defaultValue = "") const;
    std::string GetForLanguage(const std::string& langCode, const std::string& key, const std::string& defaultValue = "") const;

private:
    TextManager() = default;

    std::string DetectSystemLanguage() const;
    void        LoadEmbeddedLanguages();
    void        ParseJson(const std::string& jsonContent, std::unordered_map<std::string, std::string>& outMap);

    std::string m_currentLang = "en_US";

    std::unordered_map<std::string, std::unordered_map<std::string, std::string>> m_languages;
};

} // namespace app
