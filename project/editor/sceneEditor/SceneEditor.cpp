#include "SceneEditor.h"

#ifdef _DEBUG

/// ECS
// system
#include "system/SystemRunner.h"

/// editor
#include "editor/EditorController.h"

#include "editor/sceneEditor/DevelopControlArea.h"
#include "editor/sceneEditor/EntityInspector.h"
#include "editor/sceneEditor/HierarchyArea.h"
#include "editor/sceneEditor/SceneEditorMenus.h"
#include "editor/sceneEditor/SceneViewArea.h"
#include "editor/sceneEditor/SystemInspector.h"

using namespace OriGine;

void SceneEditorWindow::Initialize() {
    InitializeScene();

    InitializeMenus();

    InitializeAreas();

    isMaximized_ = true; // 初期状態で最大化
}

void SceneEditorWindow::DrawGui() {
    if (isSceneChanged_) {
        LoadNextScene();
        isSceneChanged_ = false;
    } else {
        currentScene_->ExecuteDeleteEntities();
        Editor::Window::DrawGui();
    }
}
void SceneEditorWindow::LoadNextScene() {
    if (nextSceneName_.empty()) {
        LOG_ERROR("Next scene name is empty.");
        return;
    }
    FinalizeScene();
    FinalizeMenus();
    FinalizeAreas();

    OriGine::EditorController::GetInstance()->ClearCommandHistory();

    InitializeScene(nextSceneName_);
    InitializeMenus();
    InitializeAreas();

    editSceneName_.SetValue(currentScene_->GetName());
    GlobalVariables::GetInstance()->SaveFile("Settings", "SceneEditor");

    nextSceneName_ = "";
}

void SceneEditorWindow::Finalize() {
    editSceneName_.SetValue(currentScene_->GetName());

    FinalizeScene();

    FinalizeMenus();
    FinalizeAreas();

    GlobalVariables::GetInstance()->SaveFile("Settings", "SceneEditor");
}

void SceneEditorWindow::InitializeMenus() {
    // メニューの初期化
    auto fileMenu = std::make_unique<SceneEditorFileMenu>(this);
    AddMenu(std::move(fileMenu));
}

void SceneEditorWindow::InitializeAreas() {
    // エリアの初期化
    AddArea(std::make_unique<SceneViewArea>(this));
    AddArea(std::make_unique<HierarchyArea>(this));
    AddArea(std::make_unique<EntityInspectorArea>(this));
    AddArea(std::make_unique<SelectAddComponentArea>(this));
    AddArea(std::make_unique<SelectAddSystemArea>(this));
    AddArea(std::make_unique<SystemInspectorArea>(this));
    AddArea(std::make_unique<DevelopControlArea>(this));
}

void SceneEditorWindow::InitializeScene(const std::string& _sceneName) {
    currentScene_ = std::make_unique<Scene>(_sceneName);
    currentScene_->Initialize();
}

void SceneEditorWindow::InitializeScene() {
    InitializeScene(editSceneName_);
}

void SceneEditorWindow::FinalizeMenus() {
    if (menus_.empty()) {
        return;
    }
    // メニューの終了処理
    for (auto& [name, menu] : menus_) {
        menu->Finalize();
    }
    menus_.clear();
}

void SceneEditorWindow::FinalizeAreas() {
    if (areas_.empty()) {
        return;
    }
    // エリアの終了処理
    for (auto& [name, area] : areas_) {
        area->Finalize();
    }
    areas_.clear();
}

void SceneEditorWindow::FinalizeScene() {
    if (!currentScene_) {
        return;
    }
    currentScene_->Finalize();
}

void AddComponentCommand::Execute() {
    auto sceneEditorWindow        = OriGine::EditorController::GetInstance()->GetWindow<SceneEditorWindow>();
    auto currentScene             = sceneEditorWindow->GetCurrentScene();
    auto inspectorArea            = dynamic_cast<EntityInspectorArea*>(sceneEditorWindow->GetArea("EntityInspectorArea").get());
    EntityHandle editEntityHandle = inspectorArea->GetEditEntityHandle();

    for (auto entityId : entityHandles_) {
        Entity* entity = currentScene->GetEntityRepositoryRef()->GetEntity(entityId);
        if (!entity) {
            LOG_ERROR("Entity with Handle : {} not found.", uuids::to_string(entityId.uuid));
            return;
        }

        // コンポーネントの追加
        IComponentArray* compArray = currentScene->GetComponentRepositoryRef()->GetComponentArray(componentTypeName_);
        compArray->AddComponent(currentScene, editEntityHandle);
        if (!compArray) {
            LOG_ERROR("Failed to add component '{}'. \n ", componentTypeName_);
            return;
        }
    }
}

void AddComponentCommand::Undo() {
    auto sceneEditorWindow        = OriGine::EditorController::GetInstance()->GetWindow<SceneEditorWindow>();
    auto currentScene             = sceneEditorWindow->GetCurrentScene();
    auto inspectorArea            = dynamic_cast<EntityInspectorArea*>(sceneEditorWindow->GetArea("EntityInspectorArea").get());
    EntityHandle editEntityHandle = inspectorArea->GetEditEntityHandle();

    for (auto entityId : entityHandles_) {
        Entity* entity = currentScene->GetEntityRepositoryRef()->GetEntity(entityId);
        if (!entity) {
            LOG_ERROR("Entity with Handle : {} not found.", uuids::to_string(entityId.uuid));
            return;
        }
        // コンポーネントの削除
        IComponentArray* compArray = currentScene->GetComponentRepositoryRef()->GetComponentArray(componentTypeName_);
        if (!compArray) {
            LOG_ERROR("ComponentArray '{}' not found.", componentTypeName_);
            return;
        }
        compArray->RemoveComponent(entityId, compArray->GetComponentCount(entityId));
    }
}

void RemoveComponentCommand::Execute() {
    auto sceneEditorWindow        = OriGine::EditorController::GetInstance()->GetWindow<SceneEditorWindow>();
    auto currentScene             = sceneEditorWindow->GetCurrentScene();
    auto inspectorArea            = dynamic_cast<EntityInspectorArea*>(sceneEditorWindow->GetArea("EntityInspectorArea").get());
    EntityHandle editEntityHandle = inspectorArea->GetEditEntityHandle();

    Entity* entity = currentScene->GetEntityRepositoryRef()->GetEntity(entityHandle_);
    if (!entity) {
        LOG_ERROR("Entity with Handle : {} not found.", uuids::to_string(entityHandle_.uuid));
        return;
    }
    // コンポーネントの削除
    IComponentArray* compArray = currentScene->GetComponentRepositoryRef()->GetComponentArray(componentTypeName_);
    if (!compArray) {
        LOG_ERROR("ComponentArray '{}' not found.", componentTypeName_);
        return;
    }
    compArray->RemoveComponent(entityHandle_, componentIndex_);
}

void RemoveComponentCommand::Undo() {
    auto sceneEditorWindow        = OriGine::EditorController::GetInstance()->GetWindow<SceneEditorWindow>();
    auto currentScene             = sceneEditorWindow->GetCurrentScene();
    auto inspectorArea            = dynamic_cast<EntityInspectorArea*>(sceneEditorWindow->GetArea("EntityInspectorArea").get());
    EntityHandle editEntityHandle = inspectorArea->GetEditEntityHandle();

    Entity* entity = currentScene->GetEntityRepositoryRef()->GetEntity(entityHandle_);
    if (!entity) {
        LOG_ERROR("Entity with Handle : {} not found.", uuids::to_string(entityHandle_.uuid));
        return;
    }

    // コンポーネントの追加
    IComponentArray* compArray = currentScene->GetComponentRepositoryRef()->GetComponentArray(componentTypeName_);
    compArray->AddComponent(currentScene, editEntityHandle);
    if (!compArray) {
        LOG_ERROR("Failed to add component '{}'. \n ", componentTypeName_);
        return;
    }
}

AddSystemCommand::AddSystemCommand(const std::list<EntityHandle>& _entityHandles, const std::string& _systemTypeName, SystemCategory _category)
    : entityHandles_(_entityHandles), systemTypeName_(_systemTypeName), systemCategory_(_category) {};

void AddSystemCommand::Execute() {
    auto sceneEditorWindow        = OriGine::EditorController::GetInstance()->GetWindow<SceneEditorWindow>();
    auto currentScene             = sceneEditorWindow->GetCurrentScene();
    auto inspectorArea            = dynamic_cast<EntityInspectorArea*>(sceneEditorWindow->GetArea("EntityInspectorArea").get());
    EntityHandle editEntityHandle = inspectorArea->GetEditEntityHandle();
    if (!currentScene) {
        LOG_ERROR("AddSystemCommand::Execute: No current scene found.");
        return;
    }
    for (auto entityId : entityHandles_) {
        Entity* entity = currentScene->GetEntityRepositoryRef()->GetEntity(entityId);
        if (!entity) {
            LOG_ERROR("AddSystemCommand::Execute: Entity with Handle : {} not found.", uuids::to_string(entityId.uuid));
            continue;
        }
        currentScene->GetSystemRunnerRef()->RegisterEntity(systemTypeName_, entityId);

        if (editEntityHandle == entityId) {
            inspectorArea->GetSystemMap()[int32_t(systemCategory_)][systemTypeName_] = currentScene->GetSystemRunnerRef()->GetSystem(systemTypeName_);
        }
    }
}

void AddSystemCommand::Undo() {
    auto sceneEditorWindow        = OriGine::EditorController::GetInstance()->GetWindow<SceneEditorWindow>();
    auto currentScene             = sceneEditorWindow->GetCurrentScene();
    auto inspectorArea            = dynamic_cast<EntityInspectorArea*>(sceneEditorWindow->GetArea("EntityInspectorArea").get());
    EntityHandle editEntityHandle = inspectorArea->GetEditEntityHandle();
    if (!currentScene) {
        LOG_ERROR("AddSystemCommand::Execute: No current scene found.");
        return;
    }
    for (auto entityId : entityHandles_) {
        Entity* entity = currentScene->GetEntityRepositoryRef()->GetEntity(entityId);
        if (!entity) {
            LOG_ERROR("AddSystemCommand::Execute: Entity with Handle : {} not found.", uuids::to_string(entityId.uuid));
            continue;
        }
        currentScene->GetSystemRunnerRef()->RemoveEntity(systemTypeName_, entityId);

        if (editEntityHandle == entityId) {
            auto itr = inspectorArea->GetSystemMap()[int32_t(systemCategory_)].find(systemTypeName_);
            inspectorArea->GetSystemMap()[int32_t(systemCategory_)].erase(itr);
        }
    }
}

RemoveSystemCommand::RemoveSystemCommand(const std::list<EntityHandle>& _entityIds, const std::string& _systemTypeName, SystemCategory _category)
    : entityHandles_(_entityIds), systemTypeName_(_systemTypeName), systemCategory_(_category) {}

void RemoveSystemCommand::Execute() {
    auto sceneEditorWindow        = OriGine::EditorController::GetInstance()->GetWindow<SceneEditorWindow>();
    auto currentScene             = sceneEditorWindow->GetCurrentScene();
    auto inspectorArea            = dynamic_cast<EntityInspectorArea*>(sceneEditorWindow->GetArea("EntityInspectorArea").get());
    EntityHandle editEntityHandle = inspectorArea->GetEditEntityHandle();

    if (!currentScene) {
        LOG_ERROR("No current scene found.");
        return;
    }
    for (auto entityId : entityHandles_) {
        Entity* entity = currentScene->GetEntityRepositoryRef()->GetEntity(entityId);
        if (!entity) {
            LOG_ERROR("Entity with Handle : {} not found.", uuids::to_string(entityId.uuid));
            continue;
        }
        currentScene->GetSystemRunnerRef()->RemoveEntity(systemTypeName_, entityId);
        if (editEntityHandle == entityId) {
            auto itr = inspectorArea->GetSystemMap()[int32_t(systemCategory_)].find(systemTypeName_);
            inspectorArea->GetSystemMap()[int32_t(systemCategory_)].erase(itr);
        }
    }
}

void RemoveSystemCommand::Undo() {
    auto sceneEditorWindow        = OriGine::EditorController::GetInstance()->GetWindow<SceneEditorWindow>();
    auto currentScene             = sceneEditorWindow->GetCurrentScene();
    auto inspectorArea            = dynamic_cast<EntityInspectorArea*>(sceneEditorWindow->GetArea("EntityInspectorArea").get());
    EntityHandle editEntityHandle = inspectorArea->GetEditEntityHandle();
    if (!currentScene) {
        LOG_ERROR("No current scene found.");
        return;
    }
    for (auto entityId : entityHandles_) {
        Entity* entity = currentScene->GetEntityRepositoryRef()->GetEntity(entityId);
        if (!entity) {
            LOG_ERROR("Entity with Handle : {} not found.", uuids::to_string(entityId.uuid));
            continue;
        }
        currentScene->GetSystemRunnerRef()->RegisterEntity(systemTypeName_, entityId);
        if (editEntityHandle == entityId) {
            inspectorArea->GetSystemMap()[int32_t(systemCategory_)][systemTypeName_] = currentScene->GetSystemRunnerRef()->GetSystem(systemTypeName_);
        }
    }
}

#endif
