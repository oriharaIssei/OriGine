#include "NormalizeString.h"

#include <algorithm>

std::string normalizeString(const std::string& path) {
    std::string normalized = path;
    std::replace(normalized.begin(), normalized.end(), '\\', '/');
    return normalized;
}
