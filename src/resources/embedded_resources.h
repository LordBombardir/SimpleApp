#pragma once

#include <cstddef>
#include <string>
#include <vector>

namespace app {

struct EmbeddedTextFile {
    std::string          name;
    const unsigned char* data;
    size_t               size;
};

bool GetEmbeddedFontJetBrainsMono(const unsigned char*& outData, size_t& outSize);

std::vector<EmbeddedTextFile> GetEmbeddedTextFiles();

} // namespace app
