#include "CollisionManager.h"

#include "Engine.h"
#include "animation/Animation.h"
#include "object3d/AnimationObject3d.h"

CollisionManager::CollisionManager() {
    colliders_.reserve(100);
}

CollisionManager::~CollisionManager() {}

void CollisionManager::Update() {
    for (std::vector<Collider*>::iterator aItr = colliders_.begin();
         aItr != colliders_.end();
         ++aItr) {

        Collider* aCollider = *aItr;
        if (!aCollider->getIsAlive()) {
            break;
        }
        aCollider->UpdateMatrix();

        std::vector<Collider*>::iterator bItr = aItr;
        ++bItr;
        for (; bItr != colliders_.end(); ++bItr) {

            Collider* bCollider = *bItr;
            if (!bCollider->getIsAlive()) {
                break;
            }
            bCollider->UpdateMatrix();

            if (aItr != bItr) {
                CheckCollisionPair(aCollider, bCollider);
            }
        }
    }

}

void CollisionManager::CheckCollisionPair(
    Collider* a,
    Collider* b) {
    Vec3f aPos = a->getPosition();
    Vec3f bPos   = b->getPosition();

    Vec3f distance = aPos - bPos;

    float radiusSum = a->getRadius() + b->getRadius();

    if (distance.lengthSq() < radiusSum * radiusSum) {
        a->OnCollision(b->getHostObject());
        b->OnCollision(a->getHostObject());
    }
}
