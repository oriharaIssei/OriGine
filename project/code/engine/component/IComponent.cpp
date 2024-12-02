#include "IComponent.h"

#include "componentManager/ComponentManager.h"

IComponent::IComponent(){
	int32_t instanceIndex_ = 0;
	while(ComponentManager::getInstance()->PutOn(this,typeName_ + std::to_string(instanceIndex_))){
		++instanceIndex_;
	}
}

void IComponent::Init(){}

void IComponent::Update(){}
