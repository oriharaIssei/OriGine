#pragma once
#include "component/IComponent.h"

/// stl
#include <unordered_map>

/// math
#include <math/Vector3.h>

enum class CollisionPushBackType {
    None     = 0,
    PushBack = 1, // 衝突時に押し戻す
    Reflect  = 2, // 衝突時に反射する

    Count
};
const char* GetCollisionPushBackTypeName(CollisionPushBackType type);

class CollisionPushBackInfo
    : public IComponent {
    friend void to_json(nlohmann::json& j, const CollisionPushBackInfo& info);
    friend void from_json(const nlohmann::json& j, CollisionPushBackInfo& info);

public:
    void Initialize(Entity* /*_hostEntity*/) override {}
    void Finalize() override;

    void Edit(Scene* _scene, Entity* _entity, [[maybe_unused]] const std::string& _parentLabel) override;

    void ClearInfo();

public:
    struct Info {
        CollisionPushBackType pushBackType;
        Vec3f collVec; // 衝突ベクトル
        Vec3f collFaceNormal; // 衝突面の法線ベクトル
        Vec3f collPoint; // 衝突点
    };

private:
    CollisionPushBackType pushBackType_ = CollisionPushBackType::None; // 衝突時の挙動
    std::unordered_map<int32_t, Info> collisionInfoMap_;

public:
    CollisionPushBackType GetPushBackType() const { return pushBackType_; }
    void SetPushBackType(CollisionPushBackType type) { pushBackType_ = type; }

    const std::unordered_map<int32_t, Info>& GetCollisionInfoMap() const { return collisionInfoMap_; }
    void SetCollisionInfoMap(const std::unordered_map<int32_t, Info>& map) { collisionInfoMap_ = map; }
    void AddCollisionInfo(int32_t entityId, const Info& info) {
        collisionInfoMap_[entityId] = info;
    }
};
