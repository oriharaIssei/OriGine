#pragma once

///stl
//memory
#include <functional>

//object
class GameObject;
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

private:
    GameObject* hostObject_ = nullptr;

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
    void resetRadius(const std::string& id);

    Vec3f getPosition() const { return transform_.worldMat[3]; }

    const Vec3f& getScale() const { return transform_.scale; }
    const Quaternion& getRotate() const { return transform_.rotate; }
    const Vec3f& getTranslate() const { return transform_.translate; }

    void setScale(const Vec3f& s) { transform_.scale = s; }
    void setRotate(const Quaternion& q) { transform_.rotate = q; }
    void setTranslate(const Vec3f& t) { transform_.translate = t; }
};
