#include "utils/utf8_utils.h"

namespace utils {

int Utf8Utils::GetCharCount(std::string_view str) noexcept {
    int count = 0;

    size_t i   = 0;
    size_t len = str.length();
    while (i < len) {
        unsigned char c = static_cast<unsigned char>(str[i]);

        int charLen = 1;
        if (c >= 0xF0) charLen = 4;
        else if (c >= 0xE0) charLen = 3;
        else if (c >= 0xC0) charLen = 2;

        i += charLen;
        count++;
    }

    return count;
}

std::string_view Utf8Utils::GetPrefixView(std::string_view str, int charCount) noexcept {
    if (charCount <= 0) return {};

    int count = 0;

    size_t i   = 0;
    size_t len = str.length();
    while (i < len && count < charCount) {
        unsigned char c = static_cast<unsigned char>(str[i]);

        int charLen = 1;
        if (c >= 0xF0) charLen = 4;
        else if (c >= 0xE0) charLen = 3;
        else if (c >= 0xC0) charLen = 2;

        i += charLen;
        count++;
    }

    return str.substr(0, i);
}

std::string Utf8Utils::GetPrefix(std::string_view str, int charCount) {
    return std::string(GetPrefixView(str, charCount));
}

} // namespace utils
