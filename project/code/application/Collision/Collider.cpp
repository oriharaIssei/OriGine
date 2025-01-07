#include "Collider.h"

#include "../GameObject/GameObject.h"
#include "object3d/Object3d.h"

Collider::Collider(const std::string& name)
    : radius_("Game", name, "radius") {}

Collider::~Collider() {}

void Collider::Init(std::function<void(GameObject*)> _onCollision) {
    isAlive_ = true;

    transform_.Init();

    currentRadius_ = radius_;

    onCollision_ = _onCollision;

    drawObject3d_ = std::make_unique<Object3d>();
    drawObject3d_->Init("resource/Models", "sphere.obj");
}

void Collider::OnCollision(GameObject* collisionObject) {
    if (onCollision_) {
        onCollision_(collisionObject);
    }
}

void Collider::UpdateMatrix() {
    transform_.UpdateMatrix();

#ifdef _DEBUG
    currentRadius_ = radius_;

    drawObject3d_->transform_.scale     = Vector3(currentRadius_, currentRadius_, currentRadius_);
    drawObject3d_->transform_.translate = transform_.worldMat[3];
    drawObject3d_->UpdateTransform();
#endif // _DEBUG
}

void Collider::Draw() {
    if (drawObject3d_) {
        drawObject3d_->Draw();
    }
}

void Collider::resetRadius(const std::string& id) {
    radius_        = {"Game", id, "radius"};
    currentRadius_ = radius_;
}
