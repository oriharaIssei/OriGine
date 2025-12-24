#include "ComponentRepository.h"

/// ECS
// component
#include "component/ComponentRegistry.h"

/// logger
#include "logger/Logger.h"

using namespace OriGine;

ComponentRepository::ComponentRepository()  = default;
ComponentRepository::~ComponentRepository() = default;

void ComponentRepository::Clear() {
    for (auto& [typeName, componentArray] : componentArrays_) {
        componentArray->Finalize();
    }
    componentArrays_.clear();
}

bool ComponentRepository::RegisterComponentArray(const std::string& _compTypeName) {
    if (componentArrays_.find(_compTypeName) != componentArrays_.end()) {
        LOG_WARN("ComponentRepository: ComponentArray already registered for type: {}", _compTypeName);
        return false;
    }

    if (ComponentRegistry::GetInstance()->HasComponentArray(_compTypeName)) {
        componentArrays_[_compTypeName] = std::move(ComponentRegistry::GetInstance()->CloneComponentArray(_compTypeName));
        componentArrays_[_compTypeName]->Initialize(1000);
    } else {
        LOG_ERROR("ComponentRepository: ComponentArray not found for type: {}", _compTypeName);
        return false;
    }
    return true;
}

void ComponentRepository::UnregisterComponentArray(const std::string& _typeName, bool _isFinalize) {
    auto itr = componentArrays_.find(_typeName);
    if (itr != componentArrays_.end()) {
        if (_isFinalize) {
            itr->second->Finalize();
        }
        componentArrays_.erase(itr);
    }
}

IComponentArray* ComponentRepository::GetComponentArray(const std::string& _typeName) {
    auto itr = componentArrays_.find(_typeName);
    if (itr == componentArrays_.end()) {
        if (RegisterComponentArray(_typeName)) {
            itr = componentArrays_.find(_typeName);
        } else {
            LOG_ERROR("ComponentRepository: ComponentArray not found for type: {}", _typeName);
            return nullptr;
        }
    }
    return itr->second.get();
}

void ComponentRepository::AddComponent(Scene* _scene, const std::string& _compTypeName, EntityHandle _handle) {
    auto* componentArray = GetComponentArray(_compTypeName);
    if (componentArray) {
        componentArray->AddComponent(_scene, _handle);
    } else {
        LOG_ERROR("ComponentRepository: ComponentArray not found for type: {}", _compTypeName);
    }
}

void ComponentRepository::AddComponent(Scene* _scene, const std::vector<std::string>& _compTypeNames, EntityHandle _handle) {
    for (const auto& compTypeName : _compTypeNames) {
        AddComponent(_scene, compTypeName, _handle);
    }
}

void ComponentRepository::RemoveComponent(const std::string& _compTypeName, EntityHandle _handle, int32_t _compIndex) {
    auto componentArray = GetComponentArray(_compTypeName);
    if (componentArray) {
        componentArray->RemoveComponent(_handle, _compIndex);
    } else {
        LOG_ERROR("ComponentRepository: ComponentArray not found for type: {}", _compTypeName);
    }
}

void ComponentRepository::RemoveEntity(EntityHandle _handle) {
    for (auto& [typeName, componentArray] : componentArrays_) {
        componentArray->RemoveAllComponents(_handle);
    }
}

uint32_t ComponentRepository::GetComponentCount() const {
    return static_cast<uint32_t>(componentArrays_.size());
}

const std::unordered_map<std::string, std::unique_ptr<IComponentArray>>& ComponentRepository::GetComponentArrayMap() const {
    return componentArrays_;
}

std::unordered_map<std::string, std::unique_ptr<IComponentArray>>& ComponentRepository::GetComponentArrayMapRef() {
    return componentArrays_;
}
