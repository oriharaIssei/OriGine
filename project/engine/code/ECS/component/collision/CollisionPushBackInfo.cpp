#include "CollisionPushBackInfo.h"

#ifdef _DEBUG
#include "myGui/MyGui.h"
#endif // _DEBUG

using namespace OriGine;

void CollisionPushBackInfo::Finalize() {
    collisionInfoMap_.clear();
}

void CollisionPushBackInfo::Edit(Scene* /*_scene*/, EntityHandle /*_entity*/, [[maybe_unused]] [[maybe_unused]] const std::string& _parentLabel) {
#ifdef _DEBUG

    std::string label = "PushBackType##" + _parentLabel;
    if (ImGui::BeginCombo(label.c_str(), GetCollisionPushBackTypeName(pushBackType_))) {
        for (int i = 0; i < static_cast<int>(CollisionPushBackType::Count); ++i) {
            CollisionPushBackType type = static_cast<CollisionPushBackType>(i);
            bool isSelected            = (pushBackType_ == type);
            if (ImGui::Selectable(GetCollisionPushBackTypeName(type), isSelected)) {
                auto command = std::make_unique<SetterCommand<CollisionPushBackType>>(&pushBackType_, type);
                OriGine::EditorController::GetInstance()->PushCommand(std::move(command));
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

const char* OriGine::GetCollisionPushBackTypeName(CollisionPushBackType _type) {
    switch (_type) {
    case CollisionPushBackType::PushBack:
        return "PushBack";
    case CollisionPushBackType::Reflect:
        return "Reflect";
    }
    return "None";
}

void OriGine::to_json(nlohmann::json& _j, const CollisionPushBackInfo& _comp) {
    _j = nlohmann::json{
        {"pushBackType", _comp.pushBackType_}};
}
void OriGine::from_json(const nlohmann::json& _j, CollisionPushBackInfo& _comp) {
    _j.at("pushBackType").get_to(_comp.pushBackType_);
};
