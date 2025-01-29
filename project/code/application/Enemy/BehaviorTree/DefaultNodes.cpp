#include "DefaultNodes.h"

#include "../IEnemy.h"
#include "animation/Animation.h"
#include "animation/AnimationManager.h"

namespace EnemyBehavior {
Status EnemyBehavior::ChangeAnimation::tick() {
    AnimationObject3d* animationObject3d = enemy_->getDrawObject3d();
    AnimationManager* animationManager   = AnimationManager::getInstance();
    std::unique_ptr<Animation> animation = std::move(animationManager->Load("resource/Animations", animationName_));

    if (time_ > 0.0f) {
        animation->setDuration(time_);
    }

    animationObject3d->setAnimation("resource/Animations", animationName_);

    return Status::SUCCESS;
}
} // namespace EnemyBehavior
