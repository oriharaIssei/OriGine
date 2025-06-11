#include "ECSEditor.h"

/// stl
#include <algorithm>

/// engine
#include "sceneManager/SceneManager.h"
#define RESOURCE_DIRECTORY
#include "EngineInclude.h"
#include "input/Input.h"
// directX12
#include "directX12/RenderTexture.h"

// transform
#include "component/renderer/MeshRenderer.h"
#include "component/transform/Transform.h"
// editor
#include "module/editor/EditorController.h"

/// lib
#include "camera/CameraManager.h"
#include "myFileSystem/MyFileSystem.h"

#ifdef _DEBUG

/// externals
#include "myGui/MyGui.h"
#include <imgui/ImGuizmo/ImGuizmo.h>

ECSEditor::ECSEditor() {}

ECSEditor::~ECSEditor() {}

void ECSEditor::Initialize() {
    ecsManager_ = ECSManager::getInstance();

    editEntity_ = nullptr;

    SortPriorityOrderFromECSManager();
}

void ECSEditor::Update() {
    SelectEntity();
    EditEntity();

    WorkerSystemList();

    SelectEntityFromCursol();
    GuizmoEdit();
}

void ECSEditor::Finalize() {
    ecsManager_ = nullptr;
    editEntity_ = nullptr;

    editEntityComponents_.clear();
    for (int32_t i = 0; i < int32_t(SystemType::Count); i++) {
        workSystemList_[i].clear();
    }
}

void ECSEditor::SelectEntity() {
    if (ImGui::Begin("Entities")) {

        // Command経由に変更
        if (ImGui::IsWindowHovered(ImGuiHoveredFlags_None) && ImGui::IsMouseClicked(ImGuiMouseButton_Right)) {
            ImGui::OpenPopup("EntityOption");
        }
        if (ImGui::BeginPopup("EntityOption")) {
            if (ImGui::Button("AddEntity")) {
                auto command = std::make_unique<CreateEntityCommand>(this);

                EditorController::getInstance()->pushCommand(std::move(command));
            }

            if (ImGui::Button("Add Entity From File")) {
                std::string directory, fileName;
                bool isSelected = myfs::selectFileDialog(kApplicationResourceDirectory, directory, fileName, {"ent"}, true);
                if (isSelected) {
                    auto command = std::make_unique<CreateEntityFromFileCommand>(this, directory, fileName);
                    EditorController::getInstance()->pushCommand(std::move(command));
                }
            }

            // 選ばれたエンティティが あれば 表示
            if (!selectedentityIDs_.empty()) {
                if (ImGui::Button("Group Join Work System")) {
                    popupJoinWorkSystem_.isOpen_ = true;
                }
                if (ImGui::Button("Group Add Component")) {
                    popupAddComponent_.isOpen_ = true;
                }
                if (ImGui::Button("Group Leave Work System")) {
                    popupLeaveWorkSystem_.isOpen_ = true;
                }
            }

            ImGui::EndPopup();
        }

        if (ImGui::Button("Add Entity")) {
            auto command = std::make_unique<CreateEntityCommand>(this);

            EditorController::getInstance()->pushCommand(std::move(command));
        }
        if (ImGui::Button("Add Entity From File")) {
            std::string directory, fileName;
            bool isSelected = myfs::selectFileDialog(kApplicationResourceDirectory, directory, fileName, {"ent"}, true);
            if (isSelected) {
                auto command = std::make_unique<CreateEntityFromFileCommand>(this, directory, fileName);

                EditorController::getInstance()->pushCommand(std::move(command));
            }
        }
        // 選ばれたエンティティが あれば表示
        if (!selectedentityIDs_.empty()) {

            if (ImGui::Button("Group Erase")) {
                auto command = std::make_unique<GroupEraseEntityCommand>(this);

                EditorController::getInstance()->pushCommand(std::move(command));
            }

            ImGui::Spacing();
            if (ImGui::Button("Group Add Component")) {
                popupAddComponent_.isOpen_ = true;
            }
            if (ImGui::Button("Group Join Work System")) {
                popupJoinWorkSystem_.isOpen_ = true;
            }
            if (ImGui::Button("Group Leave Work System")) {
                popupLeaveWorkSystem_.isOpen_ = true;
            }
        }

        ImGui::Separator();
        ImGui::Text("Entity List");

        static char searchBuffer[256] = ""; // 検索用のバッファ
        ImGui::InputText("Search", searchBuffer, sizeof(searchBuffer));

        // Active Entities
        bool isEntitySelected  = false;
        std::string checkBoxID = "";
        std::string entityName = "";

        for (auto& entity : ecsManager_->getEntities()) {
            if (entity.isAlive()) {
                entityName = entity.isUnique() ? entity.getDataType() : entity.getUniqueID();
                // 検索フィルタリング
                if (strlen(searchBuffer) > 0 && entityName.find(searchBuffer) == std::string::npos) {
                    continue; // 検索文字列に一致しない場合はスキップ
                }

                isEntitySelected = false;
                isEntitySelected = std::find(selectedentityIDs_.begin(), selectedentityIDs_.end(), &entity) != selectedentityIDs_.end();

                checkBoxID = "##entitySelect_" + entity.getUniqueID();
                if (ImGui::Checkbox(checkBoxID.c_str(), &isEntitySelected)) {
                    if (!isEntitySelected) {
                        selectedentityIDs_.remove(&entity);
                    } else {
                        selectedentityIDs_.push_back(&entity);
                    }
                }

                ImGui::SameLine();

                if (ImGui::Button(entityName.c_str())) {
                    auto command = std::make_unique<SelectEntityCommand>(this, const_cast<GameEntity*>(&entity));
                    EditorController::getInstance()->pushCommand(std::move(command));
                }
            }
        }
        PopupEntityJoinWorkSystem(editEntity_, true);
        PopupEntityAddComponent(editEntity_, true);
        PopupEntityLeaveWorkSystem(editEntity_, true);
    }

    ImGui::End();
}

void ECSEditor::EditEntity() {
    std::string label = "Entity Info";

    if (ImGui::Begin("Entity Info")) {
        // Entityが選択されていなければ Skip
        if (editEntity_ == nullptr || !editEntity_->isAlive()) {
            ImGui::Text("Entity is not selected.");
            ImGui::End();
            return;
        }

        // 右クリック + ホバーでポップアップを表示
        if (ImGui::IsWindowHovered(ImGuiHoveredFlags_None) && ImGui::IsMouseClicked(ImGuiMouseButton_Right)) {
            ImGui::OpenPopup("EntityOption");
        }

        // PopUp で Entity に対する操作を表示
        if (ImGui::BeginPopup("EntityOption")) {
            if (ImGui::Button("AddComponent")) {
                popupAddComponent_.isOpen_ = true;
            }
            if (ImGui::Button("Join Work System")) {
                popupJoinWorkSystem_.isOpen_ = true;
            }
            ImGui::EndPopup();
        }

        if (ImGui::Button("Save To File")) {
            auto command = std::make_unique<SaveEntityToFileCommand>(this, editEntity_, "entities");
            EditorController::getInstance()->pushCommand(std::move(command));
        }
        if (ImGui::Button("Erase This")) {
            auto command = std::make_unique<EraseEntityCommand>(this, editEntity_);

            EditorController::getInstance()->pushCommand(std::move(command));

            ImGui::End();
            return;
        }

        ImGui::Text("Entity UniqueID   : %s", editEntity_->getUniqueID().c_str());
        ImGui::Text("Entity ID         : %d", editEntity_->getID());

        ImGui::Text("Entity Name       :");
        ImGui::SameLine();

        char buffer[64];
        strncpy_s(buffer, editEntity_->getDataType().c_str(), sizeof(buffer) - 1);
        buffer[sizeof(buffer) - 1] = '\0'; // 念のため終端文字を保証

        // ImGui::InputText を使用
        if (ImGui::InputText("##entityName", buffer, sizeof(buffer))) {
            // 入力が変更された場合のみ更新
            std::string newDataType(buffer);
            editEntity_->setDataType(newDataType);
        }

        ImGui::Text("Entity Is Unique  :");
        ImGui::SameLine();
        bool isUnique = editEntity_->isUnique();
        if (ImGui::Checkbox("##entityIsUnique", &isUnique)) {
            if (isUnique) {
                // Unique に設定 (登録できたら true,重複などで登録できなければ false)
                ecsManager_->registerUniqueEntity(editEntity_);
            } else {
                // Unique から解除
                ecsManager_->removeUniqueEntity(editEntity_->getDataType());
            }
        }

        ImGui::Separator();

        ImGui::Text("Components");
        ImGui::SameLine();
        if (ImGui::Button("AddComponent")) {
            popupAddComponent_.isOpen_ = true;
        }

        // 空のコンポーネントVectorを削除
        std::erase_if(editEntityComponents_, [](const std::pair<std::string, std::deque<IComponent*>>& compVec) { return compVec.second.empty(); });

        // コンポーネントの表示
        std::string componentLabel = "";
        int32_t componentIndex     = 0;

        for (auto& [compTypeName, compVec] : editEntityComponents_) {
            componentLabel = "";
            componentIndex = 0;
            for (auto& comp : compVec) {
                componentLabel = "X##" + compTypeName + "[" + std::to_string(componentIndex) + "]";
                if (ImGui::Button(componentLabel.c_str())) {
                    // 遅延実行用に削除コマンドをプッシュ
                    auto command = std::make_unique<RemoveComponentCommand>(this, editEntity_, compTypeName, componentIndex);
                    EditorController::getInstance()->pushCommand(std::move(command));
                }
                ImGui::SameLine();

                componentLabel = compTypeName + "[" + std::to_string(componentIndex) + "]";
                if (ImGui::CollapsingHeader(componentLabel.c_str())) {
                    ImGui::Indent();

                    comp->Edit();

                    ImGui::Unindent();
                }

                componentIndex++;
            }
        }

        ImGui::Separator();

        ImGui::Text("Work Systems");
        ImGui::SameLine();
        if (ImGui::Button("Join Work System")) {
            popupJoinWorkSystem_.isOpen_ = true;
        }

        for (int32_t systemTypeIndex = 0; systemTypeIndex < int32_t(SystemType::Count); ++systemTypeIndex) {
            if (ImGui::CollapsingHeader(SystemTypeString[systemTypeIndex].c_str())) {
                for (auto& [systemName, system] : editEntitySystems_[systemTypeIndex]) {
                    // Popupで 処理するために保持
                    if (ImGui::Button(systemName.c_str())) {
                        popupLeaveWorkSystem_.isOpen_ = true;
                        leaveSystemName_              = systemName;
                        leaveSystem_                  = system;
                    }
                }
            }
        }
    }

    ImGui::End();

    PopupEntityJoinWorkSystem(editEntity_, false);
    PopupEntityAddComponent(editEntity_, false);
    PopupEntityLeaveWorkSystem(editEntity_, false);
}

void ECSEditor::WorkerSystemList() {
    if (ImGui::Begin("Worker System List")) {
        std::string systemLabel = "";
        int32_t systemTypeIndex = 0;

        for (auto& systemByType : workSystemList_) {
            // Typeごとで区切る
            if (ImGui::CollapsingHeader(SystemTypeString[systemTypeIndex].c_str())) {
                ImGui::Indent();

                SortPriorityOrderFromECSManager(systemTypeIndex);

                for (auto& [systemName, system] : systemByType) {
                    int32_t systemPriority    = system->getPriority();
                    int32_t preSystemPriority = systemPriority;
                    bool systemIsActive       = system->isActive();
                    bool preSystemIsActive    = systemIsActive;
                    systemLabel               = "##" + systemName + "isActive";

                    // チェックボックス
                    ImGui::Checkbox(systemLabel.c_str(), &systemIsActive);
                    ImGui::SameLine();

                    // Priority入力
                    ImGui::PushItemWidth(78);
                    systemLabel    = "##" + systemName + "_Priority";
                    systemPriority = system->getPriority();
                    if (ImGui::InputInt(systemLabel.c_str(), &systemPriority)) {
                        system->setPriority(systemPriority);
                    }
                    ImGui::PopItemWidth();
                    ImGui::SameLine();

                    // システム名をSelectableで表示（IDを持たせる）
                    std::string selectableLabel = systemName + "##SystemName";
                    bool selected               = false;
                    ImGui::Selectable(selectableLabel.c_str(), &selected, ImGuiSelectableFlags_AllowDoubleClick);

                    // Drag & Drop Source/TargetはSelectable直後で
                    if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID)) {
                        ImGui::SetDragDropPayload("SYSTEM_PRIORITY", &systemPriority, sizeof(int));
                        ImGui::Text("Dragging %s", systemName.c_str());
                        ImGui::EndDragDropSource();
                    }
                    if (ImGui::BeginDragDropTarget()) {
                        if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("SYSTEM_PRIORITY")) {
                            IM_ASSERT(payload->DataSize == sizeof(int) && "Payload data size mismatch.");
                            int droppedPriority = *(const int*)payload->Data;

                            // ドロップ位置による優先度調整（Selectableの矩形を使う）
                            ImVec2 itemRectMin    = ImGui::GetItemRectMin();
                            ImVec2 itemRectMax    = ImGui::GetItemRectMax();
                            ImVec2 itemRectCenter = ImVec2((itemRectMin.x + itemRectMax.x) * 0.5f, (itemRectMin.y + itemRectMax.y) * 0.5f);

                            if (ImGui::IsMouseHoveringRect(itemRectMin, itemRectCenter)) {
                                systemPriority = droppedPriority - 1;
                            } else if (ImGui::IsMouseHoveringRect(itemRectCenter, itemRectMax)) {
                                systemPriority = droppedPriority + 1;
                            }
                        }
                        ImGui::EndDragDropTarget();
                    }

                    std::string popupId = "SystemPopup_" + systemName;
                    if (ImGui::IsItemClicked(ImGuiMouseButton_Left)) {
                        ImGui::OpenPopup(popupId.c_str());
                    }
                    if (ImGui::BeginPopup(popupId.c_str())) {
                        ImGui::Text("%s", systemName.c_str());
                        system->Edit();
                        ImGui::EndPopup();
                    }

                    // コマンド発行
                    if (systemIsActive != preSystemIsActive) {
                        auto command = std::make_unique<ChangingSystemActivityCommand>(this, systemName, system);
                        EditorController::getInstance()->pushCommand(std::move(command));
                    }
                    if (systemPriority != preSystemPriority) {
                        auto command = std::make_unique<ChangingSystemPriorityCommand>(this, system, systemPriority);
                        EditorController::getInstance()->pushCommand(std::move(command));
                    }
                }

                ImGui::Unindent();
            }
            systemTypeIndex++;
        }
    }
    ImGui::End();
}

void ECSEditor::PopupEntityJoinWorkSystem(GameEntity* _entity, bool _isGroup) {
    if (!popupJoinWorkSystem_.isOpen_) {
        return;
    }

    if (_isGroup == selectedentityIDs_.empty()) {
        return;
    }

    static char searchBuffer[128] = ""; // 検索用のバッファ
    ImGui::Begin("Join Work System", &popupJoinWorkSystem_.isOpen_);
    if (popupJoinWorkSystem_.isOpen_ != false) {
        popupJoinWorkSystem_.isOpen_ = ImGui::IsWindowFocused();
    }

    ImGui::Text("Work Systems");

    ImGui::Spacing();

    // 検索欄
    ImGui::InputText("Search##WorkSysterm", searchBuffer, sizeof(searchBuffer));
    ImGui::Separator();
    int systemTypeIndex = 0;
    for (auto& systemByType : ecsManager_->getSystems()) {

        // 検索に一致するシステムがあるかを確認
        bool hasMatchingSystem = false;
        for (auto& [systemName, system] : systemByType) {
            if (strlen(searchBuffer) == 0 || systemName.find(searchBuffer) != std::string::npos) {
                hasMatchingSystem = true;
                break;
            }
        }

        // 一致する場合は CollapsingHeader を開く
        if (hasMatchingSystem) {
            ImGui::SetNextItemOpen(true);
        } else if (strlen(searchBuffer) != 0) {
            ImGui::SetNextItemOpen(false);
        }

        if (ImGui::CollapsingHeader(SystemTypeString[systemTypeIndex].c_str())) {
            for (auto& [systemName, system] : systemByType) {
                // 検索フィルタリング
                if (strlen(searchBuffer) > 0 && systemName.find(searchBuffer) == std::string::npos) {
                    continue; // 検索文字列に一致しない場合はスキップ
                }
                if (ImGui::Button(systemName.c_str())) {
                    if (_isGroup) {
                        // GroupCommand経由に変更
                        auto command = std::make_unique<GroupJoinWorkSystemCommand>(this, systemName, system.get());

                        EditorController::getInstance()->pushCommand(std::move(command));

                    } else {
                        // commandから 実行
                        auto command = std::make_unique<JoinWorkSystemCommand>(this, _entity, systemName, system.get());

                        EditorController::getInstance()->pushCommand(std::move(command));
                    }
                    // 実行したら閉じる
                    popupJoinWorkSystem_.isOpen_ = false;
                }
            }
        }
        ++systemTypeIndex;
    }

    ImGui::End();
}

void ECSEditor::PopupEntityAddComponent(GameEntity* _entity, bool _isGroup) {
    if (!popupAddComponent_.isOpen_) {
        return;
    }

    if (_isGroup == selectedentityIDs_.empty()) {
        return;
    }

    ImGui::Begin("AddComponent", &popupAddComponent_.isOpen_);
    if (popupAddComponent_.isOpen_ != false) {
        popupAddComponent_.isOpen_ = ImGui::IsWindowFocused();
    }

    ImGui::Spacing();

    static char searchBuffer[128] = ""; // 検索用のバッファ
    // 検索欄
    ImGui::InputText("Search##AddComponent", searchBuffer, sizeof(searchBuffer));
    ImGui::Separator();

    // コンポーネントの追加 → Command経由に変更
    for (auto& [componentTypeName, componentArray] : ecsManager_->getComponentArrayMap()) {
        // 検索フィルタリング
        if (strlen(searchBuffer) > 0 && componentTypeName.find(searchBuffer) == std::string::npos) {
            continue; // 検索文字列に一致しない場合はスキップ
        }

        if (ImGui::Button(componentTypeName.c_str())) {
            if (_isGroup) {
                // GroupCommand経由に変更
                auto command = std::make_unique<GroupAddComponentCommand>(this, componentTypeName);

                EditorController::getInstance()->pushCommand(std::move(command));
            } else {
                // commandから 実行
                auto command = std::make_unique<AddComponentCommand>(this, _entity, componentTypeName);

                EditorController::getInstance()->pushCommand(std::move(command));
            }
            // 実行したら閉じる
            popupAddComponent_.isOpen_ = false;
        }
    }
    ImGui::End();
}

void ECSEditor::PopupEntityLeaveWorkSystem(GameEntity* _entity, bool _isGroup) {
    if (!popupLeaveWorkSystem_.isOpen_) {
        return;
    };

    if (_isGroup == selectedentityIDs_.empty()) {
        return;
    }

    // IsGroup の場合, leaveSystemを決められるように
    ImGui::Begin("Leave Work System", &popupLeaveWorkSystem_.isOpen_);
    if (popupLeaveWorkSystem_.isOpen_ != false) {
        popupLeaveWorkSystem_.isOpen_ = ImGui::IsWindowFocused();
    }

    if (_isGroup) {
        if (!leaveSystem_) {
            ImGui::Text("Work Systems");
            ImGui::Spacing();
            static char searchBuffer[128] = ""; // 検索用のバッファ
            // 検索欄
            ImGui::InputText("Search##leaveSystem", searchBuffer, sizeof(searchBuffer));

            ImGui::Separator();
            int systemTypeIndex = 0;
            for (auto& systemByType : ecsManager_->getSystems()) {
                if (ImGui::CollapsingHeader(SystemTypeString[systemTypeIndex].c_str())) {
                    // 検索に一致するシステムがあるかを確認
                    bool hasMatchingSystem = false;
                    for (auto& [systemName, system] : systemByType) {
                        if (strlen(searchBuffer) == 0 || systemName.find(searchBuffer) != std::string::npos) {
                            hasMatchingSystem = true;
                            break;
                        }
                    }

                    // 一致する場合は CollapsingHeader を開く
                    if (hasMatchingSystem) {
                        ImGui::SetNextItemOpen(true);
                    } else if (strlen(searchBuffer) != 0) {
                        ImGui::SetNextItemOpen(false);
                    }

                    for (auto& [systemName, system] : systemByType) {
                        if (strlen(searchBuffer) == 0 || systemName.find(searchBuffer) != std::string::npos) {
                            continue;
                        }
                        if (ImGui::Button(systemName.c_str())) {
                            leaveSystemName_ = systemName;
                            leaveSystem_     = system.get();
                        }
                    }
                }
                ++systemTypeIndex;
            }
        }
        ImGui::Separator();
    }

    if (leaveSystem_) {
        ImGui::Text("Leave %s ?", leaveSystemName_.c_str());

        if (ImGui::Button("Yes")) {
            if (_isGroup) {
                // GroupCommand経由に変更
                auto command = std::make_unique<GroupLeaveWorkSystemCommand>(this, leaveSystemName_, leaveSystem_);

                EditorController::getInstance()->pushCommand(std::move(command));
            } else {
                // commandから 実行
                auto command = std::make_unique<LeaveWorkSystemCommand>(this, _entity, leaveSystemName_, leaveSystem_);

                EditorController::getInstance()->pushCommand(std::move(command));
            }

            // 実行したら閉じる
            popupLeaveWorkSystem_.isOpen_ = false;
            leaveSystem_                  = nullptr;
            leaveSystemName_.clear();
        }
        ImGui::SameLine();
        if (ImGui::Button("Cancel")) {
            popupLeaveWorkSystem_.isOpen_ = false;

            leaveSystem_ = nullptr;
            leaveSystemName_.clear();
        }
    }

    ImGui::End();
}

void ECSEditor::SelectEntityFromCursol() {
    Input* input = Input::getInstance();

    /// 左クリックが Release された時のみ処理を行う
    if (!input->isReleaseMouseButton(MouseButton::LEFT)) {
        return;
    }

    const Vec2f& mousePosition = input->getVirtualMousePos();
    if (mousePosition[X] < 0 || mousePosition[Y] < 0) {
        return; // マウスがウィンドウ外にある場合は無視
    }
    const Vec2f& windowSize = SceneManager::getInstance()->getSceneView()->getTextureSize();
    if (mousePosition[X] > windowSize[X] || mousePosition[Y] > windowSize[Y]) {
        return;
    }

    ECSManager* ecsManager = ECSManager::getInstance();

    const CameraTransform& cameraTransform = CameraManager::getInstance()->getTransform();

    Matrix4x4 vpvpvMat = cameraTransform.viewMat
                         * cameraTransform.projectionMat
                         * MakeMatrix::ViewPort(0.0f, 0.0f, windowSize[X], windowSize[Y], 0.0f, 1.0f);

    auto transformArray = ecsManager->getComponentArray<Transform>();
    // auto modelMeshRendererArray = ecsManager->getComponentArray<ModelMeshRenderer>();

    static const float kCursolRadius = 10.0f; // カーソルの半径
    std::list<std::pair<GameEntity*, float>> entityZDistanceList;
    for (auto& [entityIndex, transformVecIndex] : transformArray->getEntityIndexBind()) {
        GameEntity* entity = ecsManager->getEntity(entityIndex);
        if (!entity || !entity->isAlive()) {
            continue; // エンティティが存在しない、または生存していない場合はスキップ
        }

        auto transformVec = transformArray->getComponents(entity);
        for (auto itr = transformVec->begin(); itr != transformVec->end(); ++itr) {
            const Transform& transform = *itr;

            // 画面上の位置を取得
            Vec2f screenPos = WorldToScreen(transform.worldMat[3], vpvpvMat);
            if (screenPos[X] < -kCursolRadius || screenPos[Y] < -kCursolRadius) {
                continue; // 画面外のエンティティは無視
            }
            if (screenPos[X] > windowSize[X] + kCursolRadius || screenPos[Y] > windowSize[Y] + kCursolRadius) {
                continue; // 画面外のエンティティは無視
            }

            // マウス位置との距離を計算
            float distance = Vec2f(screenPos - mousePosition).lengthSq();

            if (distance > kCursolRadius * kCursolRadius) {
                continue; // カーソルの半径外は無視
            }

            // Z軸の距離を計算
            float zDistance = transform.worldMat[3][Z] - cameraTransform.translate[Z];
            zDistance       = std::abs(zDistance); // Z軸の距離は絶対値を取る

            // 距離とエンティティをリストに追加
            entityZDistanceList.emplace_back(entity, zDistance);
        }
    }

    if (entityZDistanceList.empty()) {
        return;
    }
    // Z軸の距離でソート
    entityZDistanceList.sort([](const std::pair<GameEntity*, float>& a, const std::pair<GameEntity*, float>& b) {
        return a.second < b.second; // Z軸の距離が近い順にソート
    });
    // 最も近いエンティティを選択
    GameEntity* closestEntity = entityZDistanceList.front().first;
    // 選択されたエンティティを編集対象に設定
    auto command = std::make_unique<SelectEntityCommand>(this, closestEntity);
    EditorController::getInstance()->pushCommand(std::move(command));
}

void ECSEditor::GuizmoEdit() {
    if (!editEntity_) {
        return;
    }

    ECSManager* ecsManager = ECSManager::getInstance();
    auto transformArray    = ecsManager->getComponentArray<Transform>();

    // Transformを持っていないエンティティは Skip
    if (!transformArray->hasEntity(editEntity_)) {
        return;
    }

    Transform* transform = getComponent<Transform>(editEntity_);
    if (!transform) {
        return;
    }
    /// ==========================================
    // Guizmo Edit

    // 行列の用意
    float matrix[16];
    transform->worldMat.toFloatArray(matrix); // Transform の worldMat を float[16] に変換
    // ビュー行列とプロジェクション行列の取得
    float viewMatrix[16];
    float projectionMatrix[16];
    CameraManager::getInstance()->getTransform().viewMat.toFloatArray(viewMatrix); // カメラのビュー行列を取得
    CameraManager::getInstance()->getTransform().projectionMat.toFloatArray(projectionMatrix); // カメラのプロジェクション行列を取得

    // ギズモの操作タイプ
    static ImGuizmo::OPERATION currentGizmoOperation = ImGuizmo::TRANSLATE | ImGuizmo::SCALE | ImGuizmo::ROTATE;

    [](ImGuizmo::OPERATION& _currentGizmoOperation) {
        Input* input = Input::getInstance();
        if (input->isPressKey(Key::L_SHIFT)) {
            if (input->isPressKey(Key::S)) {
                if (input->isPressKey(Key::X)) {
                    _currentGizmoOperation = ImGuizmo::SCALE_X;
                } else if (input->isPressKey(Key::Y)) {
                    _currentGizmoOperation = ImGuizmo::SCALE_Y;
                } else if (input->isPressKey(Key::Z)) {
                    _currentGizmoOperation = ImGuizmo::SCALE_Z;
                } else {
                    _currentGizmoOperation = ImGuizmo::SCALE; // Shift + S でスケール
                }
            } else if (input->isPressKey(Key::R)) {
                if (input->isPressKey(Key::X)) {
                    _currentGizmoOperation = ImGuizmo::ROTATE_X;
                } else if (input->isPressKey(Key::Y)) {
                    _currentGizmoOperation = ImGuizmo::ROTATE_Y;
                } else if (input->isPressKey(Key::Z)) {
                    _currentGizmoOperation = ImGuizmo::ROTATE_Z;
                } else {
                    _currentGizmoOperation = ImGuizmo::ROTATE; // Shift + R で回転
                }
            } else if (input->isPressKey(Key::T)) {
                if (input->isPressKey(Key::X)) {
                    _currentGizmoOperation = ImGuizmo::TRANSLATE_X;
                } else if (input->isPressKey(Key::Y)) {
                    _currentGizmoOperation = ImGuizmo::TRANSLATE_Y;
                } else if (input->isPressKey(Key::Z)) {
                    _currentGizmoOperation = ImGuizmo::TRANSLATE_Z;
                } else {
                    _currentGizmoOperation = ImGuizmo::TRANSLATE; // Shift + T で移動
                }
            }

        } else {
            _currentGizmoOperation = ImGuizmo::TRANSLATE | ImGuizmo::SCALE | ImGuizmo::ROTATE;
        }
    }(currentGizmoOperation);

    // ギズモの描画・操作
    if (ImGuizmo::Manipulate(
            viewMatrix, // カメラのビュー行列(float[16])
            projectionMatrix, // カメラのプロジェクション行列(float[16])
            currentGizmoOperation,
            ImGuizmo::LOCAL, // ローカル or ワールド
            matrix)) {

        // matrix から this->translate, this->rotate, this->scale を分解して反映
        // 例: DecomposeMatrixToComponents(matrix, this->translate, this->rotate, this->scale);
        transform->worldMat.fromFloatArray(matrix);

        transform->worldMat.decomposeMatrixToComponents(transform->scale, transform->rotate, transform->translate);
    }

    /// ==========================================
    // Editor Command
    static bool wasUsingGuizmo = false;
    bool isUsingGuizmo         = ImGuizmo::IsUsing();
    static GuiValuePool<Vec3f> vec3fPool;
    static GuiValuePool<Quaternion> quatPool;

    // Guizmo Trigger
    if (isUsingGuizmo) {
        // ImGuizmoが使用中ならば、他の操作は無効化
        ImGui::SetMouseCursor(ImGuiMouseCursor_Arrow);
        if (!wasUsingGuizmo) {
            vec3fPool.setValue(editEntity_->getUniqueID() + "Scale", transform->scale);
            quatPool.setValue(editEntity_->getUniqueID() + "Rotation", transform->rotate);
            vec3fPool.setValue(editEntity_->getUniqueID() + "Translate", transform->translate);
        }
    } else {
        // ImGuizmoが使用されていない場合は、通常のマウスカーソルに戻す
        ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);

        if (wasUsingGuizmo) {
            auto commandCombo = std::make_unique<CommandCombo>();

            /// S,R,T を コマンドで更新するように
            commandCombo->addCommand(std::make_unique<SetterCommand<Vec3f>>(&transform->scale, transform->scale, vec3fPool.popValue(editEntity_->getUniqueID() + "Scale")));
            commandCombo->addCommand(std::make_unique<SetterCommand<Quaternion>>(&transform->rotate, transform->rotate, quatPool.popValue(editEntity_->getUniqueID() + "Rotation")));
            commandCombo->addCommand(std::make_unique<SetterCommand<Vec3f>>(&transform->translate, transform->translate, vec3fPool.popValue(editEntity_->getUniqueID() + "Translate")));

            commandCombo->setFuncOnAfterCommand(
                [&transform]() {
                    transform->Update();
                },
                true);

            // push
            EditorController::getInstance()->pushCommand(std::move(commandCombo));
        }
    }

    wasUsingGuizmo = isUsingGuizmo;
}

void ECSEditor::SortPriorityOrderFromECSManager() {
    const auto& systemsArray = ecsManager_->getSystems();
    int32_t systemTypeIndex  = 0;
    for (const auto& sysMap : systemsArray) {
        workSystemList_[systemTypeIndex].clear();
        for (const auto& [sysName, sysPtr] : sysMap) {
            // システム名とそのシステムに登録されているエンティティのリストを追加
            workSystemList_[int32_t(sysPtr->getSystemType())].push_back(std::make_pair(sysName, sysPtr.get()));
        }

        std::sort(
            workSystemList_[systemTypeIndex].begin(),
            workSystemList_[systemTypeIndex].end(),
            [](const std::pair<std::string, ISystem*>& a,
                const std::pair<std::string, ISystem*>& b) {
                return a.second->getPriority() < b.second->getPriority();
            });

        systemTypeIndex++;
    }
}

void ECSEditor::SortPriorityOrderFromECSManager(int32_t systemTypeIndex) {
    const auto& systemsArray = ecsManager_->getSystems();

    workSystemList_[systemTypeIndex].clear();
    for (const auto& [sysName, sysPtr] : systemsArray[systemTypeIndex]) {
        // システム名とそのシステムに登録されているエンティティのリストを追加
        workSystemList_[systemTypeIndex].push_back(std::make_pair(sysName, sysPtr.get()));
    }

    std::sort(workSystemList_[systemTypeIndex].begin(),
        workSystemList_[systemTypeIndex].end(),
        [](const std::pair<std::string, ISystem*>& a,
            const std::pair<std::string, ISystem*>& b) {
            return a.second->getPriority() < b.second->getPriority();
        });
}

#pragma region "Commands"

void CreateEntityCommand::Execute() {
    ECSManager* ecsManager = ECSManager::getInstance();
    // エンティティ作成処理の抽出
    uint32_t addedEntity = ecsManager->registerEntity("Entity");

    prevEditEntity_ = ecsEditor_->getEditEntity(); // 編集中のエンティティを保存

    ecsEditor_->setEditEntity(ecsManager->getEntity(addedEntity));

    ecsEditor_->customEditComponents().clear();
    for (int32_t i = 0; i < int32_t(SystemType::Count); i++) {
        ecsEditor_->customEditEntitySystems()[i].clear();
    }
}

void CreateEntityCommand::Undo() {
    DestroyEntity(ecsEditor_->getEditEntity());

    // 編集中のエンティティをクリア
    ecsEditor_->setEditEntity(prevEditEntity_);
    // 編集中のコンポーネントをクリア
    ecsEditor_->customEditComponents().clear();
    // 編集中のシステムをクリア
    for (int32_t i = 0; i < int32_t(SystemType::Count); i++) {
        ecsEditor_->customEditEntitySystems()[i].clear();
    }
}

void AddComponentCommand::Execute() {
    ECSManager* ecsManager = ECSManager::getInstance();
    // ECSManagerから対象の ComponentArray を取得してコンポーネント追加
    auto& compMap     = ecsManager->getComponentArrayMap();
    auto compArrayItr = compMap.find(componentTypeName_);

    if (compArrayItr != compMap.end()) {
        addedComponentArray_ = compArrayItr->second.get();

        // 追加したコンポーネントのインデックスを記録
        addedComponentIndex_ = addedComponentArray_->getComponentSize(entity_);
        if (addedComponentArray_->entityCapacity(entity_) <= 0) {
            int32_t newSize = (std::max)(10, addedComponentIndex_ * 2);
            addedComponentArray_->reserveEntity(entity_, newSize);
        }
        // コンポーネントの追加
        addedComponentArray_->addComponent(entity_);

        // 編集用 Component に追加
        if (entityIsEditEntity_) {
            ecsEditor_->customEditComponents()[componentTypeName_].clear();

            for (int32_t compIndex = 0; compIndex < addedComponentIndex_ + 1; ++compIndex) {
                IComponent* component = addedComponentArray_->getComponent(entity_, compIndex);
                if (component) {
                    ecsEditor_->customEditComponents()[componentTypeName_].push_back(component);
                }
            }
        }
    }
}

void AddComponentCommand::Undo() {
    addedComponentArray_->removeBackComponent(entity_);
    if (entityIsEditEntity_) {
        ecsEditor_->customEditComponents()[componentTypeName_].pop_back();
    }
}

void JoinWorkSystemCommand::Execute() {
    // エンティティをシステムに参加させる処理
    system_->addEntity(entity_);
    typeIdx_ = int32_t(system_->getSystemType());
    if (entityIsEditEntity_) {
        ecsEditor_->customEditEntitySystems()[typeIdx_][systemName_] = system_;
    }
}

void JoinWorkSystemCommand::Undo() {
    system_->removeEntity(entity_);
    if (entityIsEditEntity_) {
        ecsEditor_->customEditEntitySystems()[typeIdx_].erase(systemName_);
    }
}

void SelectEntityCommand::Execute() {
    ecsEditor_->setEditEntity(nextEntity_);

    auto& editComponents = ecsEditor_->customEditComponents();
    editComponents.clear();

    if (!nextEntity_) {
        return;
    }

    ECSManager* ecsManager = ECSManager::getInstance();
    // エンティティに紐づくコンポーネントを取得
    for (auto& [componentTypeName, componentArray] : ecsManager->getComponentArrayMap()) {
        int32_t index = 0;
        while (true) {
            IComponent* component = componentArray->getComponent(ecsEditor_->getEditEntity(), index);
            if (component) {
                editComponents[componentTypeName].push_back(component);
                index++;
            } else {
                break;
            }
        }
    }

    // エンティティに紐づくシステムを取得
    auto& editEntitySystems = ecsEditor_->customEditEntitySystems();
    for (int32_t systemTypeIndex = 0; systemTypeIndex < int32_t(SystemType::Count); ++systemTypeIndex) {
        // システムをクリア
        editEntitySystems[systemTypeIndex].clear();
        for (auto& [systemName, system] : ecsManager->getSystemsBy(SystemType(systemTypeIndex))) {
            // システムにエンティティが登録されている場合のみ追加
            if (system->hasEntity(nextEntity_)) {
                editEntitySystems[systemTypeIndex][systemName] = system.get();
            }
        }
    }
}

void SelectEntityCommand::Undo() {
    ecsEditor_->setEditEntity(preEntity_);

    auto& editComponents = ecsEditor_->customEditComponents();
    editComponents.clear();

    if (!preEntity_) {
        return;
    }

    ECSManager* ecsManager = ECSManager::getInstance();
    // エンティティに紐づくコンポーネントを取得
    for (auto& [componentTypeName, componentArray] : ecsManager->getComponentArrayMap()) {
        int32_t index = 0;
        while (true) {
            IComponent* component = componentArray->getComponent(ecsEditor_->getEditEntity(), index);
            if (component) {
                editComponents[componentTypeName].push_back(component);
                index++;
            } else {
                break;
            }
        }
    }

    // エンティティに紐づくシステムを取得
    auto& editEntitySystems = ecsEditor_->customEditEntitySystems();
    for (int32_t systemTypeIndex = 0; systemTypeIndex < int32_t(SystemType::Count); ++systemTypeIndex) {
        // システムをクリア
        editEntitySystems[systemTypeIndex].clear();
        for (auto& [systemName, system] : ecsManager->getSystemsBy(SystemType(systemTypeIndex))) {
            // システムにエンティティが登録されている場合のみ追加
            if (system->hasEntity(nextEntity_)) {
                editEntitySystems[systemTypeIndex][systemName] = system.get();
            }
        }
    }
}

void EraseEntityCommand::Execute() {
    isEditEntity_ = ecsEditor_->getEditEntity() == erasedEntity_;

    // BackUp 用のデータを取得
    erasedEntityBackup_ = EntityToJson(erasedEntity_);

    // エンティティを削除
    DestroyEntity(erasedEntity_);

    if (isEditEntity_) {
        ecsEditor_->setEditEntity(nullptr);
    }
}
void EraseEntityCommand::Undo() {
    erasedEntity_ = EntityFromJson(erasedEntityBackup_);
    if (isEditEntity_) {
        ecsEditor_->setEditEntity(erasedEntity_);
    }
}

void RemoveComponentCommand::Execute() {
    ECSManager* ecsManager = ECSManager::getInstance();
    // ECSManagerから対象の ComponentArray を取得してコンポーネント削除
    auto& compMap     = ecsManager->getComponentArrayMap();
    auto compArrayItr = compMap.find(componentTypeName_);

    if (compArrayItr != compMap.end()) {
        IComponentArray* removedComponentArray_ = compArrayItr->second.get();

        // remove
        removedComponentArray_->removeComponent(entity_, componentIndex_);
        if (entityIsEditEntity_) {
            auto& editCOmponents = ecsEditor_->customEditComponents();
            editCOmponents[componentTypeName_].erase(editCOmponents[componentTypeName_].begin() + componentIndex_);
        }
    }
}

void RemoveComponentCommand::Undo() {
    ECSManager* ecsManager = ECSManager::getInstance();
    // ECSManagerから対象の ComponentArray を取得してコンポーネント追加
    auto& compMap     = ecsManager->getComponentArrayMap();
    auto compArrayItr = compMap.find(componentTypeName_);

    if (compArrayItr != compMap.end()) {
        IComponentArray* removedComponentArray_ = compArrayItr->second.get();
        // 追加
        removedComponentArray_->addComponent(entity_);

        if (entityIsEditEntity_) {
            auto& editCOmponents = ecsEditor_->customEditComponents();
            editCOmponents[componentTypeName_].insert(editCOmponents[componentTypeName_].begin() + componentIndex_, removedComponentArray_->getComponent(entity_, componentIndex_));
        }
    }
}

void LeaveWorkSystemCommand::Execute() {
    // エンティティをシステムから離脱させる処理
    system_->removeEntity(entity_);
    typeIdx_ = int32_t(system_->getSystemType());
    if (entityIsEditEntity_) {
        ecsEditor_->customEditEntitySystems()[typeIdx_].erase(systemName_);
    }
}

void LeaveWorkSystemCommand::Undo() {
    // エンティティをシステムに参加させる処理
    system_->addEntity(entity_);
    if (entityIsEditEntity_) {
        ecsEditor_->customEditEntitySystems()[typeIdx_][systemName_] = system_;
    }
}

void ChangeEntityDataTypeCommand::Execute() {
    // エンティティのデータタイプを変更
    oldDataType_ = entity_->getDataType();
    entity_->setDataType(newDataType_);
}

void ChangeEntityDataTypeCommand::Undo() {
    // エンティティのデータタイプを戻す
    entity_->setDataType(oldDataType_);
}

void ChangingSystemActivityCommand::Execute() {
    // システムのアクティブ状態を変更
    isActive_ = !system_->isActive();

    if (isActive_) {
        ECSManager::getInstance()->ActivateSystem(systemName_, system_->getSystemType(), false);
        for (auto& entity : entities_) {
            if (!entity) {
                continue;
            }
            system_->addEntity(entity);
        }
    } else {
        ECSManager::getInstance()->StopSystem(systemName_, system_->getSystemType());

        entities_.clear();
        for (auto& id : system_->getEntityIDs()) {
            GameEntity* entity = ECSManager::getInstance()->getEntity(id);
            if (!entity) {
                continue;
            }
            system_->removeEntity(entity);
        }
    }
}

void ChangingSystemActivityCommand::Undo() {
    Execute();
}

void ChangingSystemPriorityCommand::Execute() {
    // システムの優先度を変更
    system_->setPriority(newPriority_);

    int32_t systemTypeIndex = int32_t(system_->getSystemType());
    auto& systemByType      = ecsEditor_->customWorkSystemList()[systemTypeIndex];
    // Sort
    std::sort(
        systemByType.begin(),
        systemByType.end(),
        [](const std::pair<std::string, ISystem*>& a,
            const std::pair<std::string, ISystem*>& b) {
            return a.second->getPriority() < b.second->getPriority();
        });

    ECSManager::getInstance()->SortPriorityOrderSystems(systemTypeIndex);
}

void ChangingSystemPriorityCommand::Undo() {
    // システムの優先度を戻す
    system_->setPriority(oldPriority_);

    int32_t systemTypeIndex = int32_t(system_->getSystemType());
    auto& systemByType      = ecsEditor_->customWorkSystemList()[systemTypeIndex];
    // Sort
    std::sort(
        systemByType.begin(),
        systemByType.end(),
        [](const std::pair<std::string, ISystem*>& a,
            const std::pair<std::string, ISystem*>& b) {
            return a.second->getPriority() < b.second->getPriority();
        });

    ECSManager::getInstance()->SortPriorityOrderSystems(systemTypeIndex);
}

void CreateEntityFromFileCommand::Execute() {
    SceneSerializer loader;
    createdEntity = loader.LoadEntity(kApplicationResourceDirectory + "/" + directory_, dataTypeName_);

    if (!selectCommand_) {
        selectCommand_ = std::make_unique<SelectEntityCommand>(ecsEditor_, createdEntity);
    }
    selectCommand_->Execute();
}
void CreateEntityFromFileCommand::Undo() {
    selectCommand_->Undo();
    DestroyEntity(createdEntity);
}

void SaveEntityToFileCommand::Execute() {
    SceneSerializer serializer;
    serializer.SaveEntity(entity_, kApplicationResourceDirectory + "/" + directory_);
}

void SaveEntityToFileCommand::Undo() {
    myfs::deleteFile(kApplicationResourceDirectory + "/" + directory_ + entity_->getDataType() + ".ent");
}
#pragma endregion // Commands
#endif // _DEBUG
