#include "SystemCategory.h"

std::string std::to_string(const SystemCategory& _category) {
    return SystemCategoryString[static_cast<int>(_category)];
}
