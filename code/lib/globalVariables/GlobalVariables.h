#pragma once

#include <map>

#include <variant>

#include <iostream>

#include <assert.h>

#include "Vector2.h"
#include "Vector3.h"
#include "Vector4.h"

#include <string>

class GlobalVariables
{
public:
	static GlobalVariables* getInstance();

	void Update();

	void CreateScene(const std::string& scene);

	void LoadAllFile();
	void LoadFile(const std::string& scene,const std::string& groupName);
	void SaveScene(const std::string& scene);
	void SaveFile(const std::string& scene,const std::string& groupName);

private:
	GlobalVariables() = default;
	~GlobalVariables() = default;
	GlobalVariables(const GlobalVariables&) = delete;
	GlobalVariables* operator=(GlobalVariables&) = delete;
private:
	void ImGuiMenu();
private:
	struct Item
	{
		std::variant<int32_t,float,Vector2,Vector3,Vector4,bool> value;
		std::variant<int32_t*,float*,Vector2*,Vector3*,Vector4*,bool*> valuePtr;
	};

	using Group = std::map<std::string,Item>;
	using Scene = std::map<std::string,Group>;

	std::map<std::string,Scene> data_;

#ifdef _DEBUG
	std::string currentScene_ = "ALL";
	int currentSceneNum_ = 0;

	std::string currentGroupName_ = "NULL";
	int currentGroupNum_ = 0;
	Group* currentGroup_ = nullptr;
#endif // _DEBUG

public:
	template<typename T>
	void setValue(const std::string& scene,const std::string& groupName,const std::string& itemName,T& value)
	{
		Group& group = data_[scene][groupName];
		Item newItem = {value,&value};
		group[itemName] = newItem;
	}

	template<typename T>
	bool addValue(const std::string& scene,const std::string& groupName,const std::string& itemName,T& value)
	{
		auto& group = data_[scene][groupName];
		auto itemItr = group.find(itemName);
		if(itemItr != group.end())
		{
// 型が一致する場合のみ処理
			if(auto ptr = std::get_if<T>(&itemItr->second.value))
			{
				value = *ptr;
				itemItr->second.valuePtr = &value;  // ポインタを保存
				return false;
			} else
			{
				throw std::runtime_error("Type mismatch for existing item");
			}
		}
		setValue(scene,groupName,itemName,value);
		return true;
	}
	template<typename T>
	T getValue(const std::string& scene,const std::string& groupName,const std::string& itemName) const
	{
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
		try
		{
			return std::get<T>(itemItr->second.value);
		} catch(const std::bad_variant_access&)
		{
			throw std::runtime_error("Incorrect type requested");
		}
	}

	void DestroyItem(const std::string& scene,const std::string& groupName,const std::string& itemName)
	{
// Sceneの存在を確認
		auto sceneItr = data_.find(scene);
		assert(sceneItr != data_.end());
		// groupNameの存在を確認
		auto groupItr = sceneItr->second.find(groupName);
		assert(groupItr != sceneItr->second.end());

		// itemNameの存在を確認
		auto itemItr = groupItr->second.find(itemName);
		assert(itemItr != groupItr->second.end());

		std::visit([&](auto& value){
			using T = std::decay_t<decltype(value)>;
			if constexpr(std::is_same_v<T,int32_t>)
			{
				itemItr->second.valuePtr = &value;
			} else if constexpr(std::is_same_v<T,float>)
			{
				itemItr->second.valuePtr = &value;
			} else if constexpr(std::is_same_v<T,Vector2>)
			{
				itemItr->second.valuePtr = &value;
			} else if constexpr(std::is_same_v<T,Vector3>)
			{
				itemItr->second.valuePtr = &value;
			} else if constexpr(std::is_same_v<T,Vector4>)
			{
				itemItr->second.valuePtr = &value;
			} else if constexpr(std::is_same_v<T,bool>)
			{
				itemItr->second.valuePtr = &value;
			} else
			{
				throw std::runtime_error("Unsupported type");
			}
		},itemItr->second.value);
	}
};