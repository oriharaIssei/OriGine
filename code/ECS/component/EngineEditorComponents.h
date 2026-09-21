#pragma once

/// DLL境界
#include "OriGineApi.h"

namespace OriGine {

/// <summary>
/// エディタで付け外しするためだけに ComponentRegistry へ登録する、engine 標準コンポーネント群
/// (Transform2d, OutlineComponent)。ベンチマーク基盤が使う9型(CollisionCheckSystem が
/// 依存する型。FrameWork.cpp の RegisterUsingComponents() 側で登録し続ける)とは別枠にしてある
/// 理由:
///
/// RegisterComponent&lt;T&gt;() はテンプレートなので、呼んだ側の翻訳単位で
/// ComponentArray&lt;T&gt; を実体化する。ComponentArray&lt;T&gt; は IComponentArray の
/// 純粋仮想関数をすべてオーバーライドするため、型が1つでも構築されると
/// (Save/Load/AddComponent 等の呼び出しが実際に無くても)vtable のために
/// T::Initialize/Finalize/コンストラクタ/デストラクタ、および(手書き to_json/from_json の
/// 型では)to_json/from_json の実体化が要る。
///
/// OutlineComponent はこれらが OriGine.dll 内の .cpp に非inline・非exportで定義されているため、
/// EXE 側(project/application/code/FrameWork.cpp)の翻訳単位で RegisterComponent&lt;T&gt;() を
/// 呼ぶと、DLL がエクスポートしていないシンボルへの参照が生まれ LNK2019 になる
/// (実測済み。旧 FrameWork.cpp のコメント参照)。この関数を OriGine.dll 側の .cpp で定義し、
/// そこでテンプレートを実体化させることで、シンボル解決がモジュール内で閉じる
/// (境界を越えない)ようにする。
///
/// 型ごとに ORIGINE_API を付けて回る案(Q1 で不採用)ではなく、「DLL の中で登録関数を1つ
/// 用意する」方式を採っているのはこのため: エクスポートが要るのはこの関数1つだけで済み、
/// 対象コンポーネントのクラス自体には手を入れない。
///
/// Transform2d は Initialize/Finalize がヘッダ inline な No-op で、かつ表経由シリアライズ
/// (kUsesDescriptorSerialization)に切り替え済みのため、本来はこの問題を踏まない
/// (旧 FrameWork.cpp では EXE 側で直接登録していた)。ここに合流させたのは、
/// 「エディタ用に付け外しできるようにするだけの登録」という役割が同じだからで、
/// 必然ではなく整理のための移動。
///
/// SmoothingEffectParam はここでも登録しない。DLL の内側に置いても解消しない、別の既知の
/// バグ(to_json/from_json が namespace OriGine の外に定義されている)があるため、
/// 実装の .cpp 側で報告のみに留めている(EngineEditorComponents.cpp 参照)。
/// </summary>
ORIGINE_API void RegisterEngineEditorComponents();

} // namespace OriGine
