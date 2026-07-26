#include "ComponentRegistry.h"

using namespace OriGine;

/// <summary>
/// シングルトンインスタンスを取得する
/// </summary>
ComponentRegistry* ComponentRegistry::GetInstance() {
    static ComponentRegistry instance;
    return &instance;
}

ComponentRegistry::ComponentRegistry()  = default;
ComponentRegistry::~ComponentRegistry() = default;

/// <summary>
/// 登録済みのファクトリ関数からComponentArrayを複製生成する(型名指定版)
/// </summary>
std::unique_ptr<IComponentArray> ComponentRegistry::CloneComponentArray(const std::string& _compTypeName) {
    auto itr = cloneMaker_.find(_compTypeName);
    if (itr == cloneMaker_.end()) {
        LOG_ERROR("ComponentRegistry: Clone maker not found for type: {}", _compTypeName);
        return nullptr;
    }
    return itr->second();
}
