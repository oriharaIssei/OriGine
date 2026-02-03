#pragma once

/// stl
#include <array>
#include <string>
#include <unordered_map>
#include <vector>

/// engine
#include "CollisionCategory.h"

namespace OriGine {

/// <summary>
/// 衝突カテゴリ管理クラス（シングルトン）
/// string→bit変換、カテゴリ登録/取得を担当
/// </summary>
class CollisionCategoryManager {
public:
    static CollisionCategoryManager* GetInstance() {
        static CollisionCategoryManager instance;
        return &instance;
    }

private:
    static constexpr uint32_t kMaxCategories = 32;

public:
    /// <summary>
    /// カテゴリを登録する（最大32個まで）
    /// </summary>
    /// <param name="name">カテゴリ名</param>
    /// <returns>登録成功したらtrue</returns>
    bool RegisterCategory(const std::string& _name);

    /// <summary>
    /// カテゴリを取得する
    /// </summary>
    /// <param name="name">カテゴリ名</param>
    /// <returns>カテゴリ情報（見つからない場合はデフォルト）</returns>
    const CollisionCategory& GetCategory(const std::string& _name) const;

    /// <summary>
    /// カテゴリを取得する（存在しなければ登録）
    /// </summary>
    const CollisionCategory& GetOrRegisterCategory(const std::string& _name);

    /// <summary>
    /// カテゴリが存在するか確認
    /// </summary>
    bool HasCategory(const std::string& _name) const;

    /// <summary>
    /// 登録済みカテゴリ一覧を取得
    /// </summary>
    const std::unordered_map<std::string, CollisionCategory>& GetCategories() const { return categories_; }

    /// <summary>
    /// 登録数を取得
    /// </summary>
    size_t GetCategoryCount() const { return categories_.size(); }

    /// <summary>
    /// 全カテゴリをクリア
    /// </summary>
    void Clear();

    /// <summary>
    /// マスクに含まれるカテゴリ名をすべて列挙してコールバックを呼ぶ
    /// </summary>
    void ForEachCategoryInMask(uint32_t _mask, auto&& _callback) const;

private:
    CollisionCategoryManager();
    ~CollisionCategoryManager()                                          = default;
    CollisionCategoryManager(const CollisionCategoryManager&)            = delete;
    CollisionCategoryManager& operator=(const CollisionCategoryManager&) = delete;

private:
    CollisionCategory defaultCategory_;
    std::array<std::string, kMaxCategories> indexToName_;
    std::unordered_map<std::string, CollisionCategory> categories_;
};

} // namespace OriGine
