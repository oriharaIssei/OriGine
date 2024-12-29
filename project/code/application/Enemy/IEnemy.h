#pragma once

/// stl
#include <memory>

/// engine
#include "transform/Transform.h"

//math
#include "Quaternion.h"
#include "Vector3.h"

class Object3d;

class IEnemy {
public:
    IEnemy();
    ~IEnemy();

    virtual void Init()   = 0;
    virtual void Update() = 0;
    virtual void Draw();

    virtual std::unique_ptr<IEnemy> Clone() = 0;

private:
    std::unique_ptr<Object3d> drawObject3d_;
    Transform transform_;

public:
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
