#pragma once

/// stl
#include <concepts>
#include <string>

/// ECS
// entity
#include "entity/EntityHandle.h"
// component
#include "component/ComponentHandle.h"

/// externals
#include <nlohmann/json.hpp>

/// utility
#include <util/nameof.h>

namespace OriGine {
/// 前方宣言
class Entity;
class Scene;

///< summary>
/// 1コンポーネントを表すクラス(基底クラス)。
/// すべての具象コンポーネントはこれを継承することで、ComponentArray/ComponentRepository が
/// 具体的な型を知らなくても IComponent* として一様に初期化・編集・破棄できるようになる(型消去の要)。
/// 実際の格納は ComponentArray<T> がテンプレートで型付きの配列を保持し、
/// IComponentArray インターフェース越しに ComponentRepository から型を意識せず扱われる。
///</summary>
class IComponent {
public:
    IComponent();
    virtual ~IComponent();

    /// <summary>
    /// 初期化処理(派生クラスで実装)
    /// </summary>
    /// <param name="_scene">所属するScene</param>
    /// <param name="_owner">このComponentを所有するEntity</param>
    virtual void Initialize(Scene* _scene, const EntityHandle& _owner) = 0;
    /// <summary>
    /// 終了処理(派生クラスで実装)
    /// </summary>
    virtual void Finalize()                                     = 0;

    /// <summary>
    /// エディタ上での編集UIを描画する処理(派生クラスで実装)
    /// </summary>
    /// <param name="_scene">所属するScene</param>
    /// <param name="_owner">このComponentを所有するEntity</param>
    /// <param name="_parentLabel">親階層のImGuiラベル(ID衝突回避用)</param>
    virtual void Edit(Scene* _scene, const EntityHandle& _owner, const std::string& _parentLabel) = 0;
    /// <summary>
    /// デバッグ表示処理。デフォルトではEditと同じ内容を表示する
    /// </summary>
    virtual void Debug(Scene* _scene, const EntityHandle& _owner, const std::string& _parentLabel) {
        Edit(_scene, _owner, _parentLabel);
    }

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
