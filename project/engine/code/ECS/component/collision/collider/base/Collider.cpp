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

    // マスク編集（チェックボックス）
    if (ImGui::TreeNode(("Collision Mask##" + _parentLabel).c_str())) {
        for (const auto& [name, category] : categories) {
            bool isChecked = collisionMask_.CanCollideWith(category.GetBits());
            if (ImGui::Checkbox((name + "##mask_" + _parentLabel).c_str(), &isChecked)) {
                if (isChecked) {
                    collisionMask_.SetBits(category.GetBits());
                } else {
                    collisionMask_.ClearBits(category.GetBits());
                }
            }
        }
        ImGui::TreePop();
    }

    // 新規カテゴリ追加
    if (ImGui::TreeNode(("Add Category##" + _parentLabel).c_str())) {
        static char newCategoryName[64] = "";
        ImGui::InputText(("##newCategory_" + _parentLabel).c_str(), newCategoryName, sizeof(newCategoryName));
        ImGui::SameLine();
        if (ImGui::Button(("Add##addCategory_" + _parentLabel).c_str())) {
            if (strlen(newCategoryName) > 0) {
                manager->RegisterCategory(newCategoryName);
                newCategoryName[0] = '\0';
            }
        }
        ImGui::TreePop();
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

    nlohmann::json maskArray          = nlohmann::json::array();
    CollisionCategoryManager* manager = CollisionCategoryManager::GetInstance();
    manager->ForEachCategoryInMask(_c.collisionMask_.GetRaw(), [&](const std::string& name) {
        maskArray.push_back(name);
    });
    _j["collisionMask"] = maskArray;
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

    if (_j.contains("collisionMask")) {
        _c.collisionMask_.Reset();
        CollisionCategoryManager* manager = CollisionCategoryManager::GetInstance();
        for (const auto& name : _j["collisionMask"]) {
            _c.collisionMask_.Set(name.get<std::string>());
        }
    }
}
