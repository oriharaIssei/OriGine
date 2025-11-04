#include "ISystem.h"

/// engine
#include "scene/Scene.h"

#ifdef _DEBUG
#include "myGui/MyGui.h"
#endif // _DEBUG

void ISystem::eraseDeadEntity() {
    auto* entityRepository_ = scene_->getEntityRepositoryRef();
    std::erase_if(entityIDs_, [&entityRepository_](int32_t _entityID) {
        Entity* entity = entityRepository_->getEntity(_entityID);
        return !entity || !entity->isAlive();
    });
}

Entity* ISystem::getEntity(int32_t _entityID) {
    if (scene_ == nullptr) {
        LOG_ERROR("Scene is not set.");
        return nullptr;
    }
    return scene_->getEntityRepositoryRef()->getEntity(_entityID);
}

Entity* ISystem::getUniqueEntity(const std::string& _dataTypeName) {
    if (scene_ == nullptr) {
        LOG_ERROR("EntityRepository is not set.");
        return nullptr;
    }
    return scene_->getEntityRepositoryRef()->getUniqueEntity(_dataTypeName);
}

int32_t ISystem::CreateEntity(const std::string& _dataTypeName, bool _isUnique) {
    if (scene_ == nullptr) {
        LOG_ERROR("EntityRepository is not set.");
        return -1;
    }
    return scene_->getEntityRepositoryRef()->CreateEntity(_dataTypeName, _isUnique);
}

IComponentArray* ISystem::getComponentArray(const std::string& _typeName) {
    if (scene_ == nullptr) {
        LOG_ERROR("ComponentRepository is not set.");
        return nullptr;
    }
    return scene_->getComponentRepositoryRef()->getComponentArray(_typeName);
}

void ISystem::addComponent(Entity* _entity, const std::string& _typeName, IComponent* _component, bool _doInitialize) {
    if (scene_ == nullptr) {
        LOG_ERROR("ComponentRepository is not set.");
        return;
    }
    scene_->getComponentRepositoryRef()->getComponentArray(_typeName)->addComponent(_entity, _component, _doInitialize);
}

void ISystem::Run() {
#ifndef _RELEASE
    // 計測開始
    deltaTimer_.Initialize();
#endif // _RELEASE

    if (!isActive_) {
        return;
    }

    Update();

#ifndef _RELEASE
    // 計測終了
    deltaTimer_.Update();
    runningTime_ = deltaTimer_.getDeltaTime();
#endif
}

void ISystem::Update() {
    if (entityIDs_.empty()) {
        return;
    }

    eraseDeadEntity();

    for (auto& entityID : entityIDs_) {
        Entity* entity = scene_->getEntityRepositoryRef()->getEntity(entityID);
        UpdateEntity(entity);
    }
}

void ISystem::Edit() {
#ifdef _DEBUG
    // GUI表示
    ImGui::Separator();
    ImGui::Text("SystemCategory: %s", SystemCategoryString[static_cast<int>(category_)].c_str());

    ImGui::SetNextItemWidth(78);
    InputGuiCommand("Priority", priority_, "%d");

    ImGui::Text("EntityCount: %d", static_cast<int>(entityIDs_.size()));

    ImGui::Separator();

    ImGuiTableFlags tableFlags = ImGuiTableFlags_RowBg;

    if (ImGui::TreeNode("Entities")) {
        if (ImGui::BeginTable("Entities", 2, tableFlags)) {
            ImGui::TableSetupColumn("ID");
            ImGui::TableSetupColumn("Type");
            ImGui::TableHeadersRow();

            for (auto& entityID : entityIDs_) {
                Entity* entity = scene_->getEntityRepositoryRef()->getEntity(entityID);
                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0);
                ImGui::Text("%d", entity->getID());
                ImGui::TableSetColumnIndex(1);
                ImGui::Text("%s", entity->getDataType().c_str());
            }

            ImGui::EndTable();
        }
        ImGui::TreePop();
    }

#endif
}
