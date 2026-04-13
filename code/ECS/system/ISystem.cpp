#include "ISystem.h"

/// engine
#include "scene/Scene.h"

/// ECS
// entity
#include "entity/Entity.h"

/// external
#include "logger/Logger.h"

/// gui
#ifdef _DEBUG
#include "myGui/MyGui.h"
#endif // _DEBUG

namespace OriGine {

/// <summary>
/// 無効なエンティティの除外
/// </summary>
void ISystem::EraseDeadEntity() {
    ::std::erase_if(entities_, [&entityRepository = this->entityRepository_](EntityHandle _handle) {
        Entity* entity = entityRepository->GetEntity(_handle);
        return !_handle.IsValid() || !entity || !entity->IsAlive();
    });
}

/// <summary>
/// エンティティの取得
/// </summary>
/// <param name="_handle">対象のハンドル</param>
/// <returns>エンティティのポインタ</returns>
Entity* ISystem::GetEntity(EntityHandle _handle) {
    if (scene_ == nullptr) {
        LOG_ERROR("Scene is not Set.");
        return nullptr;
    }
    return scene_->GetEntityRepositoryRef()->GetEntity(_handle);
}

/// <summary>
/// ユニークエンティティの取得
/// </summary>
/// <param name="_dataTypeName">取得する型名</param>
/// <returns>エンティティハンドル</returns>
EntityHandle ISystem::GetUniqueEntity(const ::std::string& _dataTypeName) {
    if (scene_ == nullptr) {
        LOG_ERROR("EntityRepository is not Set.");
        return EntityHandle();
    }
    return scene_->GetEntityRepositoryRef()->GetUniqueEntity(_dataTypeName);
}

/// <summary>
/// エンティティの作成
/// </summary>
/// <param name="_dataTypeName">作成する型名</param>
/// <param name="_isUnique">ユニークなエンティティとして作成するか</param>
/// <returns>作成したエンティティのハンドル</returns>
EntityHandle ISystem::CreateEntity(const ::std::string& _dataTypeName, bool _isUnique) {
    if (scene_ == nullptr) {
        LOG_ERROR("EntityRepository is not Set.");
        return EntityHandle();
    }
    return scene_->GetEntityRepositoryRef()->CreateEntity(_dataTypeName, _isUnique);
}

/// <summary>
/// コンポーネント配列の取得
/// </summary>
/// <param name="_typeName">コンポーネントの型名</param>
/// <returns>コンポーネント配列のインターフェース</returns>
IComponentArray* ISystem::GetComponentArray(const ::std::string& _typeName) {
    if (scene_ == nullptr) {
        LOG_ERROR("ComponentRepository is not Set.");
        return nullptr;
    }
    return scene_->GetComponentRepositoryRef()->GetComponentArray(_typeName);
}

/// <summary>
/// コンポーネントの追加
/// </summary>
/// <param name="_entity">対象のエンティティハンドル</param>
/// <param name="_typeName">追加するコンポーネントの型名</param>
/// <returns>追加したコンポーネントのハンドル</returns>
ComponentHandle ISystem::AddComponent(EntityHandle _entity, const ::std::string& _typeName) {
    if (scene_ == nullptr) {
        LOG_ERROR("ComponentRepository is not Set.");
        return ComponentHandle();
    }
    return scene_->GetComponentRepositoryRef()->GetComponentArray(_typeName)->AddComponent(scene_, _entity);
}

/// <summary>
/// 所属シーンの設定
/// </summary>
/// <param name="_scene">所属させるシーン</param>
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

/// <summary>
/// システムの実行
/// </summary>
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

/// <summary>
/// システムの基本更新処理
/// </summary>
void ISystem::Update() {
    if (entities_.empty()) {
        return;
    }

    EraseDeadEntity();

    for (auto& entityID : entities_) {
        UpdateEntity(entityID);
    }
}

/// <summary>
/// GUI編集処理
/// </summary>
void ISystem::Edit() {
#ifdef _DEBUG
    // GUI表示
    ::ImGui::Separator();
    ::ImGui::Text("SystemCategory: %s", kSystemCategoryString[static_cast<int>(category_)].c_str());

    ::ImGui::SetNextItemWidth(78);
    InputGuiCommand("Priority", priority_, "%d");

    ::ImGui::Text("EntityCount: %d", static_cast<int>(entities_.size()));

    ::ImGui::Separator();

    ImGuiTableFlags tableFlags = ImGuiTableFlags_RowBg;

    if (::ImGui::TreeNode("Entities")) {
        if (::ImGui::BeginTable("Entities", 2, tableFlags)) {
            ::ImGui::TableSetupColumn("ID");
            ::ImGui::TableSetupColumn("Type");
            ::ImGui::TableHeadersRow();

            for (auto& entityID : entities_) {
                Entity* entity = scene_->GetEntityRepositoryRef()->GetEntity(entityID);
                if (!entity) {
                    continue;
                }
                ::ImGui::TableNextRow();
                ::ImGui::TableSetColumnIndex(0);
                ::ImGui::Text("%s", uuids::to_string(entity->GetHandle().uuid).c_str());
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
