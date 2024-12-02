#include "ComponentManager.h"

#include <algorithm>


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
	if(!activeInstancesPool_[activeInstanceIndices_[variableName]]){
		// すでに インスタンスが 存在しない
		return false;
	}
	activeInstancesPool_[activeInstanceIndices_[variableName]].reset();
	return true;
}

template <class T>
inline std::string CreateName(T* t){
	std::string name = typeid(*t).name();
	name = name.substr(std::string("class ").length());
	return name;
}

std::string ComponentManager::getTypeName(IComponent* instance) const{
	return CreateName(instance);
}