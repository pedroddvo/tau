#include <string>
#include <optional>

namespace resource {
std::string canonicalize_path(const std::string& path);

std::optional<std::string> read_resource_to_string(const std::string& path);
}