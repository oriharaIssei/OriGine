#pragma once

/// stl
#include <concepts>
#include <string>

/// externals
#include <nlohmann/json.hpp>

/// utility
#include <util/nameof.h>

namespace OriGine {

class Entity;
class Scene;
///< summary>
/// 1コンポーネントを表すクラス(基底クラス)
///</summary>
class IComponent {
public:
    IComponent();
    virtual ~IComponent();

    virtual void Initialize(Entity* _entity) = 0;

    virtual void Edit(Scene* _scene, Entity* _entity, const std::string& _parentLabel) = 0;
    virtual void Debug(Scene* _scene, Entity* _entity, [[maybe_unused]] const std::string& _parentLabel) { Edit(_scene, _entity, _parentLabel); }

    virtual void Finalize() = 0;
};

/// <summary>
/// コンポーネントを継承しているかどうかを判定する
/// </summary>
template <typename componentType>
concept IsComponent = std::derived_from<componentType, IComponent>;

} // namespace OriGine
