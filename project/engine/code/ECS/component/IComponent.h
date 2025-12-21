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
/// 1コンポーネントを表すクラス(基底クラス)
///</summary>
class IComponent {
public:
    virtual ~IComponent() = default;

    virtual void Initialize(OriGine::Scene* _scene, OriGine::EntityHandle _owner) = 0;
    virtual void Finalize()                                     = 0;

    virtual void Edit(Scene* _scene, EntityHandle _owner, const std::string& _parentLabel) = 0;
    virtual void Debug(Scene* _scene, EntityHandle _owner, const std::string& _parentLabel) {
        Edit(_scene, _owner, _parentLabel);
    }

private:
    ComponentHandle handle_{};

public:
    ComponentHandle GetHandle() const { return handle_; }
    /// <summary>
    /// 禁止: コンポーネントハンドルの設定
    /// </summary>
    /// <param name="_handle"></param>
    void SetHandle(ComponentHandle _handle) { handle_ = _handle; }
};

/// <summary>
/// コンポーネントを継承しているかどうかを判定する
/// </summary>
template <typename componentType>
concept IsComponent = ::std::derived_from<componentType, IComponent>;

} // namespace OriGine
