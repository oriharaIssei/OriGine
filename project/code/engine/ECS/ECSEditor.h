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

#pragma region EditorCommands
/// <summary>
/// コンポーネントを追加するコマンド
/// </summary>
/// <typeparam name="ComponentType"></typeparam>
template <IsComponent ComponentType>
class AddComponentForDataTypeCommand
    : public IEditCommand {
public:
    AddComponentForDataTypeCommand(const std::string& _entityDataType)
        : entityDataType_(_entityDataType), componentType_(nameof<ComponentType>()) {}
    ~AddComponentForDataTypeCommand() {}

    void Execute() override {
        ECSManager::getInstance()->registerComponentTypeByDataType<ComponentType>(entityDataType_);
    }
    void Undo() override {
        ECSManager::getInstance()->removeComponentTypeByDataType(entityDataType_, componentType_);
    }

private:
    std::string entityDataType_;
    std::string componentType_;
    int32_t componentSize_;
};

template <IsSystem SystemType>
class AddSystemForDataTypeCommand
    : public IEditCommand {
public:
    AddSystemForDataTypeCommand(const std::string& dataType, const SystemType& system)
        : dataType_(dataType), system_(system) {}
    ~AddSystemForDataTypeCommand() {}

    void Execute() override {
        // システムの追加処理
        ECSManager::getInstance()->registerSystemByDataType(dataType_, system_);
    }
    void Undo() override {
        // システムの削除処理
        ECSManager::getInstance()->removeSystem(dataType_, system_);
    }

private:
    std::string dataType_;
    SystemType system_;
};

/// <summary>
/// コンポーネントを削除するコマンド
/// </summary>
template <IsComponent ComponentType>
class RemoveComponentCommand
    : public IEditCommand {
public:
    RemoveComponentCommand(GameEntity* entity, const ComponentType& component)
        : pEntity_(entity), component_(component) {}
    ~RemoveComponentCommand() {}

    void Execute() override {
        // コンポーネントの削除処理
        ECSManager::getInstance()->removeComponent(pEntity_, component_);
    }
    void Undo() override {
        // コンポーネントの追加処理
        ECSManager::getInstance()->addComponent(pEntity_, component_);
    }

private:
    GameEntity* pEntity_;
    ComponentType component_;
};

/// <summary>
/// エンティティのデータタイプを作成するコマンド
/// </summary>
class CreateEntityDataTypeCommand
    : public IEditCommand {
public:
    CreateEntityDataTypeCommand(const std::string& dataType)
        : dataType_(dataType) {}
    ~CreateEntityDataTypeCommand() {}

    void Execute() override {
        // データタイプの作成処理
        ECSManager::getInstance()->registerNewEntityDataType(dataType_);
    }
    void Undo() override {
        // データタイプの削除処理
        ECSManager::getInstance()->removeEntityDataType(dataType_);
    }

private:
    std::string dataType_;
};

/// <summary>
/// エンティティのデータタイプを削除するコマンド
/// </summary>
class DeleteEntityDataTypeCommand
    : public IEditCommand {
public:
    DeleteEntityDataTypeCommand(const std::string& dataType)
        : dataType_(dataType) {}
    ~DeleteEntityDataTypeCommand() {}

    void Execute() override {
        // 復元できるようにデータを保持
        componentsList_ = ECSManager::getInstance()->getComponentTypeByDataType(dataType_);
        systemList_     = ECSManager::getInstance()->getSystemsByDataType(dataType_);
        // データタイプの削除処理
        ECSManager::getInstance()->removeEntityDataType(dataType_);
    }
    void Undo() override {
        // データタイプの作成処理
        ECSManager::getInstance()->registerNewEntityDataType(dataType_);
    }

private:
    std::vector<std::pair<std::string, int32_t>> componentsList_;
    std::vector<std::string> systemList_;
    std::string dataType_;
};

template <IsComponent ComponentType>
class EditComponentCommand
    : public IEditCommand {
public:
    EditComponentCommand(GameEntity* _entity, ComponentType* _pEditComponent, const ComponentType& _editedData)
        : entity_(_entity), pEditComponent_(_pEditComponent), componentData_(_editedData) {}
    ~EditComponentCommand() {}

    void Execute() override {
        // コンポーネントの編集処理
        *pEditComponent_ = componentData_;
    }
    void Undo() override {
        // コンポーネントの編集処理
        *pEditComponent_ = componentData_;
    }

private:
    GameEntity* entity_            = nullptr;
    ComponentType* pEditComponent_ = nullptr;
    ComponentType componentData_;
};

#pragma endregion

class ECSEditor
    : public IEditor {
public:
    ECSEditor();
    ~ECSEditor();

    void Update() override;

private:
};
