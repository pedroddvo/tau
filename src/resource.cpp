#include "resource.h"
#include <fstream>
#include <spdlog/spdlog.h>

std::string resource::qualify_path(const std::string& path) {
    return "resource/" + path;
}

std::optional<std::string> resource::read_bytes(const std::string& path) {
    std::ifstream file(qualify_path(path), std::ios::binary | std::ios::ate);
    if (!file) {
        spdlog::error("failed to load resource {}", path);
        return {};
    }

    std::ios::pos_type contents_len = file.tellg();
    file.seekg(0);

    std::string contents(contents_len, '\0');
    file.read(contents.data(), contents_len);

    return contents;
}
