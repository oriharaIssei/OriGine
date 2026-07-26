#include "IComponent.h"

/// engine
// ECS
#include "entity/Entity.h"

namespace OriGine {

/// <summary>
/// コンストラクタ。
/// IComponent は全コンポーネントの基底インターフェースであり、それ自体は状態を持たない。
/// 実際の初期化は派生クラスの Initialize() が ComponentArray から呼ばれた時点で行われる
/// </summary>
IComponent::IComponent() {}

/// <summary>
/// デストラクタ。
/// 解放処理は派生クラスの Finalize()（ComponentArray::RemoveComponent が呼び出す）側の責務で、
/// ここでは何もしない
/// </summary>
IComponent::~IComponent() {}

} // namespace OriGine
