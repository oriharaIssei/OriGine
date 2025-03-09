#pragma once

/// parent
#include "component/IComponent.h"

/// engine
// component
#include "component/transform/Transform.h"

/// stl
#include <concepts>
#include <map>
#include <memory>

/// math
#include "shape/Shapes.h"

enum class CollisionShapeType {
    None   = 0b0,
    Sphere = 0b1,
    AABB   = 0b10,
    // OBB    = 0b100,
    // Capsule = 0b100,

    Count = 2
};

static const char* CollisionShapeTypeStr[] = {
    "None",
    "Sphere",
    "AABB",
    //"OBB",
    //"Capsule",

};

enum class CollisionState {
    None, // 衝突していない
    Stay, // 衝突中
    Enter, // 衝突開始時
    Exit // 衝突終了時
};

class Collider
    : public IComponent {
public:
    Collider(GameEntity* _hostEntity);
    virtual ~Collider() {}

    virtual void Init();

    virtual bool Edit();
    virtual void Save(BinaryWriter& _writer);
    virtual void Load(BinaryReader& _reader);

    virtual void Finalize();

    /// <summary>
    /// 衝突判定の記録開始処理
    /// </summary>
    void StartCollision();
    /// <summary>
    /// 衝突判定の記録の終了処理
    /// </summary>
    void EndCollision();

private:
    void InitShapeByType(CollisionShapeType _type);

private:
    // 衝突判定を行うかどうか
    bool isActive_ = true;

    CollisionShapeType shapeType_;

    std::unique_ptr<IShape> shape_;
    std::unique_ptr<IShape> worldShape_;

    Transform transform_;

    std::map<GameEntity*, CollisionState> collisionStateMap_;
    std::map<GameEntity*, CollisionState> preCollisionStateMap_;

public:
    bool isActive() const { return isActive_; }
    void setActive(bool _isActive) { isActive_ = _isActive; }

    CollisionShapeType getShapeType() const { return shapeType_; }
    void setShapeType(CollisionShapeType _type) {
        shapeType_ = _type;
        InitShapeByType(shapeType_);
    }
    IShape* getLocalShape() const { return shape_.get(); }
    IShape* getWorldShape() const { return worldShape_.get(); }

    void setCollisionState(GameEntity* _other) {
        if (preCollisionStateMap_[_other] == CollisionState::None) {
            collisionStateMap_[_other] = CollisionState::Enter;
            return;
        }
        collisionStateMap_[_other] = CollisionState::Stay;
    }
};
