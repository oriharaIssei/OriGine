#include "ComponentRegistry.h"

ComponentRegistry* ComponentRegistry::getInstance() {
    static ComponentRegistry instance;
    return &instance;
}

ComponentRegistry::ComponentRegistry()  = default;
ComponentRegistry::~ComponentRegistry() = default;

IComponentArray* ComponentRegistry::getComponentArray(const std::string& _typeName) const {
    auto itr = componentArrays_.find(_typeName);
    if (itr == componentArrays_.end()) {
        LOG_ERROR("ComponentRegistry: ComponentArray not found for type: {}", _typeName);
        return nullptr;
    }
    return itr->second.get();
}

std::unique_ptr<IComponentArray> ComponentRegistry::cloneComponentArray(const std::string& _compTypeName) {
    auto itr = cloneMaker_.find(_compTypeName);
    if (itr == cloneMaker_.end()) {
        LOG_ERROR("ComponentRegistry: Clone maker not found for type: {}", _compTypeName);
        return nullptr;
    }
    return itr->second();
}

const std::unordered_map<std::string, std::unique_ptr<IComponentArray>>&
ComponentRegistry::getComponentArrayMap() const {
    return componentArrays_;
}

std::unordered_map<std::string, std::unique_ptr<IComponentArray>>&
ComponentRegistry::getComponentArrayMapRef() {
    return componentArrays_;
}
