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

namespace OriGine {

/// <summary>
/// 定数値を json に保存する
/// </summary>
class GlobalVariables {

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
    void LoadFile(const std::string& _scene, const std::string& _groupName);
    /// <summary>
    /// GlobalVariablesのシーン(フォルダー)を保存する
    /// </summary>
    /// <param name="scene">保存するシーン名(フォルダー)</param>
    void SaveScene(const std::string& _scene);
    /// <summary>
    /// Jsonファイルにデータを保存する
    /// </summary>
    /// <param name="scene">保存するデータが所属するシーン名(フォルダー)</param>
    /// <param name="groupName">保存するグループ名(ファイル)</param>
    void SaveFile(const std::string& _scene, const std::string& _groupName);

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
    /// <summary>
    /// 指定されたシーンのデータを取得
    /// </summary>
    /// <param name="scene">シーン名</param>
    /// <returns>シーンデータへのポインタ</returns>
    Scene* GetScene(const std::string& _scene) {
        auto itr = data_.find(_scene);
        if (itr == data_.end()) {
            return nullptr;
        }
        return &itr->second;
    }

    // 新しいアイテムを作成してセット
    template <typename T>
    void SetValue(const std::string& _scene, const std::string& _groupName, const std::string& _itemName, const T& _value) {
        data_[_scene][_groupName][_itemName] = _value;
    }

    // アイテムを取得、存在しない場合は作成してそのポインタを返す
    template <typename T>
    T* AddValue(const std::string& _scene, const std::string& _groupName, const std::string& _itemName) {
        // グループを取得
        auto& group = data_[_scene][_groupName];
        // アイテムを検索
        auto itemItr = group.find(_itemName);

        if (itemItr != group.end()) {
            // 既存のアイテムが存在し、型が一致する場合はそのポインタを返す
            if (auto* ptr = std::get_if<T>(&itemItr->second)) {
                return ptr;
            } else {
                throw std::runtime_error("Type mismatch for existing item.");
            }
        }

        // アイテムが存在しない場合は新規作成してポインタを返す
        SetValue(_scene, _groupName, _itemName, T{});
        return std::get_if<T>(&group[_itemName]);
    }

    // アイテムを取得、存在しない場合は作成してそのポインタを返す
    template <typename T>
    T* AddValue(const std::string& _scene, const std::string& _groupName, const std::string& _itemName, const T& _defaultValue) {
        // グループを取得
        auto& group = data_[_scene][_groupName];
        // アイテムを検索
        auto itemItr = group.find(_itemName);

        if (itemItr != group.end()) {
            // 既存のアイテムが存在し、型が一致する場合はそのポインタを返す
            if (auto* ptr = std::get_if<T>(&itemItr->second)) {
                return ptr;
            } else {
                throw std::runtime_error("Type mismatch for existing item.");
            }
        }

        // アイテムが存在しない場合は新規作成してポインタを返す
        SetValue(_scene, _groupName, _itemName, _defaultValue);
        return std::get_if<T>(&group[_itemName]);
    }

    // アイテムの値を取得（const）
    template <typename T>
    T GetValue(const std::string& _scene, const std::string& _groupName, const std::string& _itemName) const {
        // Sceneの存在を確認
        auto sceneItr = data_.find(_scene);
        assert(sceneItr != data_.end());

        // groupNameの存在を確認
        auto groupItr = sceneItr->second.find(_groupName);
        assert(groupItr != sceneItr->second.end());

        // itemNameの存在を確認
        auto itemItr = groupItr->second.find(_itemName);
        assert(itemItr != groupItr->second.end());

        // 指定された型で値を取得
        try {
            return std::get<T>(itemItr->second);
        } catch (const std::bad_variant_access&) {
            throw std::runtime_error("Incorrect type requested.");
        }
    }

    /// <summary>
    /// 指定されたグループを削除する
    /// </summary>
    /// <param name="scene">シーン名</param>
    /// <param name="groupName">グループ名</param>
    void DestroyGroup(const std::string& _scene, const std::string& _groupName) {
        data_[_scene][_groupName].clear();
    }
    /// <summary>
    /// 指定された値を削除する
    /// </summary>
    /// <param name="scene">シーン名</param>
    /// <param name="groupName">グループ名</param>
    /// <param name="valueName">値の名前</param>
    void DestroyValue(const std::string& _scene, const std::string& _groupName, const std::string& _valueName) {
        data_[_scene][_groupName].erase(_valueName);
    }

#ifdef _DEBUG
    // Editor用の関数
    void ChangeGroupName(const std::string& _scene, const std::string& _oldGroupName, const std::string& _newGroupName) {
        data_[_scene][_newGroupName] = std::move(data_[_scene][_oldGroupName]);
        data_[_scene].erase(_oldGroupName);
    }
#endif // _DEBUG
};

} // namespace OriGine
