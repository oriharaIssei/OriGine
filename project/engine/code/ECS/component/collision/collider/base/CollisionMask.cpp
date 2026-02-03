#include "CollisionMask.h"
#include "CollisionCategoryManager.h"

namespace OriGine {

void CollisionMask::Set(const std::string& _categoryName) {
    auto* manager        = CollisionCategoryManager::GetInstance();
    const auto& category = manager->GetOrRegisterCategory(_categoryName);
    if (category.GetBits() != 0) {
        bits_ |= category.GetBits();
    }
}

} // namespace OriGine
