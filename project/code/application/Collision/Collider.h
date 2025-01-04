#pragma once

///stl
//memory
#include <functional>

//object
#include "../GameObject/GameObject.h"
///engine
//component
#include "transform/Transform.h"
class Object3d;
//lib
#include "globalVariables/SerializedField.h"

class Collider {
public:
    Collider(const std::string& name);
    ~Collider();

    void Init(std::function<void(GameObject*)> _onCollision = nullptr);
    void OnCollision(GameObject* collisionObject);

    void UpdateMatrix();
    void Draw();

private:
    GameObject* hostObject_ = nullptr;

    std::unique_ptr<Object3d> drawObject3d_;

    Transform transform_;
    SerializedField<float> radius_;
    float currentRadius_ = 0.0f;

    std::function<void(GameObject*)> onCollision_ = nullptr;

    bool isAlive_ = false;

public:
    bool getIsAlive() const { return isAlive_; }
    void setIsAlive(bool isAlive) { isAlive_ = isAlive; }

    void setHostObject(GameObject* hostObject) { hostObject_ = hostObject; }
    GameObject* getHostObject() { return hostObject_; }
    void setParent(Transform* parent) { transform_.parent = parent; }

    float getRadius() const { return currentRadius_; }

    Vector3 getPosition() const { return transform_.worldMat[3]; }

    const Vector3& getScale() const { return transform_.scale; }
    const Quaternion& getRotate() const { return transform_.rotate; }
    const Vector3& getTranslate() const { return transform_.translate; }

    void setScale(const Vector3& s) { transform_.scale = s; }
    void setRotate(const Quaternion& q) { transform_.rotate = q; }
    void setTranslate(const Vector3& t) { transform_.translate = t; }
};
