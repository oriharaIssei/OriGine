#include "ComponentManager.h"

#include <algorithm>

#include <typeinfo>

ComponentManager* ComponentManager::getInstance(){
	static ComponentManager instance;
	return &instance;
}

void ComponentManager::Init(){
	activeInstancesPool_.reserve(1000); // 初期容量の予約
	freeIndices_.reserve(1000);
}

void ComponentManager::UpdateActiveComponents(){
	for(auto& instance : activeInstancesPool_){
		if(instance){
			instance->Update();
		}
	}
}

bool ComponentManager::PutOn(IComponent* instance,const std::string& variableName){
	if(activeInstanceIndices_.count(variableName)){
		// 同じ名前のインスタンスが既に存在する
		return false;
	}

	size_t index;
	if(!freeIndices_.empty()){
		// フリーリストから再利用
		index = freeIndices_.back();
		freeIndices_.pop_back();
		activeInstancesPool_[index].reset(instance);
	} else{
		// 新しいインデックスを割り当て
		index = activeInstancesPool_.size();
		activeInstancesPool_.emplace_back(instance);
	}

	activeInstanceIndices_[variableName] = index;
	return true;
}

bool ComponentManager::Destroy(const std::string& variableName){
	auto it = activeInstanceIndices_.find(variableName);
	if(it == activeInstanceIndices_.end()){
		// 存在しないインスタンス
		return false;
	}

	size_t index = it->second;
	activeInstancesPool_[index].reset(); // インスタンスを削除
	freeIndices_.push_back(index);       // インデックスをフリーリストに追加
	activeInstanceIndices_.erase(it);    // マッピングを削除

	return true;
}

void ComponentManager::CreateComponentInstance(IComponent* instance){
	instance->typeName_ = typeid(*instance).name();

	int32_t num = 0;
	instance->variableName_ =
		instance->typeName_ + std::to_string(num);

	while(PutOn(instance,instance->variableName_)){
		++num;
		instance->variableName_ =
			instance->typeName_ + std::to_string(num);
	}
}

void CreateComponentInstance(IComponent* instance){
	ComponentManager::getInstance()->CreateComponentInstance(instance);
}
