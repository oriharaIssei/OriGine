#include "IComponent.h"

#include "componentManager/ComponentManager.h"

void IComponent::Destroy(){
	ComponentManager::getInstance()->Destroy(variableName_);
}