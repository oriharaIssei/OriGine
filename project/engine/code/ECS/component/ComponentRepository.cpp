#include "ComponentRepository.h"

#include "component/ComponentRegistry.h"

ComponentRepository::ComponentRepository()  = default;
ComponentRepository::~ComponentRepository() = default;

void ComponentRepository::clear() {
    for (auto& [typeName, componentArray] : componentArrays_) {
        componentArray->Finalize();
    }
    componentArrays_.clear();
}

bool ComponentRepository::registerComponentArray(const std::string& _compTypeName) {
    if (componentArrays_.find(_compTypeName) != componentArrays_.end()) {
        LOG_WARN("ComponentRepository: ComponentArray already registered for type: {}", _compTypeName);
        return false;
    }
    auto componentArray = ComponentRegistry::getInstance()->getComponentArray(_compTypeName);
    if (componentArray) {
        componentArrays_[_compTypeName] = std::move(ComponentRegistry::getInstance()->cloneComponentArray(_compTypeName));
        componentArrays_[_compTypeName]->Initialize(1000);
    } else {
        LOG_ERROR("ComponentRepository: ComponentArray not found for type: {}", _compTypeName);
        return false;
    }
    return true;
}

void ComponentRepository::unregisterComponentArray(const std::string& _typeName, bool _isFinalize) {
    auto itr = componentArrays_.find(_typeName);
    if (itr != componentArrays_.end()) {
        if (_isFinalize) {
            itr->second->Finalize();
        }
        componentArrays_.erase(itr);
    }
}

IComponentArray* ComponentRepository::getComponentArray(const std::string& _typeName) {
    auto itr = componentArrays_.find(_typeName);
    if (itr == componentArrays_.end()) {
        if (registerComponentArray(_typeName)) {
            itr = componentArrays_.find(_typeName);
        } else {
            LOG_ERROR("ComponentRepository: ComponentArray not found for type: {}", _typeName);
            return nullptr;
        }
    }
    return itr->second.get();
}

void ComponentRepository::addComponent(const std::string& _compTypeName, Entity* _entity, bool _doInitialize) {
    auto componentArray = getComponentArray(_compTypeName);
    if (componentArray) {
        componentArray->addComponent(_entity, _doInitialize);
    } else {
        LOG_ERROR("ComponentRepository: ComponentArray not found for type: {}", _compTypeName);
    }
}

void ComponentRepository::addComponent(const std::vector<std::string>& _compTypeNames, Entity* _entity, bool _doInitialize) {
    for (const auto& compTypeName : _compTypeNames) {
        addComponent(compTypeName, _entity, _doInitialize);
    }
}

void ComponentRepository::removeComponent(const std::string& _compTypeName, Entity* _entity, int32_t _compIndex) {
    auto componentArray = getComponentArray(_compTypeName);
    if (componentArray) {
        componentArray->removeComponent(_entity, _compIndex);
    } else {
        LOG_ERROR("ComponentRepository: ComponentArray not found for type: {}", _compTypeName);
    }
}

void ComponentRepository::deleteEntity(Entity* _entity) {
    for (auto& [typeName, componentArray] : componentArrays_) {
        componentArray->deleteEntity(_entity);
    }
}

uint32_t ComponentRepository::getComponentCount() const {
    return static_cast<uint32_t>(componentArrays_.size());
}

const std::unordered_map<std::string, std::unique_ptr<IComponentArray>>& ComponentRepository::getComponentArrayMap() const {
    return componentArrays_;
}

std::unordered_map<std::string, std::unique_ptr<IComponentArray>>& ComponentRepository::getComponentArrayMapRef() {
    return componentArrays_;
}
