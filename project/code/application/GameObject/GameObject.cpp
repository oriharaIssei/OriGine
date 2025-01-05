#include "GameObject.h"

GameObject::GameObject(const std::string& id)
    : id_(id) {}

GameObject::~GameObject() {}

void GameObject::Draw() {
    drawObject3d_->Draw();
}
