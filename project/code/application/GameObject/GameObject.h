#pragma once

/// stl
//memory
#include <memory>
//string
#include <string>
/// Engine
// component
#include "Object3d/AnimationObject3d.h"
#include "application/Collision/Collider.h"
#include "transform/Transform.h"

class GameObject {
public:
    GameObject(const std::string& id);
    virtual ~GameObject();
    virtual void Init()   = 0;
    virtual void Update() = 0;
    virtual void Draw();

protected:
    std::unique_ptr<AnimationObject3d> drawObject3d_;

    std::unique_ptr<Collider> hitCollider_;

    bool isAlive_ = false;

private:
    std::string id_;

public:
    bool getIsAlive() const { return isAlive_; }
    void setIsAlive(bool isAlive) { isAlive_ = isAlive; }

    const std::string& getID() const { return id_; }

    Collider* getHitCollider() const { return hitCollider_.get(); }

    AnimationObject3d* getDrawObject3d() const {
        return drawObject3d_.get();
    }

    const Transform& getTransform() const {
        return drawObject3d_->transform_;
    }

    const Vec3f& getScale() const {
        return drawObject3d_->transform_.scale;
    }
    const Quaternion& getRotate() const {
        return drawObject3d_->transform_.rotate;
    }
    const Vec3f& getTranslate() const {
        return drawObject3d_->transform_.translate;
    }

    void setScale(const Vec3f& s) {
        drawObject3d_->transform_.scale = s;
    }
    void setScaleX(float x) {
        drawObject3d_->transform_.scale.x() = x;
    }
    void setScaleY(float y) {
        drawObject3d_->transform_.scale.y() = y;
    }
    void setScaleZ(float z) {
        drawObject3d_->transform_.scale.z() = z;
    }

    void setRotate(const Quaternion& q) {
        drawObject3d_->transform_.rotate = q;
    }
    void setRotateX(float x) {
        drawObject3d_->transform_.rotate.x = x;
    }
    void setRotateY(float y) {
        drawObject3d_->transform_.rotate.y = y;
    }
    void setRotateZ(float z) {
        drawObject3d_->transform_.rotate.z = z;
    }
    void setRotateW(float w) {
        drawObject3d_->transform_.rotate.w = w;
    }

    void setTranslate(const Vec3f& t) {
        drawObject3d_->transform_.translate = t;
    }
    void setTranslateX(float x) {
        drawObject3d_->transform_.translate.x() = x;
    }
    void setTranslateY(float y) {
        drawObject3d_->transform_.translate.y() = y;
    }
    void setTranslateZ(float z) {
        drawObject3d_->transform_.translate.z() = z;
    }
};
