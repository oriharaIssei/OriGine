#pragma once
#include "ComponentArray.h"

namespace OriGine {

/// <summary>
/// Component Registry
/// ComponentTypeは EXEにおいて一意であり,
/// Sceneで実際に使用される実体 は ComponentRepository に格納される.
/// </summary>
class ComponentRegistry final {
public:
    /// <summary>
    /// シングルトンインスタンスを取得する
    /// </summary>
    static ComponentRegistry* GetInstance();

    /// <summary>
    /// ComponentTypeに対応するComponentArrayの複製関数を登録する
    /// </summary>
    /// <param name="_makeCloneFunc">ComponentArrayを生成するファクトリ関数(省略時はデフォルトのnew)</param>
    template <IsComponent ComponentType>
    void RegisterComponent(
        std::function<std::unique_ptr<IComponentArray>()> _makeCloneFunc =
            []() -> std::unique_ptr<IComponentArray> {
            return std::make_unique<ComponentArray<ComponentType>>();
        });

    /// <summary>
    /// 登録済みのファクトリ関数からComponentArrayを複製生成する(型指定版)
    /// </summary>
    /// <returns>生成されたComponentArray</returns>
    template <IsComponent ComponentType>
    std::unique_ptr<IComponentArray> CloneComponentArray();

    /// <summary>
    /// 登録済みのファクトリ関数からComponentArrayを複製生成する(型名指定版)
    /// </summary>
    /// <param name="_compTypeName">ComponentTypeの型名</param>
    /// <returns>生成されたComponentArray</returns>
    std::unique_ptr<IComponentArray> CloneComponentArray(const std::string& _compTypeName);

private:
    ComponentRegistry();
    ~ComponentRegistry();
    ComponentRegistry(const ComponentRegistry&)            = delete;
    ComponentRegistry& operator=(const ComponentRegistry&) = delete;

private:
    std::unordered_map<std::string, std::function<std::unique_ptr<IComponentArray>()>> cloneMaker_; // 型名 -> ComponentArray生成関数

#ifdef _DEBUG
    std::vector<std::string> componentTypeNames_; // デバッグ用: 登録済みComponent型名の一覧
#endif // _DEBUG

public:
#ifdef _DEBUG
    const std::vector<std::string>& GetComponentTypeNames() const {
        return componentTypeNames_;
    }
#endif // _DEBUG

    bool HasComponentArray(const std::string& _typeName) const {
        return cloneMaker_.find(_typeName) != cloneMaker_.end();
    }
    template <IsComponent ComponentType>
    bool HasComponentArray() const {
        std::string typeName = nameof<ComponentType>();
        return cloneMaker_.find(typeName) != cloneMaker_.end();
    }
};

template <IsComponent ComponentType>
void ComponentRegistry::RegisterComponent(
    std::function<std::unique_ptr<IComponentArray>()> _makeCloneFunc) {
    std::string typeName = nameof<ComponentType>();
    if (cloneMaker_.find(typeName) != cloneMaker_.end()) {
        // 二重登録は上書きされるだけなので警告のみ出す
        LOG_WARN("ComponentRegistry: ComponentArray already registered for type: {}", typeName);
    }
    cloneMaker_[typeName] = _makeCloneFunc;

#ifdef _DEBUG
    componentTypeNames_.push_back(typeName);
#endif // _DEBUG
}

template <IsComponent ComponentType>
std::unique_ptr<IComponentArray> ComponentRegistry::CloneComponentArray() {
    std::string _typeName = nameof<ComponentType>();
    auto itr              = cloneMaker_.find(_typeName);
    if (itr == cloneMaker_.end()) {
        LOG_ERROR("ComponentRegistry: Clone maker not found for type: {}", _typeName);
        return nullptr;
    }
    return itr->second();
}


} // namespace OriGine
