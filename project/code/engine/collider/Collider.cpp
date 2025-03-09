#include "Collider.h"

/// engine
/// ECS
#include "ECSManager.h"
// system
#include "system/Collision/CollisionCheckSystem.h"

/// externals
#include "imgui/imgui.h"

Collider::Collider(GameEntity* _hostEntity)
    : IComponent(_hostEntity) {
    Transform* transform = getComponent<Transform>(_hostEntity);

    transform_.parent = transform;

#ifdef _DEBUG
    // 衝突判定システムに登録
    CollisionCheckSystem* collisionSystem = ECSManager::getInstance()->getSystem<CollisionCheckSystem>();
    if (collisionSystem) {
        collisionSystem->addEntity(_hostEntity);
    }
#endif // _DEBUG
}

void Collider::Init() {
    InitShapeByType(shapeType_);
}

bool Collider::Edit() {
    bool isChange = false;

    isChange = ImGui::Checkbox("IsActive", &isActive_);

    // 形状に関する設定
    if (ImGui::TreeNode("Shape")) {
        ImGui::Text("ShapeType");
        if (ImGui::BeginCombo("##ShapeType", CollisionShapeTypeStr[int32_t(shapeType_)])) {
            for (int32_t i = 0; i < int32_t(CollisionShapeType::Count); ++i) {
                bool isSelected = (int32_t(shapeType_) == i);
                if (ImGui::Selectable(CollisionShapeTypeStr[i], isSelected)) {
                    shapeType_ = CollisionShapeType(i);
                    InitShapeByType(shapeType_);
                    isChange = true;
                }
                if (isSelected) {
                    ImGui::SetItemDefaultFocus();
                }
            }
            ImGui::EndCombo();
        }

        if (shape_) {
            isChange |= shape_->Edit();
        }

        ImGui::TreePop();
    }

    if (ImGui::TreeNode("Transform")) {
        isChange |= transform_.Edit();
    }

    return isChange;
}

void Collider::Save(BinaryWriter& _writer) {
    _writer.Write(isActive_);

    // ------------------------ 形状の保存 ------------------------
    if (shape_) {
        _writer.Write(int32_t(shapeType_));
        shape_->Save(_writer);
    }

    // ------------------------ Transformの保存 ------------------------
    transform_.Save(_writer);
}

void Collider::Load(BinaryReader& _reader) {
    _reader.Read(isActive_);

    // ------------------------ 形状の読み込み ------------------------
    int32_t shapeType;
    _reader.Read(shapeType);
    shapeType_ = CollisionShapeType(shapeType);

    InitShapeByType(shapeType_);
    if (shape_) {
        shape_->Load(_reader);
    }

    // ------------------------ Transformの読み込み ------------------------
    transform_.Load(_reader);
}

void Collider::Finalize() {
    shape_.reset();
    collisionStateMap_.clear();
    preCollisionStateMap_.clear();
}

void Collider::StartCollision() {
    preCollisionStateMap_ = collisionStateMap_;
    collisionStateMap_.clear();

    // Local -> World
    if (!isActive_) {
        return;
    }

    transform_.Update();
    // ToDo : cast による Shape の変換
    if (shapeType_ == CollisionShapeType::Sphere) {
        Sphere* sphere      = reinterpret_cast<Sphere*>(shape_.get());
        Sphere* worldSphere = reinterpret_cast<Sphere*>(worldShape_.get());

        worldSphere->center_ = Vec3f(transform_.worldMat[3]) + sphere->center_;

    } else if (shapeType_ == CollisionShapeType::AABB) {
        AABB* aabb      = reinterpret_cast<AABB*>(shape_.get());
        AABB* worldAABB = reinterpret_cast<AABB*>(worldShape_.get());
        worldAABB->min_ = Vec3f(transform_.worldMat[3]) + aabb->min_;
        worldAABB->max_ = Vec3f(transform_.worldMat[3]) + aabb->max_;
    }
}

void Collider::EndCollision() {
    for (auto& [entity, state] : preCollisionStateMap_) {
        if (state == CollisionState::Exit) {
            return;
        }
        if (collisionStateMap_[entity] == CollisionState::None) {
            collisionStateMap_[entity] = CollisionState::Exit;
        }
    }
}

void Collider::InitShapeByType(CollisionShapeType _type) {
    // Noneは Skip
    if (_type == CollisionShapeType::None) {
        return;
    }

    switch (_type) {
    case CollisionShapeType::Sphere:
        shape_      = std::make_unique<Sphere>();
        worldShape_ = std::make_unique<Sphere>();
        break;
    case CollisionShapeType::AABB:
        shape_      = std::make_unique<AABB>();
        worldShape_ = std::make_unique<AABB>();
        break;
        /* case CollisionShapeType::OBB:
             shape_ = std::make_unique<OBB>();
             break;
         case CollisionShapeType::Capsule:
             shape_ = std::make_unique<Capsule>();
             break;*/
    default:
        break;
    }
}
