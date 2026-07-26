#include "ComponentRepository.h"

/// ECS
// component
#include "component/ComponentRegistry.h"

/// logger
#include "logger/Logger.h"

using namespace OriGine;

ComponentRepository::ComponentRepository()  = default;
ComponentRepository::~ComponentRepository() = default;

/// <summary>
/// 全てのコンポーネント配列をクリアする.
/// </summary>
void ComponentRepository::Clear() {
    for (auto& [typeName, componentArray] : componentArrays_) {
        componentArray->Finalize();
    }
    componentArrays_.clear();
}

/// <summary>
/// 指定した型名のコンポーネント配列を登録する
/// </summary>
bool ComponentRepository::RegisterComponentArray(const std::string& _compTypeName) {
    if (componentArrays_.find(_compTypeName) != componentArrays_.end()) {
        LOG_WARN("ComponentRepository: ComponentArray already registered for type: {}", _compTypeName);
        return false;
    }

    if (ComponentRegistry::GetInstance()->HasComponentArray(_compTypeName)) {
        // ComponentRegistryに登録済みのファクトリからComponentArrayの実体を複製生成する
        componentArrays_[_compTypeName] = std::move(ComponentRegistry::GetInstance()->CloneComponentArray(_compTypeName));
        componentArrays_[_compTypeName]->Initialize(1000);
    } else {
        LOG_ERROR("ComponentRepository: ComponentArray not found for type: {}", _compTypeName);
        return false;
    }
    return true;
}

/// <summary>
/// 指定した型名のコンポーネント配列を登録解除する
/// </summary>
void ComponentRepository::UnregisterComponentArray(const std::string& _typeName, bool _isFinalize) {
    auto itr = componentArrays_.find(_typeName);
    if (itr != componentArrays_.end()) {
        if (_isFinalize) {
            itr->second->Finalize();
        }
        componentArrays_.erase(itr);
    }
}

/// <summary>
/// 指定した型名のコンポーネント配列を取得する
/// </summary>
IComponentArray* ComponentRepository::GetComponentArray(const std::string& _typeName) {
    auto itr = componentArrays_.find(_typeName);
    if (itr == componentArrays_.end()) {
        // 未登録の場合はここで遅延登録する
        if (RegisterComponentArray(_typeName)) {
            itr = componentArrays_.find(_typeName);
        } else {
            LOG_ERROR("ComponentRepository: ComponentArray not found for type: {}", _typeName);
            return nullptr;
        }
    }
    return itr->second.get();
}

/// <summary>
/// 指定したエンティティにコンポーネントを追加する
/// </summary>
void ComponentRepository::AddComponent(Scene* _scene, const std::string& _compTypeName, const EntityHandle& _handle) {
    auto* componentArray = GetComponentArray(_compTypeName);
    if (componentArray) {
        componentArray->AddComponent(_scene, _handle);
    } else {
        LOG_ERROR("ComponentRepository: ComponentArray not found for type: {}", _compTypeName);
    }
}

/// <summary>
/// 指定したエンティティにコンポーネント群を追加する
/// </summary>
void ComponentRepository::AddComponent(Scene* _scene, const std::vector<std::string>& _compTypeNames, const EntityHandle& _handle) {
    for (const auto& compTypeName : _compTypeNames) {
        AddComponent(_scene, compTypeName, _handle);
    }
}

/// <summary>
/// 指定したエンティティからコンポーネントを削除する
/// </summary>
void ComponentRepository::RemoveComponent(const std::string& _compTypeName, const EntityHandle& _handle, int32_t _compIndex) {
    auto componentArray = GetComponentArray(_compTypeName);
    if (componentArray) {
        componentArray->RemoveComponent(_handle, _compIndex);
    } else {
        LOG_ERROR("ComponentRepository: ComponentArray not found for type: {}", _compTypeName);
    }
}

/// <summary>
/// 指定したエンティティから全てのコンポーネントを削除する。
/// 型ごとの ComponentArray を1つずつ回り、各配列に「このEntityの分だけ消して」と依頼する形を取る。
/// 呼び出し側(Scene::ExecuteDeleteEntities)は、このコンポーネント一括削除 -> システムからの登録解除 ->
/// EntityRepositoryでのEntity実体削除、という順序を守って呼び出す。逆順にすると、
/// システムやコンポーネントが解放済み/無効化済みのエンティティを指したままになってしまう
/// </summary>
void ComponentRepository::RemoveEntity(const EntityHandle& _handle) {
    for (auto& [typeName, componentArray] : componentArrays_) {
        componentArray->RemoveAllComponents(_handle);
    }
}

/// <summary>
/// 指定したエンティティが持つ全てのコンポーネントを取得する
/// </summary>
std::unordered_map<std::string, std::vector<IComponent*>> OriGine::ComponentRepository::GetAllComponentsOfEntity(const EntityHandle& _handle) {
    std::unordered_map<std::string, std::vector<IComponent*>> result;

    for (const auto& [typeName, componentArray] : componentArrays_) {
        if (componentArray->HasEntity(_handle)) {
            auto comps = componentArray->GetIComponents(_handle);
            if (comps.empty()) {
                // 実体を持たない型は結果に含めない
                continue;
            }
            result[typeName] = comps;
        }
    }

    return result;
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
