#include "CollisionPushBackInfo.h"

#ifdef _DEBUG
#include "myGui/MyGui.h"
#endif // _DEBUG

void CollisionPushBackInfo::Finalize() {
    collisionInfoMap_.clear();
}

void CollisionPushBackInfo::Edit(Scene* /*_scene*/, Entity* /*_entity*/, [[maybe_unused]] [[maybe_unused]] const std::string& _parentLabel) {
#ifdef _DEBUG

    std::string label = "PushBackType##" + _parentLabel;
    if (ImGui::BeginCombo(label.c_str(), GetCollisionPushBackTypeName(pushBackType_))) {
        for (int i = 0; i < static_cast<int>(CollisionPushBackType::Count); ++i) {
            CollisionPushBackType type = static_cast<CollisionPushBackType>(i);
            bool isSelected            = (pushBackType_ == type);
            if (ImGui::Selectable(GetCollisionPushBackTypeName(type), isSelected)) {
                auto command = std::make_unique<SetterCommand<CollisionPushBackType>>(&pushBackType_, type);
                EditorController::GetInstance()->PushCommand(std::move(command));
            }
            if (isSelected) {
                ImGui::SetItemDefaultFocus();
            }
        }
        ImGui::EndCombo();
    }

#endif // _DEBUG
}

void CollisionPushBackInfo::ClearInfo() {
    collisionInfoMap_.clear();
}

const char* GetCollisionPushBackTypeName(CollisionPushBackType type) {
    switch (type) {
    case CollisionPushBackType::PushBack:
        return "PushBack";
    case CollisionPushBackType::Reflect:
        return "Reflect";
    }
    return "None";
}

void to_json(nlohmann::json& j, const CollisionPushBackInfo& info) {
    j = nlohmann::json{
        {"pushBackType", info.pushBackType_}};
}
void from_json(const nlohmann::json& j, CollisionPushBackInfo& info) {
    j.at("pushBackType").get_to(info.pushBackType_);
};
