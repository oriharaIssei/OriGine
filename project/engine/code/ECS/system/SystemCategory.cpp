#include "SystemCategory.h"

using namespace OriGine;

std::string ToString(const SystemCategory& _category) {
    return kSystemCategoryString[static_cast<int>(_category)];
}
