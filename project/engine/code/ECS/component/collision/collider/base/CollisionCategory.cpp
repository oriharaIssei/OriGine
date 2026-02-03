#include "CollisionCategory.h"

using namespace OriGine;

void CollisionMask::Set(const std::string& _categoryName) {
    CollisionCategoryManager* manager = CollisionCategoryManager::GetInstance();
    const CollisionCategory& category = manager->GetCategory(_categoryName);
    if (category.GetBits() != 0) {
        bits_ |= category.GetBits();
    }
}
