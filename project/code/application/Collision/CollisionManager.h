#pragma once

///stl
#include <list>

///engine
#include "Collider.h"

class CollisionManager {
public:
    CollisionManager();
    ~CollisionManager();

    void Update();
    void Draw();

private:
    void CheckCollisionPair(Collider* a, Collider* b);

    std::list<Collider*> colliders_;

public:
    void addCollider(Collider* collider) {
        if (collider) {
            colliders_.push_back(collider);
        }
    }
    void removeDeadCollider() {
        colliders_.remove_if([](Collider* c) {
            return !c->getIsAlive() || c == nullptr;
        });
    }
    void clearCollider() {
        colliders_.clear();
    }
};
