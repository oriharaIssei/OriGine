#include "CollisionCategoryManager.h"

namespace OriGine {

void CollisionCategoryManager::ForEachCategoryInMask(uint32_t _mask, auto&& _callback) const {
    while (_mask != 0) {
        uint32_t index = std::countr_zero(_mask); // C++20: 最下位ビット位置
        _callback(indexToName_[index]);
        _mask &= (_mask - 1); // 最下位ビットをクリア
    }
}

CollisionCategoryManager::CollisionCategoryManager() {
    // デフォルトカテゴリを登録
    RegisterCategory("Default");
}

bool CollisionCategoryManager::RegisterCategory(const std::string& _name) {
    // すでに登録されている場合は何もしない
    if (categories_.find(_name) != categories_.end()) {
        return false;
    }

    // 最大数チェック
    if (categories_.size() >= kMaxCategories) {
        return false;
    }

    indexToName_[categories_.size()] = _name;
    uint32_t newBits                 = 1u << static_cast<uint32_t>(categories_.size());
    categories_.emplace(_name, CollisionCategory(_name, newBits));
    return true;
}

const CollisionCategory& CollisionCategoryManager::GetCategory(const std::string& _name) const {
    auto it = categories_.find(_name);
    if (it != categories_.end()) {
        return it->second;
    }
    return defaultCategory_;
}

const CollisionCategory& CollisionCategoryManager::GetOrRegisterCategory(const std::string& _name) {
    auto it = categories_.find(_name);
    if (it != categories_.end()) {
        return it->second;
    }

    // 登録されていなければ登録を試みる
    if (RegisterCategory(_name)) {
        return categories_[_name];
    }

    // 登録失敗時はデフォルトを返す
    return defaultCategory_;
}

bool CollisionCategoryManager::HasCategory(const std::string& _name) const {
    return categories_.find(_name) != categories_.end();
}

void CollisionCategoryManager::Clear() {
    categories_.clear();
    indexToName_.fill("");
    // デフォルトカテゴリを再登録
    RegisterCategory("Default");
}

} // namespace OriGine
