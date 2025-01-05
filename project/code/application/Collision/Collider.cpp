#include "Collider.h"

#include "object3d/Object3d.h"

Collider::Collider(const std::string& name)
    : radius_("Game", name, "radius") {}

Collider::~Collider() {}

void Collider::Init(std::function<void(GameObject*)> _onCollision) {
    transform_.Init();

    currentRadius_ = radius_;

    onCollision_ = _onCollision;

    isAlive_ = true;

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

    drawObject3d_->transform_.scale = Vector3(currentRadius_, currentRadius_, currentRadius_);
    if (transform_.parent) {
        drawObject3d_->transform_.rotate    = transform_.parent->rotate;
        drawObject3d_->transform_.translate = transform_.parent->translate;
    } else {
        drawObject3d_->transform_.rotate    = transform_.parent->rotate;
        drawObject3d_->transform_.translate = transform_.parent->translate;
    }
    drawObject3d_->UpdateTransform();
#endif // _DEBUG
}

void Collider::Draw() {
    if (drawObject3d_) {
        drawObject3d_->Draw();
    }
}
