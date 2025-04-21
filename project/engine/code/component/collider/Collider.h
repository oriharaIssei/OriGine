#pragma once

/// parent
#include "component/IComponent.h"
/// stl
#include <concepts>
#include <map>

/// engine
/// ECS
#include "ECSManager.h"
// component
#include "component/transform/Transform.h"

/// math
#include "math/shape/Shapes.h"

// external
#include "imgui/imgui.h"

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

    virtual bool Edit()                      = 0;
    virtual void Save(BinaryWriter& _writer) = 0;
    virtual void Load(BinaryReader& _reader) = 0;

    virtual void CalculateWorldShape() = 0;

    virtual void StartCollision();
    virtual void EndCollision();

protected:
    bool isActive_ = true;
    Transform transform_;
    std::map<GameEntity*, CollisionState> collisionStateMap_;
    std::map<GameEntity*, CollisionState> preCollisionStateMap_;

public: // accessor
    bool isActive() const { return isActive_; }
    void setActive(bool _isActive) { isActive_ = _isActive; }

    const Transform& getTransform() const { return transform_; }
    void setParent(Transform* _trans) { transform_.parent = _trans; }

    // 衝突状態の操作
    void setCollisionState(GameEntity* _other) {
        if (this->preCollisionStateMap_[_other] == CollisionState::None)
            this->collisionStateMap_[_other] = CollisionState::Enter;
        else
            this->collisionStateMap_[_other] = CollisionState::Stay;
    }
    const std::map<GameEntity*, CollisionState>& getCollisionStateMap() { return collisionStateMap_; }
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

    virtual bool Edit()                      = 0;
    virtual void Save(BinaryWriter& _writer) = 0;
    virtual void Load(BinaryReader& _reader) = 0;

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
public:
    using ShapeType = AABB;

    AABBCollider()
        : Collider<AABB>() {}
    ~AABBCollider() {}

    bool Edit() override {
        bool isChange = false;
        isChange      = ImGui::Checkbox("IsActive", &this->isActive_);

        if (ImGui::TreeNode("AABB")) {
            isChange |= ImGui::InputFloat3("Min", shape_.min_.v);
            isChange |= ImGui::InputFloat3("Max", shape_.max_.v);
            ImGui::TreePop();
        }
        if (ImGui::TreeNode("Transform")) {
            isChange |= transform_.Edit();
            ImGui::TreePop();
        }
        return isChange;
    }
    void Save(BinaryWriter& _writer) override {
        _writer.Write("isActive", isActive_);
        _writer.Write<3, float>("min", shape_.min_);
        _writer.Write<3, float>("max", shape_.max_);
    }
    void Load(BinaryReader& _reader) override {
        _reader.Read("isActive", isActive_);
        _reader.Read<3, float>("min", shape_.min_);
        _reader.Read<3, float>("max", shape_.max_);
    }

    void CalculateWorldShape() override;

public: // accessor
    const Vec3f& getLocalMin() const { return shape_.min_; }
    void setLocal(const Vec3f& _min) { shape_.min_ = _min; }

    const Vec3f& getLocalMax() const { return shape_.max_; }
    void setLocal(const Vec3f& _max) { shape_.max_ = _max; }

    const Vec3f& getWorldMin() const { return worldShape_.min_; }
    void setWorld(const Vec3f& _min) { worldShape_.min_ = _min; }
    const Vec3f& getWorldMax() const { return worldShape_.max_; }
    void setWorld(const Vec3f& _max) { worldShape_.max_ = _max; }

    const Vec3f& getWorldCenter() const { return (worldShape_.min_ + worldShape_.max_) * 0.5f; }
};

class SphereCollider
    : public Collider<Sphere> {
public:
    using ShapeType = Sphere;

    SphereCollider() : Collider<Sphere>() {}
    ~SphereCollider() {}

    bool Edit() override {
        bool isChange = false;
        isChange      = ImGui::Checkbox("IsActive", &this->isActive_);

        if (ImGui::TreeNode("Sphere")) {
            isChange |= ImGui::InputFloat3("Center", shape_.center_.v);
            isChange |= ImGui::InputFloat("Radius", &shape_.radius_);
            ImGui::TreePop();
        }
        if (ImGui::TreeNode("Transform")) {
            isChange |= transform_.Edit();
            ImGui::TreePop();
        }
        return isChange;
    }
    void Save(BinaryWriter& _writer) override {
        _writer.Write("isActive", isActive_);
        _writer.Write<3, float>("center", shape_.center_);
        _writer.Write("radius", shape_.radius_);
    }
    void Load(BinaryReader& _reader) override {
        _reader.Read("isActive", isActive_);
        _reader.Read<3, float>("center", shape_.center_);
        _reader.Read("radius", shape_.radius_);
    }

    void CalculateWorldShape() override;

public: // accessor
    const Vec3f& getLocalCenter() const { return shape_.center_; }
    void setLocalCenter(const Vec3f& _center) { shape_.center_ = _center; }
    const float& getLocalRadius() const { return shape_.radius_; }
    void setLocalRadius(const float& _radius) { shape_.radius_ = _radius; }

    const Vec3f& getWorldCenter() const { return worldShape_.center_; }
    const float& getWorldRadius() const { return worldShape_.radius_; }
};
