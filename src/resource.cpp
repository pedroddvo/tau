#include "resource.hpp"
#include <fstream>
#include <sstream>

namespace resource {

std::string canonicalize_path(const std::string& path) {
    return "resource/" + path;
}

std::optional<std::string> read_resource_to_string(const std::string& path) {
    auto canonical_path = canonicalize_path(path);

    std::ifstream file(canonical_path);
    if (file.fail()) return {};

    std::stringstream buf;
    buf << file.rdbuf();

    return buf.str();
}

} // namespace resource