#include "GameObject.h"

GameObject::GameObject() {}

GameObject::~GameObject() {}

void GameObject::Draw() {
    drawObject3d_->Draw();
}
