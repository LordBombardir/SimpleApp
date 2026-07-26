#ifndef NOMINMAX
#define NOMINMAX
#endif

#include "utils/url_utils.h"

#ifdef _WIN32
#include <windows.h>
#include <shellapi.h>
#else
#include <cstdlib>
#endif

namespace app::utils {

void UrlUtils::OpenURL(const std::string& rawUrl) {
    if (rawUrl.empty()) return;

    if (rawUrl.find("http://") != 0 && rawUrl.find("https://") != 0) {
        return;
    }

    std::string safeUrl;
    safeUrl.reserve(rawUrl.size());
    for (char c : rawUrl) {
        if (c == '"' || c == '\'' || c == '`' || c == '$' || c == ';' ||
            c == '&' || c == '|'  || c == '>' || c == '<' || c == ' ' ||
            c == '\r' || c == '\n' || c == '\t') {
            continue;
        }
        safeUrl.push_back(c);
    }

    if (safeUrl.empty()) return;

#ifdef _WIN32
    ShellExecuteA(nullptr, "open", safeUrl.c_str(), nullptr, nullptr, SW_SHOWNORMAL);
#elif defined(__APPLE__)
    std::string cmd = "open \"" + safeUrl + "\" &";
    int ret = ::system(cmd.c_str());
    (void)ret;
#else
    std::string cmd = "xdg-open \"" + safeUrl + "\" &";
    int ret = ::system(cmd.c_str());
    (void)ret;
#endif
}

bool UrlUtils::ExtractUrl(const std::string& line, std::string& prefix, std::string& url) {
    size_t httpPos = line.find("http://");
    if (httpPos == std::string::npos) {
        httpPos = line.find("https://");
    }

    if (httpPos == std::string::npos) {
        prefix = line;
        url.clear();
        return false;
    }

    prefix = line.substr(0, httpPos);
    url    = line.substr(httpPos);

    while (!url.empty() && (url.back() == '\r' || url.back() == '\n' || url.back() == ' ')) {
        url.pop_back();
    }

    return !url.empty();
}

} // namespace app::utils
