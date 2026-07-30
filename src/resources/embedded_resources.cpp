#include "resources/embedded_resources.h"

#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>

#define IDR_FONT_JETBRAINS_MONO 101
#else
extern "C" {
extern const unsigned char _binary_JetBrainsMono_Regular_ttf_start[];
extern const unsigned char _binary_JetBrainsMono_Regular_ttf_end[];
}
#endif

namespace app {

#ifdef _WIN32
static bool GetWin32Resource(int resId, const unsigned char*& outData, size_t& outSize) {
    HMODULE hModule = GetModuleHandle(nullptr);
    HRSRC   hRes    = FindResourceW(hModule, MAKEINTRESOURCEW(resId),
                                    MAKEINTRESOURCEW(10)); // RT_RCDATA
    if (!hRes) return false;

    HGLOBAL hMem = LoadResource(hModule, hRes);
    if (!hMem) return false;

    outData = static_cast<const unsigned char*>(LockResource(hMem));
    outSize = static_cast<size_t>(SizeofResource(hModule, hRes));
    return (outData != nullptr && outSize > 0);
}

bool GetEmbeddedFontJetBrainsMono(const unsigned char*& outData, size_t& outSize) {
    return GetWin32Resource(IDR_FONT_JETBRAINS_MONO, outData, outSize);
}
#else
bool GetEmbeddedFontJetBrainsMono(const unsigned char*& outData, size_t& outSize) {
    outData = _binary_JetBrainsMono_Regular_ttf_start;
    outSize = static_cast<size_t>(_binary_JetBrainsMono_Regular_ttf_end - _binary_JetBrainsMono_Regular_ttf_start);
    return (outData != nullptr && outSize > 0);
}
#endif

#include "embedded_texts_impl.inc"

} // namespace app
