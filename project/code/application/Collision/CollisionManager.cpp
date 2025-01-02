#include "CollisionManager.h"

CollisionManager::CollisionManager() {}

CollisionManager::~CollisionManager() {}

void CollisionManager::Update() {
    for (std::list<Collider*>::iterator aItr = colliders_.begin();
         aItr != colliders_.end();
         ++aItr) {

        Collider* aCollider = *aItr;
        if (!aCollider->getIsAlive()) {
            break;
        }
        aCollider->UpdateMatrix();

        for (std::list<Collider*>::iterator bItr = aItr;
             bItr != colliders_.end();
             ++bItr) {
            ++bItr;

            Collider* bCollider = *aItr;
            if (!bCollider->getIsAlive()) {
                break;
            }
            bCollider->UpdateMatrix();

            if (aItr != bItr) {
                CheckCollisionPair(*aItr, *bItr);
            }
        }
    }
}

void CollisionManager::CheckCollisionPair(
    Collider* a,
    Collider* b) {
    Vector3 aPos = a->getPosition();
    Vector3 bPos = b->getPosition();

    Vector3 distance = aPos - bPos;

    float radiusSum = a->getRadius() + b->getRadius();

    if (distance.lengthSq() < radiusSum * radiusSum) {
        if (b->getHostObject()) {
            a->OnCollision(b->getHostObject());
        }
        if (a->getHostObject()) {
            b->OnCollision(a->getHostObject());
        }
    }
}
