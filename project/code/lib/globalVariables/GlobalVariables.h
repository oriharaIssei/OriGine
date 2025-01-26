#pragma once

#include "Vector2.h"
#include "Vector3.h"
#include "Vector4.h"

#include <assert.h>
#include <iostream>
#include <map>
#include <string>
#include <variant>

/// <summary>
/// 定数値を json に保存する
/// </summary>
class GlobalVariables {
public:
    static GlobalVariables* getInstance();

    void Update();

    void LoadAllFile();
    void LoadFile(const std::string& scene, const std::string& groupName);
    void SaveScene(const std::string& scene);
    void SaveFile(const std::string& scene, const std::string& groupName);

private:
    GlobalVariables();
    ~GlobalVariables();
    GlobalVariables(const GlobalVariables&)      = delete;
    GlobalVariables* operator=(GlobalVariables&) = delete;

private:
    void ImGuiMenu();

public:
    using Item  = std::variant<int32_t, float, std::string, Vec2f, Vec3f, Vec4f, bool>;
    using Group = std::map<std::string, Item>;
    using Scene = std::map<std::string, Group>;

private:
    std::map<std::string, Scene> data_;

    std::string currentScene_ = "NULL";
    int currentSceneNum_      = 0;

    std::string currentGroupName_ = "NULL";
    int currentGroupNum_          = 0;
    Group* currentGroup_          = nullptr;

public:
    Scene* getScene(const std::string& scene) {
        auto itr = data_.find(scene);
        if (itr == data_.end()) {
            return nullptr;
        }
        return &itr->second;
    }

    // 新しいアイテムを作成してセット
    template <typename T>
    void setValue(const std::string& scene, const std::string& groupName, const std::string& itemName, const T& value) {
        data_[scene][groupName][itemName] = value;
    }

    // アイテムを取得、存在しない場合は作成してそのポインタを返す
    template <typename T>
    T* addValue(const std::string& scene, const std::string& groupName, const std::string& itemName) {
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
        setValue(scene, groupName, itemName, T{});
        return std::get_if<T>(&group[itemName]);
    }

    // アイテムの値を取得（const）
    template <typename T>
    T getValue(const std::string& scene, const std::string& groupName, const std::string& itemName) const {
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

    // Editor用の関数
    void ChangeGroupName(const std::string& scene, const std::string& oldGroupName, const std::string& newGroupName) {
        data_[scene][newGroupName] = std::move(data_[scene][oldGroupName]);
        data_[scene].erase(oldGroupName);
    }
};
