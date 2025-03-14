#pragma once

/// parent
#include "module/editor/IEditor.h"

/// stl
#include <memory>

/// ECS
#include "component/IComponent.h"
#include "ECSManager.h"
#include "Entity.h"
// util
#include "util/nameof.h"

class ECSEditor
    : public IEditor {
public:
    ECSEditor();
    ~ECSEditor();

    void Initialize() override;
    void Update() override;
    void Finalize() override;

private:
    void SelectEntity();
    void EditEntity();
    void WorkerSystemList();

    /// <summary>
    ///  エンティティを WorkSystem に追加するためのポップアップ
    /// </summary>
    /// <param name="_entity"></param>
    void PopupEntityJoinWorkSystem(GameEntity* _entity);
    /// <summary>
    /// エンティティに Component を追加するためのポップアップ
    /// </summary>
    /// <param name="_entity"></param>
    void PopupEntityAddComponent(GameEntity* _entity);

private:
    ECSManager* ecsManager_ = nullptr;

    GameEntity* editEntity_ = nullptr;
    std::vector<std::pair<std::string, IComponent*>> editEntityComponents_;
    std::array<std::vector<std::pair<std::string, ISystem*>>, int32_t(SystemType::Count)> editEntitySystems_;

    std::array<std::vector<std::pair<std::string, ISystem*>>, int32_t(SystemType::Count)> workSystemList_;

    bool isOpenPopupJoinWorkSystem_ = false;
    bool isOpenPopUpAddComponent_   = false;

public:
    void SortPriorityOrderFromECSManager();
    void SortPriorityOrderFromECSManager(int32_t systemTypeIndex);

    GameEntity* getEditEntity() const {
        return editEntity_;
    }
    void setEditEntity(GameEntity* _entity) {
        editEntity_ = _entity;
    }

   std::vector<std::pair<std::string, IComponent*>>& customEditComponents() {
        return editEntityComponents_;
    }
    std::array<std::vector<std::pair<std::string, ISystem*>>, int32_t(SystemType::Count)>& customEditEntitySystems() {
        return editEntitySystems_;
    }
};

#pragma region "Command"
class SelectEntityCommand
    : public IEditCommand {
public:
    SelectEntityCommand(ECSEditor* _ecsEditor, GameEntity* _selectedEntity)
        : ecsEditor_(_ecsEditor), nextEntity_(_selectedEntity) {
        preEntity_ = ecsEditor_->getEditEntity();
    }
    ~SelectEntityCommand() {}

    void Execute() override;
    void Undo() override ;

private:
    ECSEditor* ecsEditor_;

    GameEntity* preEntity_  = nullptr;
    GameEntity* nextEntity_ = nullptr;
};

#pragma endregion
