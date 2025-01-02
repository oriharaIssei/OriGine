#pragma once

/// stl
//memory
#include <memory>
/// Engine
// component
#include "Object3d/Object3d.h"
#include "transform/Transform.h"

class GameObject {
public:
    GameObject();
    virtual ~GameObject();
    virtual void Init() = 0;
    virtual void Update() = 0;
    virtual void Draw();

private:
    std::unique_ptr<Object3d> drawObject3d_;
    Transform transform_;

    bool isAlive_ = false;

public:
    bool getIsAlive() const { return isAlive_; }
    void setIsAlive(bool isAlive) { isAlive_ = isAlive; }

    const Vector3& getScale() const {
        return transform_.scale;
    }

    const Quaternion& getRotate() const {
        return transform_.rotate;
    }

    const Vector3& getTranslate() const {
        return transform_.translate;
    }

    void setScale(const Vector3& s) {
        transform_.scale = s;
    }

    void setRotate(const Quaternion& q) {
        transform_.rotate = q;
    }

    void setTranslate(const Vector3& t) {
        transform_.translate = t;
    }
};
