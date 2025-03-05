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

    void Update() override;

private:
    void SelectEntity();
    void EditComponent();

private:
    GameEntity* editEntity_ = nullptr;
    std::vector<IComponent*> editComponents_;

    bool isOpenPopUpAddComponent_ = false;

public:
    GameEntity* getEditEntity() const {
        return editEntity_;
    }
    void setEditEntity(GameEntity* _entity) {
        editEntity_ = _entity;
    }

    std::vector<IComponent*> getEditComponents() const {
        return editComponents_;
    }
    void setEditComponents(const std::vector<IComponent*>& _components) {
        editComponents_ = _components;
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
