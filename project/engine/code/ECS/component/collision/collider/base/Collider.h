#pragma once

/// parent
#include "component/IComponent.h"
/// stl
#include <concepts>
#include <unordered_map>

/// engine
/// ECS
// component
#include "component/collision/collider/base/CollisionCategory.h"
#include "component/collision/collider/base/CollisionMask.h"
#include "component/transform/Transform.h"

/// math
#include "bounds/base/IBounds.h"

// external
#ifdef _DEBUG
#include "imgui/imgui.h"
#include "myGui/MyGui.h"
#endif // _DEBUG

namespace OriGine {

/// <summary>
/// 衝突状態
/// </summary>
enum class CollisionState { None, // 衝突していない
    Stay, // 衝突中
    Enter, // 衝突開始時
    Exit // 衝突終了時
};

/// <summary>
/// コライダーのインターフェース
/// </summary>
class ICollider
    : public IComponent {
    friend void to_json(nlohmann::json& _j, const ICollider& _c);
    friend void from_json(const nlohmann::json& _j, ICollider& _c);

public:
    ICollider() {}
    virtual ~ICollider() {}

    virtual void Initialize(Scene* _scene, EntityHandle _entity) = 0;
    virtual void Finalize()                                      = 0;

    virtual void Edit(Scene* _scene, EntityHandle _handle, const std::string& _parentLabel);

    virtual void CalculateWorldShape() = 0;

    virtual void StartCollision();
    virtual void EndCollision();

    /// <summary>
    /// 衝突可能か判定
    /// </summary>
    /// <param name="_other">相手のコライダー</param>
    bool CanCollideWith(const ICollider& _other) const {
        return collisionMask_.CanCollideWith(_other.collisionCategory_.GetBits());
    }

protected:
    bool isActive_ = true;

    CollisionCategory collisionCategory_ = CollisionCategory();
    CollisionMask collisionMask_         = CollisionMask();

    Transform transform_;
    std::unordered_map<EntityHandle, CollisionState> collisionStateMap_;
    std::unordered_map<EntityHandle, CollisionState> preCollisionStateMap_;

public: // accessor
    bool IsActive() const { return isActive_; }
    void SetActive(bool _isActive) { isActive_ = _isActive; }

    const Transform& GetTransform() const { return transform_; }
    void SetParent(Transform* _parent) { transform_.parent = _parent; }

    const CollisionCategory& GetCollisionCategory() const { return collisionCategory_; }
    void SetCollisionCategory(const CollisionCategory& _category) { collisionCategory_ = _category; }
    const CollisionMask& GetCollisionMask() const { return collisionMask_; }
    void SetCollisionMask(const CollisionMask& _mask) { collisionMask_ = _mask; }

    // 衝突状態の操作
    void SetCollisionState(EntityHandle _otherHandle) {
        if (this->preCollisionStateMap_[_otherHandle] == CollisionState::None) {
            this->collisionStateMap_[_otherHandle] = CollisionState::Enter;
        } else {
            this->collisionStateMap_[_otherHandle] = CollisionState::Stay;
        }
    }
    const std::unordered_map<EntityHandle, CollisionState>& GetCollisionStateMap() const { return collisionStateMap_; }
};

template <Bounds::IsBounds BoundsClass>
class Collider
    : public ICollider {
public:
    Collider() {}
    void Initialize(Scene* /*_scene*/, EntityHandle /*_entity*/) override {}
    void Finalize() override {
        this->collisionStateMap_.clear();
        this->preCollisionStateMap_.clear();
    }

    virtual void Edit(Scene* _scene, EntityHandle _handle, const std::string& _parentLabel) = 0;

    virtual void CalculateWorldShape() = 0;

protected:
    BoundsClass shape_;
    BoundsClass worldShape_;

public:
    const BoundsClass& GetLocalShape() { return shape_; }
    const BoundsClass& GetWorldShape() { return worldShape_; }

    BoundsClass* GetLocalShapePtr() { return &shape_; }
    BoundsClass* GetWorldShapePtr() { return &worldShape_; }
};

} // namespace OriGine
