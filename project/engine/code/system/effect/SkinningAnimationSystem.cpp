#include "SkinningAnimationSystem.h"

#define DELTA_TIME
#include "EngineInclude.h"

/// ECS
#include "ECS/ECSManager.h"
// component
#include "component/animation/SkinningAnimationComponent.h"
#include "component/renderer/MeshRenderer.h"

/// lib
#include "logger/Logger.h"

static void ApplyAnimation(Skeleton& _skeleton, AnimationData* _animationData, float _animationTime) {
    for (Joint& joint : _skeleton.joints) {
        auto itr = _animationData->animationNodes_.find(joint.name);
        if (itr == _animationData->animationNodes_.end()) {
            LOG_WARN("Joint {} not found in animation data", joint.name);
            continue;
        }
        const ModelAnimationNode& nodeAnimation = itr->second;
        joint.transform.scale                   = CalculateValue::Linear(nodeAnimation.scale, _animationTime);
        joint.transform.rotate                  = CalculateValue::Linear(nodeAnimation.rotate, _animationTime);
        joint.transform.translate               = CalculateValue::Linear(nodeAnimation.translate, _animationTime);
    }
}
static void SkeletonUpdate(Skeleton& _skeleton) {
    for (auto& joint : _skeleton.joints) {
        joint.localMatrix = MakeMatrix::Affine(
            joint.transform.scale,
            Quaternion::Normalize(joint.transform.rotate),
            joint.transform.translate);

        if (joint.parent.has_value()) {
            joint.skeletonSpaceMatrix = _skeleton.joints[joint.parent.value()].skeletonSpaceMatrix * joint.localMatrix;
        } else {
            joint.skeletonSpaceMatrix = joint.localMatrix;
        }
    }
}

SkinningAnimationSystem::SkinningAnimationSystem()
    : ISystem(SystemType::Effect) {}
SkinningAnimationSystem::~SkinningAnimationSystem() {}

void SkinningAnimationSystem::Initialize() {}

void SkinningAnimationSystem::Finalize() {}

void SkinningAnimationSystem::UpdateEntity(GameEntity* _entity) {
    if (!_entity) {
        return;
    }

    int32_t compSize = ECSManager::getInstance()->getComponentArray<SkinningAnimationComponent>()->getComponentSize(_entity);

    const float deltaTime = getMainDeltaTime();
    for (int32_t i = 0; i < compSize; ++i) {
        auto* animationComponent = getComponent<SkinningAnimationComponent>(_entity, i);
        if (!animationComponent) {
            continue;
        }
        if (!animationComponent->isPlay()) {
            continue;
        }
        if (!animationComponent->getAnimationData()) {
            continue;
        }

        animationComponent->setIsEnd(false);

        // アニメーションの更新
        float currentTime = animationComponent->getCurrentTime();
        currentTime += deltaTime * animationComponent->getPlaybackSpeed();
        if (currentTime >= animationComponent->getDuration()) {
            if (animationComponent->isLoop()) {
                currentTime = std::fmod(currentTime, animationComponent->getDuration());
            } else {
                currentTime = animationComponent->getDuration();
                animationComponent->setIsEnd(true);
            }
        }

        animationComponent->setCurrentTime(currentTime);

        // アニメーションの状態を更新
        auto* modelRenderer = getComponent<ModelMeshRenderer>(_entity, animationComponent->getBindModeMeshRendererIndex());
        if (!modelRenderer) {
            LOG_ERROR("ModelMeshRenderer not found for entity: {}", _entity->getID());
            return;
        }
        if (!modelRenderer->getSkeleton().has_value()) {
            LOG_ERROR("Skeleton not found for entity: {}", _entity->getID());
            return;
        }

        ApplyAnimation(
            modelRenderer->getSkeletonRef().value(),
            animationComponent->getAnimationData().get(),
            currentTime);
        modelRenderer->getSkeletonRef().value().Update();
    }
}
