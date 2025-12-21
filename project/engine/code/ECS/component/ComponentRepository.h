#pragma once

#include "ComponentArray.h"
#include "ComponentRegistry.h"

namespace OriGine {

/// <summary>
/// Component Repository
/// ComponentRepositoryは実際にシーンで使用されるコンポーネントの実体を保持する.
/// </summary>
class ComponentRepository final {
public:
    ComponentRepository();
    ~ComponentRepository();

    /// <summary>
    /// 全てのコンポーネント配列をクリアする.
    /// </summary>
    void Clear();

    /// <summary>
    /// 指定した型のコンポーネント配列を登録する
    /// </summary>
    /// <typeparam name="ComponentType">コンポーネントの型</typeparam>
    /// <returns>登録ができた ＝ true ,できなかった = false</returns>
    template <IsComponent ComponentType>
    bool RegisterComponentArray();

    /// <summary>
    /// 指定した型名のコンポーネント配列を登録する
    /// </summary>
    /// <param name="_compTypeName">コンポーネントの型名</param>
    /// <returns>登録ができた ＝ true ,できなかった = false</returns>
    bool RegisterComponentArray(const std::string& _compTypeName);
    /// <summary>
    /// 指定した型名のコンポーネント配列を登録解除する
    /// </summary>
    /// <param name="_typeName">コンポーネントの型名</param>
    /// <param name="_isFinalize">Finalizeを呼び出すかどうか</param>
    void UnregisterComponentArray(const std::string& _typeName, bool _isFinalize = true);

    /// <summary>
    /// 指定した型のコンポーネント配列を取得する
    /// </summary>
    /// <typeparam name="ComponentType">コンポーネントの型</typeparam>
    /// <returns>コンポーネント配列</returns>
    template <IsComponent ComponentType>
    ComponentArray<ComponentType>* GetComponentArray();
    /// <summary>
    /// 指定した型名のコンポーネント配列を取得する
    /// </summary>
    /// <param name="_typeName">コンポーネントの型名</param>
    IComponentArray* GetComponentArray(const std::string& _typeName);

    /// <summary>
    /// 指定したエンティティが持つ指定した型のコンポーネント群を取得する
    /// </summary>
    /// <typeparam name="ComponentType">コンポーネントの型</typeparam>
    /// <param name="_entity">コンポーネントを持つエンティティ</param>
    /// <returns> _entityが持つコンポーネント郡 </returns>
    template <IsComponent ComponentType>
    std::vector<ComponentType>* GetComponents(EntityHandle _handle);

    /// <summary>
    /// 指定したエンティティが持つ指定した型のコンポ
    /// </summary>
    /// <typeparam name="ComponentType">コンポーネントの型</typeparam>
    /// <param name="_entity">コンポーネントを持つエンティティ</param>
    /// <param name="_index">コンポーネントのインデックス</param>
    /// <returns> _entityが持つコンポーネント </returns>
    template <IsComponent ComponentType>
    ComponentType* GetComponent(EntityHandle _handle, uint32_t _index = 0);

    /// <summary>
    /// 指定したエンティティにコンポーネントを追加する
    /// </summary>
    /// <typeparam name="ComponentType">コンポーネントの型</typeparam>
    /// <param name="_entity">コンポーネントを追加するエンティティ</param>
    /// <param name="_doInitialize">追加したコンポーネントのInitializeを呼び出すかどうか</param>
    template <IsComponent... ComponentType>
    void AddComponent(Scene* _scene, EntityHandle _handle);
    /// <summary>
    /// 指定したエンティティにコンポーネントを追加する
    /// </summary>
    /// <param name="_compTypeName">コンポーネントの型名</param>
    /// <param name="_entity">コンポーネントを追加するエンティティ</param>
    /// <param name="_doInitialize">追加したコンポーネントのInitializeを呼び出すかどうか</param>
    void AddComponent(Scene* _scene, const std::string& _compTypeName, EntityHandle _handle);
    /// <summary>
    /// 指定したエンティティにコンポーネント群を追加する
    /// </summary>
    /// <param name="_compTypeNames">コンポーネントの型名群</param>
    /// <param name="_entity">コンポーネントを追加するエンティティ</param>
    /// <param name="_doInitialize">追加したコンポーネントのInitializeを呼び出すかどうか</param>
    void AddComponent(Scene* _scene, const std::vector<std::string>& _compTypeNames, EntityHandle _handle, bool _doInitialize = true);

    /// <summary>
    /// 指定したエンティティからコンポーネントを削除する
    /// </summary>
    /// <param name="_compTypeName">削除するコンポーネントの型名</param>
    /// <param name="_entity">コンポーネントを削除されるエンティティ</param>
    /// <param name="_compIndex">削除するコンポーネントのインデックス</param>
    void RemoveComponent(const std::string& _compTypeName, EntityHandle _handle, int32_t _compIndex = 0);

    /// <summary>
    /// 指定したエンティティからコンポーネント群を削除
    /// </summary>
    /// <param name="_compTypeNames">削除するコンポーネントの型名群</param>
    /// <param name="_entity">コンポーネントを削除されるエンティティ</param>
    /// <param name="_doFinalize">終了処理を呼び出すかどうか</param>
    template <IsComponent ComponentType>
    void RemoveComponent(EntityHandle _handle, bool _doFinalize = true);

    /// <summary>
    /// 指定したエンティティから全てのコンポーネントを削除する
    /// </summary>
    /// <param name="_entity">コンポーネントを削除されるエンティティ</param>
    void DeleteEntity(EntityHandle _handle);

private:
    /// <summary>
    /// コンポーネントの型名をキーに持つコンポーネント配列のマップ
    /// </summary>
    std::unordered_map<std::string, std::unique_ptr<IComponentArray>> componentArrays_;

public:
    uint32_t GetComponentCount() const;
    const std::unordered_map<std::string, std::unique_ptr<IComponentArray>>& GetComponentArrayMap() const;
    std::unordered_map<std::string, std::unique_ptr<IComponentArray>>& GetComponentArrayMapRef();
};

template <IsComponent ComponentType>
inline bool ComponentRepository::RegisterComponentArray() {
    std::string typeName = nameof<ComponentType>();
    if (componentArrays_.find(typeName) != componentArrays_.end()) {
        LOG_WARN("ComponentRepository: ComponentArray already registered for type: {}", typeName);
        return false;
    }
    auto componentArray = ComponentRegistry::GetInstance()->GetComponentArray(typeName);
    if (componentArray) {
        componentArrays_[typeName] = std::move(ComponentRegistry::GetInstance()->CloneComponentArray<ComponentType>());
        componentArrays_[typeName]->Initialize(1000);
    } else {
        LOG_ERROR("ComponentRepository: ComponentArray not found for type: {}", typeName);
        return false;
    }
    return true;
}

template <IsComponent ComponentType>
inline ComponentArray<ComponentType>* ComponentRepository::GetComponentArray() {
    std::string typeName = nameof<ComponentType>();
    auto itr             = componentArrays_.find(typeName);
    if (itr == componentArrays_.end()) {
        if (RegisterComponentArray<ComponentType>()) {
            itr = componentArrays_.find(typeName);
        } else {
            LOG_ERROR("ComponentRepository: ComponentArray not found for type: {}", typeName);
            return nullptr;
        }
    }
    return reinterpret_cast<ComponentArray<ComponentType>*>(itr->second.get());
}

template <IsComponent ComponentType>
inline std::vector<ComponentType>* ComponentRepository::GetComponents(EntityHandle _handle) {
    auto componentArray = GetComponentArray<ComponentType>();
    if (componentArray == nullptr) {
        return nullptr;
    }
    return componentArray->GetComponents(_entity);
}

template <IsComponent ComponentType>
inline ComponentType* ComponentRepository::GetComponent(EntityHandle _handle, uint32_t _index) {
    auto componentArray = GetComponentArray<ComponentType>();
    if (componentArray == nullptr) {
        return nullptr;
    }
    return componentArray->GetDynamicComponent(_entity, _index);
}

template <IsComponent... ComponentType>
inline void ComponentRepository::AddComponent(Scene* _scene, EntityHandle _handle) {
    (this->GetComponentArray<ComponentType>()->AddComponent(_scene,_handle), ...);
}

template <IsComponent ComponentType>
inline void ComponentRepository::RemoveComponent(EntityHandle _handle, bool _doFinalize) {
    auto componentArray = GetComponentArray<ComponentType>();
    if (componentArray) {
        componentArray->RemoveComponent(_entity);
        if (_doFinalize) {
            componentArray->clearComponent(_entity);
        }
    } else {
        LOG_ERROR("ComponentRepository: ComponentArray not found for type: {}", nameof<ComponentType>());
    }
}

} // namespace OriGine
