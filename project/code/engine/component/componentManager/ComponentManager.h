#pragma once

#include <memory>
#include <unordered_map>
#include <vector>
#include <string>
#include <cstdint>
#include <algorithm>

#include "Component/IComponent.h"

class ComponentManager{
public:
	static ComponentManager* getInstance();

	void Init();
	void UpdateActiveComponents();

	bool PutOn(IComponent* instance,const std::string& variableName);
	bool Destroy(const std::string& variableName);

	IComponent* getVariable(const std::string& name){
		auto it = activeInstanceIndices_.find(name);
		if(it != activeInstanceIndices_.end() && it->second < activeInstancesPool_.size() && activeInstancesPool_[it->second]){
			return activeInstancesPool_[it->second].get();
		}
		return nullptr;
	}

	void CreateComponentInstance(IComponent* instance);

private:
	// コンポーネントを格納するプール
	std::vector<std::unique_ptr<IComponent>> activeInstancesPool_;
	// フリーリスト（削除されたインデックスを保持）
	std::vector<size_t> freeIndices_;
	// 名前からインデックスへのマッピング
	std::unordered_map<std::string,size_t> activeInstanceIndices_;
};

void CreateComponentInstance(IComponent* instance);