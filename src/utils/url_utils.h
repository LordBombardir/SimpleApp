#pragma once

#include <string>

namespace app::utils {

class UrlUtils {
public:
    UrlUtils() = delete;

    /**
     * @brief Opens the specified URL in the OS default web browser.
     */
    static void OpenURL(const std::string& url);

    /**
     * @brief Extracts prefix text and URL substring from a given text line.
     * @return true if a URL (http:// or https://) was found.
     */
    static bool ExtractUrl(const std::string& line, std::string& prefix, std::string& url);
};

} // namespace app::utils
