#include "CollisionMask.h"
#include "CollisionCategoryManager.h"

namespace OriGine {

/// <summary>
/// カテゴリ名を指定して、そのカテゴリと衝突可能になるようマスクにビットを追加する
/// </summary>
/// <param name="_categoryName">衝突を許可したいカテゴリ名（未登録なら新規登録される）</param>
void CollisionMask::Set(const std::string& _categoryName) {
    auto* manager        = CollisionCategoryManager::GetInstance();
    const auto& category = manager->GetOrRegisterCategory(_categoryName);
    // カテゴリのビットをOR演算でマスクに立てる。
    // これにより CanCollideWith(相手のカテゴリビット) が呼ばれた際に、
    // このビットが立っている相手とは (bits_ & otherCategoryBits) != 0 となり衝突可能と判定される
    if (category.GetBits() != 0) {
        bits_ |= category.GetBits();
    }
}

} // namespace OriGine
