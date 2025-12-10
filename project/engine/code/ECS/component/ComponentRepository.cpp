#include "ComponentRepository.h"

#include "component/ComponentRegistry.h"

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
    auto componentArray = ComponentRegistry::GetInstance()->GetComponentArray(_compTypeName);
    if (componentArray) {
        componentArrays_[_compTypeName] = std::move(ComponentRegistry::GetInstance()->CloneComponentArray(_compTypeName));
        componentArrays_[_compTypeName]->Initialize(1000);
    } else {
        LOG_ERROR("ComponentRepository: ComponentArray not found for type: {}", _compTypeName);
        return false;
    }
    return true;
}

void ComponentRepository::UnRegisterComponentArray(const std::string& _typeName, bool _isFinalize) {
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

void ComponentRepository::AddComponent(const std::string& _compTypeName, Entity* _entity, bool _doInitialize) {
    auto componentArray = GetComponentArray(_compTypeName);
    if (componentArray) {
        componentArray->AddComponent(_entity, _doInitialize);
    } else {
        LOG_ERROR("ComponentRepository: ComponentArray not found for type: {}", _compTypeName);
    }
}

void ComponentRepository::AddComponent(const std::vector<std::string>& _compTypeNames, Entity* _entity, bool _doInitialize) {
    for (const auto& compTypeName : _compTypeNames) {
        AddComponent(compTypeName, _entity, _doInitialize);
    }
}

void ComponentRepository::RemoveComponent(const std::string& _compTypeName, Entity* _entity, int32_t _compIndex) {
    auto componentArray = GetComponentArray(_compTypeName);
    if (componentArray) {
        componentArray->RemoveComponent(_entity, _compIndex);
    } else {
        LOG_ERROR("ComponentRepository: ComponentArray not found for type: {}", _compTypeName);
    }
}

void ComponentRepository::DeleteEntity(Entity* _entity) {
    for (auto& [typeName, componentArray] : componentArrays_) {
        componentArray->DeleteEntity(_entity);
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
