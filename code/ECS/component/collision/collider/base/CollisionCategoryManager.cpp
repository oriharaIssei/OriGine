#include "CollisionCategoryManager.h"
#include "util/globalVariables/GlobalVariables.h"
#include "util/StringUtil.h"

namespace OriGine {

// GlobalVariablesで使用するキー
static const std::string kSceneName = "Settings";
static const std::string kGroupName = "Collision";
static const std::string kItemName  = "Categories";

CollisionCategoryManager::CollisionCategoryManager() {
    // デフォルトカテゴリを登録
    RegisterCategory("Default");
}

bool CollisionCategoryManager::RegisterCategory(const std::string& _name) {
    // すでに登録されている & 文字列が空 の場合は何もしない
    if (_name.empty() || categories_.find(_name) != categories_.end()) {
        return false;
    }

    // 最大数チェック
    if (categories_.size() >= kMaxCategories) {
        return false;
    }

    indexToName_[categories_.size()] = _name;
    uint32_t newBits                 = 1u << static_cast<uint32_t>(categories_.size());
    categories_.emplace(_name, CollisionCategory(_name, newBits));

    // デフォルトマスクは全カテゴリと衝突
    categoryMasks_[_name] = 0xFFFFFFFF;

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
    categoryMasks_.clear();
    indexToName_.fill("");
    // デフォルトカテゴリを再登録
    RegisterCategory("Default");
}

void CollisionCategoryManager::SetCategoryMask(const std::string& _category, uint32_t _mask) {
    categoryMasks_[_category] = _mask;

    // カテゴリ内のキャッシュも更新
    auto it = categories_.find(_category);
    if (it != categories_.end()) {
        it->second.SetMaskBits(_mask);
    }
}

uint32_t CollisionCategoryManager::GetCategoryMask(const std::string& _category) const {
    auto it = categoryMasks_.find(_category);
    if (it != categoryMasks_.end()) {
        return it->second;
    }
    return 0xFFFFFFFF; // デフォルトは全カテゴリと衝突
}

bool CollisionCategoryManager::CanCollide(const std::string& _categoryA, const std::string& _categoryB) const {
    auto itA = categories_.find(_categoryA);
    auto itB = categories_.find(_categoryB);
    if (itA == categories_.end() || itB == categories_.end()) {
        return true; // 未登録カテゴリは衝突する
    }

    uint32_t maskA = GetCategoryMask(_categoryA);
    uint32_t bitsB = itB->second.GetBits();
    return (maskA & bitsB) != 0;
}

void CollisionCategoryManager::LoadFromGlobalVariables() {
    GlobalVariables* globalVariables = GlobalVariables::GetInstance();

    // シーンが存在するか確認
    auto* scene = globalVariables->GetScene(kSceneName);
    if (!scene) {
        return;
    }

    // グループが存在するか確認
    auto groupItr = scene->find(kGroupName);
    if (groupItr == scene->end()) {
        return;
    }

    // アイテムが存在するか確認
    auto itemItr = groupItr->second.find(kItemName);
    if (itemItr == groupItr->second.end()) {
        return;
    }

    // 文字列として取得（カンマ区切り）
    const std::string* categoriesStr = std::get_if<std::string>(&itemItr->second);
    if (!categoriesStr || categoriesStr->empty()) {
        return;
    }

    // カンマ区切りで分割して登録
    std::vector<std::string> categoryNames = Split(*categoriesStr, ',');
    for (const auto& name : categoryNames) {
        std::string trimmed = Trim(name);
        if (!trimmed.empty()) {
            RegisterCategory(trimmed);
        }
    }

    // マトリクス情報を読み込む（形式: "CategoryName=0xXXXXXXXX"）
    auto masksItr = groupItr->second.find("Masks");
    if (masksItr != groupItr->second.end()) {
        const std::string* masksStr = std::get_if<std::string>(&masksItr->second);
        if (masksStr && !masksStr->empty()) {
            std::vector<std::string> maskEntries = Split(*masksStr, ';');
            for (const auto& entry : maskEntries) {
                std::vector<std::string> parts = Split(entry, '=');
                if (parts.size() == 2) {
                    std::string catName = Trim(parts[0]);
                    std::string maskVal = Trim(parts[1]);
                    if (!catName.empty() && !maskVal.empty()) {
                        uint32_t mask = static_cast<uint32_t>(std::stoul(maskVal, nullptr, 16));
                        // SetCategoryMaskを使用してcategories_内のmaskBits_も更新
                        SetCategoryMask(catName, mask);
                    }
                }
            }
        }
    }
}

void CollisionCategoryManager::SaveToGlobalVariables() {
    GlobalVariables* globalVariables = GlobalVariables::GetInstance();

    // カテゴリ名をカンマ区切りで連結
    std::string categoriesStr;
    bool first = true;
    for (const auto& [name, category] : categories_) {
        if (!first) {
            categoriesStr += ",";
        }
        categoriesStr += name;
        first = false;
    }

    globalVariables->SetValue<std::string>(kSceneName, kGroupName, kItemName, categoriesStr);

    // マトリクス情報を保存（形式: "CategoryName=0xXXXXXXXX;..."）
    std::string masksStr;
    first = true;
    for (const auto& [name, mask] : categoryMasks_) {
        if (!first) {
            masksStr += ";";
        }
        char hexBuf[16];
        snprintf(hexBuf, sizeof(hexBuf), "0x%08X", mask);
        masksStr += name + "=" + hexBuf;
        first = false;
    }

    globalVariables->SetValue<std::string>(kSceneName, kGroupName, "Masks", masksStr);
    globalVariables->SaveFile(kSceneName, kGroupName);
}

} // namespace OriGine
