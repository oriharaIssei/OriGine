#pragma once

///stl
#include <vector>

///engine
#include "Collider.h"

class CollisionManager {
public:
    CollisionManager();
    ~CollisionManager();

    void Update();
private:
    void CheckCollisionPair(Collider* a, Collider* b);

    std::vector<Collider*> colliders_;

public:
    void addCollider(Collider* collider) {
        if (collider) {
            colliders_.push_back(collider);
        }
    }
    void clearCollider() {
        colliders_.clear();
    }
};
