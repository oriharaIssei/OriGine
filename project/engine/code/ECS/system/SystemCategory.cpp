#include "SystemCategory.h"

std::string std::to_string(const OriGine::SystemCategory& _category) {
    return OriGine::SystemCategoryString[static_cast<int>(_category)];
}
