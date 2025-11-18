#pragma once

/// stl
#include <assert.h>
#include <iostream>
#include <map>
#include <string>
#include <variant>

/// Engine
// editor
#include "editor/IEditor.h"

/// math
#include "Vector2.h"
#include "Vector3.h"
#include "Vector4.h"

class GlobalVariablesEditor;

/// <summary>
/// 定数値を json に保存する
/// </summary>
class GlobalVariables {

#ifdef _DEBUG
    friend class GlobalVariablesEditor;
#endif // _DEBUG

public:
    static GlobalVariables* GetInstance();

    /// <summary>
    /// GlobalVariablesのデータをすべて読み込む
    /// </summary>
    void LoadAllFile();
    /// <summary>
    /// Jsonファイルからデータを読み込む
    /// </summary>
    /// <param name="scene">入力元のシーン名(フォルダー)</param>
    /// <param name="groupName">入力元のグループ名(ファイル)</param>
    void LoadFile(const std::string& scene, const std::string& groupName);
    /// <summary>
    /// GlobalVariablesのシーン(フォルダー)を保存する
    /// </summary>
    /// <param name="scene">保存するシーン名(フォルダー)</param>
    void SaveScene(const std::string& scene);
    /// <summary>
    /// Jsonファイルにデータを保存する
    /// </summary>
    /// <param name="scene">保存するデータが所属するシーン名(フォルダー)</param>
    /// <param name="groupName">保存するグループ名(ファイル)</param>
    void SaveFile(const std::string& scene, const std::string& groupName);

private:
    GlobalVariables();
    ~GlobalVariables();
    GlobalVariables(const GlobalVariables&)      = delete;
    GlobalVariables* operator=(GlobalVariables&) = delete;

public:
    using Item  = std::variant<int32_t, float, std::string, Vec2f, Vec3f, Vec4f, bool>;
    using Group = std::map<std::string, Item>;
    using Scene = std::map<std::string, Group>;

private:
    std::map<std::string, Scene> data_;

public:
    Scene* GetScene(const std::string& scene) {
        auto itr = data_.find(scene);
        if (itr == data_.end()) {
            return nullptr;
        }
        return &itr->second;
    }

    // 新しいアイテムを作成してセット
    template <typename T>
    void SetValue(const std::string& scene, const std::string& groupName, const std::string& itemName, const T& value) {
        data_[scene][groupName][itemName] = value;
    }

    // アイテムを取得、存在しない場合は作成してそのポインタを返す
    template <typename T>
    T* AddValue(const std::string& scene, const std::string& groupName, const std::string& itemName) {
        // グループを取得
        auto& group = data_[scene][groupName];
        // アイテムを検索
        auto itemItr = group.find(itemName);

        if (itemItr != group.end()) {
            // 既存のアイテムが存在し、型が一致する場合はそのポインタを返す
            if (auto* ptr = std::get_if<T>(&itemItr->second)) {
                return ptr;
            } else {
                throw std::runtime_error("Type mismatch for existing item.");
            }
        }

        // アイテムが存在しない場合は新規作成してポインタを返す
        SetValue(scene, groupName, itemName, T{});
        return std::get_if<T>(&group[itemName]);
    }

    // アイテムを取得、存在しない場合は作成してそのポインタを返す
    template <typename T>
    T* AddValue(const std::string& scene, const std::string& groupName, const std::string& itemName,const T& defaultValue) {
        // グループを取得
        auto& group = data_[scene][groupName];
        // アイテムを検索
        auto itemItr = group.find(itemName);

        if (itemItr != group.end()) {
            // 既存のアイテムが存在し、型が一致する場合はそのポインタを返す
            if (auto* ptr = std::get_if<T>(&itemItr->second)) {
                return ptr;
            } else {
                throw std::runtime_error("Type mismatch for existing item.");
            }
        }

        // アイテムが存在しない場合は新規作成してポインタを返す
        SetValue(scene, groupName, itemName, defaultValue);
        return std::get_if<T>(&group[itemName]);
    }

    // アイテムの値を取得（const）
    template <typename T>
    T GetValue(const std::string& scene, const std::string& groupName, const std::string& itemName) const {
        // Sceneの存在を確認
        auto sceneItr = data_.find(scene);
        assert(sceneItr != data_.end());

        // groupNameの存在を確認
        auto groupItr = sceneItr->second.find(groupName);
        assert(groupItr != sceneItr->second.end());

        // itemNameの存在を確認
        auto itemItr = groupItr->second.find(itemName);
        assert(itemItr != groupItr->second.end());

        // 指定された型で値を取得
        try {
            return std::get<T>(itemItr->second);
        } catch (const std::bad_variant_access&) {
            throw std::runtime_error("Incorrect type requested.");
        }
    }

    void DestroyGroup(const std::string& scene, const std::string& groupName) {
        data_[scene][groupName].clear();
    }
    void DestroyValue(const std::string& scene, const std::string& groupName, const std::string& valueName) {
        data_[scene][groupName].erase(valueName);
    }

#ifdef _DEBUG
    // Editor用の関数
    void ChangeGroupName(const std::string& scene, const std::string& oldGroupName, const std::string& newGroupName) {
        data_[scene][newGroupName] = std::move(data_[scene][oldGroupName]);
        data_[scene].erase(oldGroupName);
    }
#endif // _DEBUG
};
