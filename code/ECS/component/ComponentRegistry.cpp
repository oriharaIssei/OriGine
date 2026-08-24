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

/// <summary>
/// 型名に対応する型IDを返す。未採番なら新たに採番する。
/// 採番は 0 始まりの連番で、欠番を作らない(Phase 5 がこの値をビット位置として使うため)。
/// </summary>
uint32_t ComponentRegistry::AcquireTypeId(const std::string& _typeName) {
    auto itr = typeNameToId_.find(_typeName);
    if (itr != typeNameToId_.end()) {
        return itr->second;
    }

    const uint32_t newId = static_cast<uint32_t>(idToTypeName_.size());
    if (newId >= kMaxComponentTypes) {
        // 上限を超えると ComponentRepository の固定長配列に収まらない。
        // 黙って壊れるより、ここで止めて型数を減らすか上限を引き上げるかを判断させる。
        LOG_ERROR("ComponentRegistry: component type count exceeded kMaxComponentTypes({}). type: {}",
            kMaxComponentTypes, _typeName);
        return kInvalidComponentTypeId;
    }

    typeNameToId_[_typeName] = newId;
    idToTypeName_.push_back(_typeName);
    return newId;
}

/// <summary>
/// 型名に対応する型IDを返す。未採番でも採番しない。
/// </summary>
uint32_t ComponentRegistry::FindTypeId(const std::string& _typeName) const {
    auto itr = typeNameToId_.find(_typeName);
    return itr != typeNameToId_.end() ? itr->second : kInvalidComponentTypeId;
}

/// <summary>
/// 型IDから型名を逆引きする。
/// </summary>
const std::string& ComponentRegistry::GetTypeName(uint32_t _typeId) const {
    // 呼び出し側が参照を保持できるよう、範囲外でも空の実体を返す(nullptr を返さない)
    static const std::string kEmptyTypeName;
    if (_typeId >= idToTypeName_.size()) {
        return kEmptyTypeName;
    }
    return idToTypeName_[_typeId];
}

