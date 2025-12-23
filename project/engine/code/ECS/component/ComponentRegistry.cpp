#include "ComponentRegistry.h"

using namespace OriGine;

ComponentRegistry* ComponentRegistry::GetInstance() {
    static ComponentRegistry instance;
    return &instance;
}

ComponentRegistry::ComponentRegistry()  = default;
ComponentRegistry::~ComponentRegistry() = default;

std::unique_ptr<IComponentArray> ComponentRegistry::CloneComponentArray(const std::string& _compTypeName) {
    auto itr = cloneMaker_.find(_compTypeName);
    if (itr == cloneMaker_.end()) {
        LOG_ERROR("ComponentRegistry: Clone maker not found for type: {}", _compTypeName);
        return nullptr;
    }
    return itr->second();
}
