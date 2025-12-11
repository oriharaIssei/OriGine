#include "ISystem.h"

/// engine
#include "scene/Scene.h"

/// external
#include "logger/Logger.h"

/// gui
#ifdef _DEBUG
#include "myGui/MyGui.h"
#endif // _DEBUG

namespace OriGine {

void ISystem::EraseDeadEntity() {
    ::std::erase_if(entityIDs_, [&entityRepository = this->entityRepository_](int32_t _entityID) {
        Entity* entity = entityRepository->GetEntity(_entityID);
        return !entity || !entity->IsAlive();
    });
}

Entity* ISystem::GetEntity(int32_t _entityID) {
    if (scene_ == nullptr) {
        LOG_ERROR("Scene is not Set.");
        return nullptr;
    }
    return scene_->GetEntityRepositoryRef()->GetEntity(_entityID);
}

Entity* ISystem::GetUniqueEntity(const ::std::string& _dataTypeName) {
    if (scene_ == nullptr) {
        LOG_ERROR("EntityRepository is not Set.");
        return nullptr;
    }
    return scene_->GetEntityRepositoryRef()->GetUniqueEntity(_dataTypeName);
}

int32_t ISystem::CreateEntity(const ::std::string& _dataTypeName, bool _isUnique) {
    if (scene_ == nullptr) {
        LOG_ERROR("EntityRepository is not Set.");
        return -1;
    }
    return scene_->GetEntityRepositoryRef()->CreateEntity(_dataTypeName, _isUnique);
}

IComponentArray* ISystem::GetComponentArray(const ::std::string& _typeName) {
    if (scene_ == nullptr) {
        LOG_ERROR("ComponentRepository is not Set.");
        return nullptr;
    }
    return scene_->GetComponentRepositoryRef()->GetComponentArray(_typeName);
}

void ISystem::AddComponent(Entity* _entity, const ::std::string& _typeName, IComponent* _component, bool _doInitialize) {
    if (scene_ == nullptr) {
        LOG_ERROR("ComponentRepository is not Set.");
        return;
    }
    scene_->GetComponentRepositoryRef()->GetComponentArray(_typeName)->AddComponent(_entity, _component, _doInitialize);
}

void ISystem::SetScene(Scene* _scene) {
    scene_ = _scene;

    if (!scene_) {
        LOG_ERROR("Scene changing to null");
        entityRepository_    = nullptr;
        componentRepository_ = nullptr;
        return;
    }
    entityRepository_    = scene_->GetEntityRepositoryRef();
    componentRepository_ = scene_->GetComponentRepositoryRef();
}

void ISystem::Run() {
#ifndef _RELEASE
    // 計測開始
    deltaTimer_.Initialize();
#endif // ! _RELEASE

    if (!isActive_) {
        return;
    }

    Update();

#ifndef _RELEASE
    // 計測終了
    deltaTimer_.Update();
#endif
}

void ISystem::Update() {
    if (entityIDs_.empty()) {
        return;
    }

    EraseDeadEntity();

    for (auto& entityID : entityIDs_) {
        Entity* entity = scene_->GetEntityRepositoryRef()->GetEntity(entityID);
        UpdateEntity(entity);
    }
}

void ISystem::Edit() {
#ifdef _DEBUG
    // GUI表示
    ::ImGui::Separator();
    ::ImGui::Text("SystemCategory: %s", kSystemCategoryString[static_cast<int>(category_)].c_str());

    ::ImGui::SetNextItemWidth(78);
    InputGuiCommand("Priority", priority_, "%d");

    ::ImGui::Text("EntityCount: %d", static_cast<int>(entityIDs_.size()));

    ::ImGui::Separator();

    ImGuiTableFlags tableFlags = ImGuiTableFlags_RowBg;

    if (::ImGui::TreeNode("Entities")) {
        if (::ImGui::BeginTable("Entities", 2, tableFlags)) {
            ::ImGui::TableSetupColumn("ID");
            ::ImGui::TableSetupColumn("Type");
            ::ImGui::TableHeadersRow();

            for (auto& entityID : entityIDs_) {
                Entity* entity = scene_->GetEntityRepositoryRef()->GetEntity(entityID);
                ::ImGui::TableNextRow();
                ::ImGui::TableSetColumnIndex(0);
                ::ImGui::Text("%d", entity->GetID());
                ::ImGui::TableSetColumnIndex(1);
                ::ImGui::Text("%s", entity->GetDataType().c_str());
            }

            ::ImGui::EndTable();
        }
        ::ImGui::TreePop();
    }
#endif
}

} // namespace OriGine
