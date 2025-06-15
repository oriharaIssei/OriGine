#include "CollisionPushBackInfo.h"

#ifdef _DEBUG
#include "myGui/MyGui.h"
#endif // _DEBUG

void CollisionPushBackInfo::Finalize() {
    collisionInfoMap_.clear();
}

bool CollisionPushBackInfo::Edit() {
    bool isChanged = false;
#ifdef _DEBUG
    if (ImGui::BeginCombo("PushBackType", GetCollisionPushBackTypeName(pushBackType_))) {
        for (int i = 0; i < static_cast<int>(CollisionPushBackType::Count); ++i) {
            CollisionPushBackType type = static_cast<CollisionPushBackType>(i);
            bool isSelected            = (pushBackType_ == type);
            if (ImGui::Selectable(GetCollisionPushBackTypeName(type), isSelected)) {
                auto command = std::make_unique<SetterCommand<CollisionPushBackType>>(&pushBackType_, type);
                EditorController::getInstance()->pushCommand(std::move(command));
                isChanged = true;
            }
            if (isSelected) {
                ImGui::SetItemDefaultFocus();
            }
        }
        ImGui::EndCombo();
    }
#endif // _DEBUG
    return false;
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
