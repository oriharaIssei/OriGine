#include "PrimitiveType.h"

using namespace OriGine;

std::string std::to_string(PrimitiveType _type) {
    int32_t index = static_cast<int32_t>(_type);
    if (index < 0 || index >= static_cast<int32_t>(PrimitiveType::Count)) {
        return "Unknown";
    }
    return kPrimitiveTypes[index];
}
