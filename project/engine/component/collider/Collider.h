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

    virtual void StartCollision();
    virtual void EndCollision();

protected:
    bool isActive_ = true;
    Transform transform_;
    std::map<GameEntity*, CollisionState> collisionStateMap_;
    std::map<GameEntity*, CollisionState> preCollisionStateMap_;

public: // accessor
    bool isActive() { return isActive_; }
    void setActive(bool _isActive) { isActive_ = _isActive; }

    const Transform& getTransform() const { return transform_; }

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

    // virtual void StartCollision();
    // virtual void EndCollision();

protected:
    ShapeClass shape_;
    ShapeClass worldShape_;

public:
    ShapeClass* getLocalShape() { return &shape_; }
    ShapeClass* getWorldShape() { return &worldShape_; }
};

class AABBCollider
    : public Collider<AABB> {
public:
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
        _writer.Write(isActive_);
        _writer.Write<3, float>(shape_.min_);
        _writer.Write<3, float>(shape_.max_);
    }
    void Load(BinaryReader& _reader) override {
        _reader.Read(isActive_);
        _reader.Read<3, float>(shape_.min_);
        _reader.Read<3, float>(shape_.max_);
    }

    void StartCollision() override {
        Collider::StartCollision();

        if (!isActive_) {
            return;
        }

        this->worldShape_.min_ = Vec3f(transform_.worldMat[3]) + shape_.min_;
        this->worldShape_.max_ = Vec3f(transform_.worldMat[3]) + shape_.max_;
    };
    void EndCollision() override {
        Collider::EndCollision();
    }
};

class SphereCollider
    : public Collider<Sphere> {
public:
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
        _writer.Write(isActive_);
        _writer.Write<3, float>(shape_.center_);
        _writer.Write(shape_.radius_);
    }
    void Load(BinaryReader& _reader) override {
        _reader.Read(isActive_);
        _reader.Read<3, float>(shape_.center_);
        _reader.Read(shape_.radius_);
    }

    void StartCollision() override {
        Collider::StartCollision();

        if (!isActive_) {
            return;
        }

        this->worldShape_.center_ = Vec3f(transform_.worldMat[3]) + shape_.center_;
        this->worldShape_.radius_ = shape_.radius_;
    };
    void EndCollision() override {
        Collider::EndCollision();
    }
};
