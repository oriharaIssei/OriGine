#include "ComponentManager.h"

#include <algorithm>

// リフレクション登録のためのマクロ
#define SetReflection(className) namespace className##Reflection

ComponentManager* ComponentManager::getInstance(){
	static ComponentManager instance{};
	return &instance;
}

bool ComponentManager::PutOn(IComponent* instance,const std::string& variableName){
	if(activeInstancesPool_[activeInstanceIndices_[variableName]]){
		// すでに インスタンスが 存在する
		return false;
	}
	std::sort(freeIndices_.begin(),freeIndices_.end(),std::greater<>());
	activeInstancesPool_[freeIndices_.back()] = std::unique_ptr<IComponent>(instance);
	freeIndices_.pop_back();
	return true;
}

bool ComponentManager::Destroy(const std::string& variableName){
	activeInstancesPool_[activeInstanceIndices_[variableName]].reset();
	return false;
}
