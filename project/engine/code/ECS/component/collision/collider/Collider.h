#pragma once

/// parent
#include "component/IComponent.h"
/// stl
#include <concepts>
#include <map>

/// engine
/// ECS

// component
#include "component/transform/Transform.h"

/// math
#include "bounds/base/IBounds.h"

// external
#ifdef _DEBUG
#include "imgui/imgui.h"
#include "myGui/MyGui.h"
#endif // _DEBUG

enum class CollisionState {
    None, // 衝突していない
    Stay, // 衝突中
    Enter, // 衝突開始時
    Exit // 衝突終了時
};

class ICollider
    : public OriGine::IComponent {
public:
    ICollider() {}
    virtual ~ICollider() {}

    virtual void Initialize(Entity* _hostEntity);
    virtual void Finalize() = 0;

    virtual void Edit(Scene* _scene, Entity* _entity, const std::string& _parentLabel) = 0;

    virtual void CalculateWorldShape() = 0;

    virtual void StartCollision();
    virtual void EndCollision();

protected:
    bool isActive_ = true;
    Transform transform_;
    std::map<int32_t, CollisionState> collisionStateMap_;
    std::map<int32_t, CollisionState> preCollisionStateMap_;

public: // accessor
    bool IsActive() const { return isActive_; }
    void SetActive(bool _isActive) { isActive_ = _isActive; }

    const Transform& GetTransform() const { return transform_; }
    void SetParent(Transform* _trans) { transform_.parent = _trans; }

    // 衝突状態の操作
    void SetCollisionState(int32_t _otherId) {
        if (this->preCollisionStateMap_[_otherId] == CollisionState::None)
            this->collisionStateMap_[_otherId] = CollisionState::Enter;
        else
            this->collisionStateMap_[_otherId] = CollisionState::Stay;
    }
    const std::map<int32_t, CollisionState>& GetCollisionStateMap() { return collisionStateMap_; }
};

template <math::bounds::IsBounds BoundsClass>
class Collider
    : public ICollider {
public:
    Collider() {}
    void Initialize(Entity* _hostEntity) override {
        ICollider::Initialize(_hostEntity);
    }
    void Finalize() override {
        this->collisionStateMap_.clear();
        this->preCollisionStateMap_.clear();
    }

    virtual void Edit(Scene* _scene, Entity* _entity, const std::string& _parentLabel) = 0;

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
