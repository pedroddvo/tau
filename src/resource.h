#pragma once
#include <optional>
#include <string>

namespace resource {
std::string qualify_path(const std::string& path);
std::optional<std::string> read_bytes(const std::string& path);
}
