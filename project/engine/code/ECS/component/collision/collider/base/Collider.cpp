#include "Collider.h"

#include "CollisionCategoryManager.h"

#ifdef _DEBUG
#include "myGui/MyGui.h"
#endif // _DEBUG

using namespace OriGine;

void ICollider::Initialize(Scene* /*_scene*/, EntityHandle /*_entity*/) {}

void OriGine::ICollider::Edit(Scene* /*_scene*/, EntityHandle /*_handle*/, [[maybe_unused]] const std::string& _parentLabel) {
#ifdef _DEBUG

    CheckBoxCommand("IsActive##" + _parentLabel, isActive_);

    // カテゴリ選択Combo
    auto* manager                          = CollisionCategoryManager::GetInstance();
    const auto& categories                 = manager->GetCategories();
    const std::string& currentCategoryName = collisionCategory_.GetName();
    std::string comboLabel                 = "Category##" + _parentLabel;

    if (ImGui::BeginCombo(comboLabel.c_str(), currentCategoryName.c_str())) {
        for (const auto& [name, category] : categories) {
            bool isSelected = (currentCategoryName == name);
            if (ImGui::Selectable(name.c_str(), isSelected)) {
                collisionCategory_ = category;
            }
            if (isSelected) {
                ImGui::SetItemDefaultFocus();
            }
        }
        ImGui::EndCombo();
    }

#endif // _DEBUG
}

void ICollider::StartCollision() {
    this->preCollisionStateMap_ = this->collisionStateMap_;
    this->collisionStateMap_.clear();

    CalculateWorldShape();
}

void ICollider::EndCollision() {
    for (auto& [entity, state] : this->preCollisionStateMap_) {
        if (state == CollisionState::Exit)
            return;
        if (this->collisionStateMap_[entity] == CollisionState::None)
            this->collisionStateMap_[entity] = CollisionState::Exit;
    }
}

void OriGine::to_json(nlohmann::json& _j, const ICollider& _c) {
    _j["isActive"]          = _c.isActive_;
    _j["collisionCategory"] = _c.collisionCategory_.GetName();
}

void OriGine::from_json(const nlohmann::json& _j, ICollider& _c) {
    if (_j.contains("isActive")) {
        _c.isActive_ = _j["isActive"].get<bool>();
    }

    if (_j.contains("collisionCategory")) {
        std::string categoryName          = _j["collisionCategory"].get<std::string>();
        CollisionCategoryManager* manager = CollisionCategoryManager::GetInstance();
        _c.collisionCategory_             = manager->GetCategory(categoryName);
    }
}

bool ICollider::CanCollideWith(const ICollider& _other) const {
    return collisionCategory_.CanCollideWith(_other.collisionCategory_.GetBits());
}
