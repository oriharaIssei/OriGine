#include "EngineEditorComponents.h"

#include "component/ComponentRegistry.h"

#include "component/effect/post/OutlineComponent.h"
#include "component/effect/post/SmoothingEffectParam.h"
#include "component/transform/Transform2d.h"

namespace OriGine {

void RegisterEngineEditorComponents() {
    ComponentRegistry* componentRegistry = ComponentRegistry::GetInstance();

    // RegisterComponent<T>() の実体化はこの翻訳単位(OriGine.dll)で起きる。
    // 型ごとの理由は EngineEditorComponents.h のクラスコメント参照。
    componentRegistry->RegisterComponent<Transform2d>();
    componentRegistry->RegisterComponent<OutlineComponent>();
    componentRegistry->RegisterComponent<SmoothingEffectParam>();
}

} // namespace OriGine
