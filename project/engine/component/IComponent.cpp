#include "IComponent.h"

/// engine
// ECS
#include "Entity.h"

IComponent::IComponent(GameEntity* _hostEntity) {
    // Constructor implementation
    hostEntity_ = _hostEntity;
}

IComponent::~IComponent() {
    // Destructor implementation
}
