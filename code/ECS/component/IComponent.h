#pragma once

/// stl
#include <concepts>
#include <string>

/// ECS
// entity
#include "entity/EntityHandle.h"
// component
#include "component/ComponentHandle.h"
#include "component/ComponentReflection.h"

/// externals
#include <nlohmann/json.hpp>

/// utility
#include <util/nameof.h>

/// DLL境界
#include "OriGineApi.h"

namespace OriGine {
/// 前方宣言
class Entity;
class Scene;

///< summary>
/// 1コンポーネントを表すクラス(基底クラス)。
/// すべての具象コンポーネントはこれを継承する。ComponentHandle の保持だけがここでの役割で、
/// Initialize/Finalize/Edit は非仮想メンバとして派生クラス側にだけ実体を持つ(Phase 3 3B。
/// 旧: 純粋仮想関数でIComponent*経由の型消去を担っていたが、非仮想化に伴いその役割は失った)。
/// 実際の格納は ComponentArray<T> がテンプレートで型付きの配列を保持し、
/// ComponentType& のまま具象型を知った状態で Initialize/Finalize を呼ぶ。
///</summary>
class ORIGINE_API IComponent {
public:
    IComponent();
    ~IComponent();

    // Initialize / Finalize / Edit は派生クラス側にだけ実体を持つ非仮想メンバ関数になった
    // (Phase 3 3B。旧: 純粋仮想。基底に実装がないため、宣言ごと派生へ移した。
    // 呼び出し側は具象型 or ComponentType& のテンプレート越しにしか呼ばないため、
    // 基底からこの3つの宣言を消してもコンパイルは通る)。
    //
    // Debug も同様の理由でここから削除した。既定実装はEditを呼ぶだけだったが、
    // Editが基底から無くなったため中身が書けなくなった。IComponent::Debug は
    // 調査の結果コードベース全体で呼び出し箇所が0件だった(A-2)ため、
    // 復活させる必要が生じたら呼び出し側の設計と合わせて再検討する。

private:
    ComponentHandle handle_{}; // このComponent自身を一意に識別するHandle

public:
    /// <summary>
    /// このコンポーネント自身を識別するハンドルを取得する
    /// </summary>
    /// <returns>コンポーネントハンドル</returns>
    ComponentHandle GetHandle() const { return handle_; }
    /// <summary>
    /// 禁止: コンポーネントハンドルの設定。
    /// 格納先の配列インデックスと不整合になるため、ComponentArray が生成・格納時に設定する目的でのみ使用し、
    /// 利用者コード側から呼び出してはならない。
    /// </summary>
    /// <param name="_handle">設定するコンポーネントハンドル</param>
    void SetHandle(ComponentHandle _handle) { handle_ = _handle; }
};

/// <summary>
/// コンポーネントを継承しているかどうかを判定する
/// </summary>
template <typename componentType>
concept IsComponent = ::std::derived_from<componentType, IComponent>;

} // namespace OriGine
