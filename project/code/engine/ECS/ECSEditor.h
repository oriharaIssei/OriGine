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

    void Init();
    void Update() override;

private:
    void SelectEntity();
    void EditComponent();
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
    std::vector<IComponent*> editEntityComponents_;

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

    std::vector<IComponent*> getEditComponents() const {
        return editEntityComponents_;
    }
    void setEditComponents(const std::vector<IComponent*>& _components) {
        editEntityComponents_ = _components;
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

    void Execute() override {
        ecsEditor_->setEditEntity(nextEntity_);
    }
    void Undo() override {
        ecsEditor_->setEditEntity(preEntity_);
    }

private:
    ECSEditor* ecsEditor_;

    GameEntity* preEntity_  = nullptr;
    GameEntity* nextEntity_ = nullptr;
};

#pragma endregion
