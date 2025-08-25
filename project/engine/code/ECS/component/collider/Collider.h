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
#include "math/shape/Shapes.h"

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
    : public IComponent {
public:
    ICollider() {}
    virtual ~ICollider() {}

    virtual void Initialize(GameEntity* _hostEntity);
    virtual void Finalize() = 0;

    virtual void Edit(Scene* _scene, GameEntity* _entity, const std::string& _parentLabel) = 0;

    virtual void CalculateWorldShape() = 0;

    virtual void StartCollision();
    virtual void EndCollision();

protected:
    bool isActive_ = true;
    Transform transform_;
    std::map<int32_t, CollisionState> collisionStateMap_;
    std::map<int32_t, CollisionState> preCollisionStateMap_;

public: // accessor
    bool isActive() const { return isActive_; }
    void setActive(bool _isActive) { isActive_ = _isActive; }

    const Transform& getTransform() const { return transform_; }
    void setParent(Transform* _trans) { transform_.parent = _trans; }

    // 衝突状態の操作
    void setCollisionState(int32_t _otherId) {
        if (this->preCollisionStateMap_[_otherId] == CollisionState::None)
            this->collisionStateMap_[_otherId] = CollisionState::Enter;
        else
            this->collisionStateMap_[_otherId] = CollisionState::Stay;
    }
    const std::map<int32_t, CollisionState>& getCollisionStateMap() { return collisionStateMap_; }
};

template <IsShape ShapeClass>
class Collider
    : public ICollider {
public:
    using ShapeType = ShapeClass;

    Collider() {}
    void Initialize(GameEntity* _hostEntity) override {
        ICollider::Initialize(_hostEntity);
    }
    void Finalize() override {
        this->collisionStateMap_.clear();
        this->preCollisionStateMap_.clear();
    }

    virtual void Edit(Scene* _scene, GameEntity* _entity, const std::string& _parentLabel) = 0;

    virtual void CalculateWorldShape() = 0;

    // virtual void StartCollision();
    // virtual void EndCollision();

protected:
    ShapeClass shape_;
    ShapeClass worldShape_;

public:
    const ShapeClass& getLocalShape() { return shape_; }
    const ShapeClass& getWorldShape() { return worldShape_; }

    ShapeClass* getLocalShapePtr() { return &shape_; }
    ShapeClass* getWorldShapePtr() { return &worldShape_; }
};

class AABBCollider
    : public Collider<AABB> {
    friend void to_json(nlohmann::json& _json, const AABBCollider& _primitiveNodeAnimation);
    friend void from_json(const nlohmann::json& _json, AABBCollider& _primitiveNodeAnimation);

public:
    using ShapeType = AABB;

    AABBCollider()
        : Collider<AABB>() {}
    ~AABBCollider() {}

    void Edit( Scene* _scene, GameEntity* _entity, const std::string& _parentLabel) override;

    void CalculateWorldShape() override;

public: // accessor
    const Vec3f& getLocalMin() const { return shape_.min_; }
    void setLocalMin(const Vec3f& _min) { shape_.min_ = _min; }

    const Vec3f& getLocalMax() const { return shape_.max_; }
    void setLocalMax(const Vec3f& _max) { shape_.max_ = _max; }

    const Vec3f& getWorldMin() const { return worldShape_.min_; }
    void setWorldMin(const Vec3f& _min) { worldShape_.min_ = _min; }
    const Vec3f& getWorldMax() const { return worldShape_.max_; }
    void setWorldMax(const Vec3f& _max) { worldShape_.max_ = _max; }

    const Vec3f& getWorldCenter() const { return (worldShape_.min_ + worldShape_.max_) * 0.5f; }
};

class SphereCollider
    : public Collider<Sphere> {
    friend void to_json(nlohmann::json& _json, const SphereCollider& _primitiveNodeAnimation);
    friend void from_json(const nlohmann::json& _json, SphereCollider& _primitiveNodeAnimation);

public:
    using ShapeType = Sphere;

    SphereCollider() : Collider<Sphere>() {}
    ~SphereCollider() {}

    void Edit(Scene* _scene, GameEntity* _entity, const std::string& _parentLabel) override;

    void CalculateWorldShape() override;

public: // accessor
    const Vec3f& getLocalCenter() const { return shape_.center_; }
    void setLocalCenter(const Vec3f& _center) { shape_.center_ = _center; }
    const float& getLocalRadius() const { return shape_.radius_; }
    void setLocalRadius(const float& _radius) { shape_.radius_ = _radius; }

    const Vec3f& getWorldCenter() const { return worldShape_.center_; }
    void setWorldCenter(const Vec3f& _center) { worldShape_.center_ = _center; }
    const float& getWorldRadius() const { return worldShape_.radius_; }
    void setWorldRadius(const float& _radius) { worldShape_.radius_ = _radius; }
};

class OBBCollider
    : public Collider<OBB> {
    friend void to_json(nlohmann::json& _json, const OBBCollider& _primitiveNodeAnimation);
    friend void from_json(const nlohmann::json& _json, OBBCollider& _primitiveNodeAnimation);

public:
    using ShapeType = OBB;
    OBBCollider() : Collider<OBB>() {}
    ~OBBCollider() {}


    void Edit(Scene* _scene, GameEntity* _entity, const std::string& _parentLabel) override;
    void CalculateWorldShape() override;

public: // accessor
    const Vec3f& getLocalCenter() const { return shape_.center_; }
    void setLocalCenter(const Vec3f& _center) { shape_.center_ = _center; }
    const Vec3f& getLocalHalfSize() const { return shape_.halfSize_; }
    void setLocalHalfSize(const Vec3f& _halfSize) { shape_.halfSize_ = _halfSize; }
    const Vec3f& getWorldCenter() const { return worldShape_.center_; }
    void setWorldCenter(const Vec3f& _center) { worldShape_.center_ = _center; }
    const Vec3f& getWorldHalfSize() const { return worldShape_.halfSize_; }
    void setWorldHalfSize(const Vec3f& _halfSize) { worldShape_.halfSize_ = _halfSize; }

};
