#include "EngineEditorComponents.h"

#include "component/ComponentRegistry.h"

#include "component/effect/post/OutlineComponent.h"
#include "component/transform/Transform2d.h"

namespace OriGine {

void RegisterEngineEditorComponents() {
    ComponentRegistry* componentRegistry = ComponentRegistry::GetInstance();

    // RegisterComponent<T>() の実体化はこの翻訳単位(OriGine.dll)で起きる。
    // 型ごとの理由は EngineEditorComponents.h のクラスコメント参照。
    componentRegistry->RegisterComponent<Transform2d>();
    componentRegistry->RegisterComponent<OutlineComponent>();

    // SmoothingEffectParam はここでも登録しない。DLL側に置いてもLNK2019が別の理由で残る
    // (実測済み): SmoothingEffectParam.cpp の to_json/from_json は `namespace OriGine` の
    // 外(グローバル名前空間)に定義されており、ヘッダのfriend宣言が指す OriGine::to_json/
    // from_json とは別物になっている(SmoothingEffectParam.h の既存コメント、
    // SerializeGoldenTests.cpp 末尾のコメントで既知)。RegisterComponent<SmoothingEffectParam>()
    // は ComponentArray<SmoothingEffectParam> の SaveComponent 経由でこの ADL to_json/from_json
    // を要求するため、DLLの内側であっても実体が無く LNK2019 になる
    // (OutlineComponentのような「非exportだが実体はある」問題ではなく、そもそも
    // 期待される名前空間に定義が無い、独立した既知のバグ)。
    // 修正(2箇所を namespace OriGine {} で正しく囲むだけ)は1行規模だが、
    // docs/plans/phase-04.md の「Phase 4に入れないもの」表と SmoothingEffectParam.h の
    // コメントが、この既知の問題を意図的にPhase 3/4のスコープ外としているため、
    // このタスクでは修正せず報告のみに留める。
}

} // namespace OriGine
