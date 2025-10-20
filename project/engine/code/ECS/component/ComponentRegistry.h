#pragma once
#include "ComponentArray.h"

/// <summary>
/// Component Registry
/// ComponentTypeは EXEにおいて一意であり,
/// Sceneで実際に使用される実体 は ComponentRepository に格納される.
/// </summary>
class ComponentRegistry final {
public:
    static ComponentRegistry* getInstance();

    template <IsComponent ComponentType>
    void registerComponent(
        std::function<std::unique_ptr<IComponentArray>()> _makeCloneFunc =
            []() -> std::unique_ptr<IComponentArray> {
            return std::make_unique<ComponentArray<ComponentType>>();
        });

    template <IsComponent ComponentType>
    ComponentArray<ComponentType>* getComponentArray() const;

    IComponentArray* getComponentArray(const std::string& _typeName) const;

    template <IsComponent ComponentType>
    std::unique_ptr<IComponentArray> cloneComponentArray();

    std::unique_ptr<IComponentArray> cloneComponentArray(const std::string& _compTypeName);

private:
    ComponentRegistry();
    ~ComponentRegistry();
    ComponentRegistry(const ComponentRegistry&)            = delete;
    ComponentRegistry& operator=(const ComponentRegistry&) = delete;

private:
    std::unordered_map<std::string, std::unique_ptr<IComponentArray>> componentArrays_;
    std::unordered_map<std::string, std::function<std::unique_ptr<IComponentArray>()>> cloneMaker_;

public:
    const std::unordered_map<std::string, std::unique_ptr<IComponentArray>>& getComponentArrayMap() const;
    std::unordered_map<std::string, std::unique_ptr<IComponentArray>>& getComponentArrayMapRef();
};

template <IsComponent ComponentType>
void ComponentRegistry::registerComponent(
    std::function<std::unique_ptr<IComponentArray>()> _makeCloneFunc) {
    std::string typeName = nameof<ComponentType>();
    if (componentArrays_.find(typeName) != componentArrays_.end()) {
        LOG_WARN("ComponentRegistry: ComponentArray already registered for type: {}", typeName);
    }

    componentArrays_[typeName] = std::make_unique<ComponentArray<ComponentType>>();
    componentArrays_[typeName]->Initialize();
    cloneMaker_[typeName] = _makeCloneFunc;
}

template <IsComponent ComponentType>
ComponentArray<ComponentType>* ComponentRegistry::getComponentArray() const {
    std::string typeName = nameof<ComponentType>();
    auto itr             = componentArrays_.find(typeName);
    if (itr == componentArrays_.end()) {
        LOG_ERROR("ComponentRegistry: ComponentArray not found for type: {}", typeName);
        return nullptr;
    }
    return dynamic_cast<ComponentArray<ComponentType>*>(itr->second.get());
}

template <IsComponent ComponentType>
std::unique_ptr<IComponentArray> ComponentRegistry::cloneComponentArray() {
    std::string _typeName = nameof<ComponentType>();
    auto itr              = cloneMaker_.find(_typeName);
    if (itr == cloneMaker_.end()) {
        LOG_ERROR("ComponentRegistry: Clone maker not found for type: {}", _typeName);
        return nullptr;
    }
    return itr->second();
}
