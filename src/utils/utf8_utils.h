#pragma once

#include <string>
#include <string_view>

namespace utils {

/**
 * @brief Utility functions for working with UTF-8 encoded strings.
 */
class Utf8Utils {
public:
    Utf8Utils() = delete;

    /**
     * @brief Calculates the number of UTF-8 characters (code points) in a string view.
     */
    [[nodiscard]] static int GetCharCount(std::string_view str) noexcept;

    /**
     * @brief Returns a view of the prefix containing up to charCount UTF-8 characters.
     */
    [[nodiscard]] static std::string_view GetPrefixView(std::string_view str, int charCount) noexcept;

    /**
     * @brief Returns a std::string containing up to charCount UTF-8 characters.
     */
    [[nodiscard]] static std::string GetPrefix(std::string_view str, int charCount);
};

} // namespace utils
