#pragma once

#ifdef ENGINE_INCLUDE
#include "Engine.h"
#endif // ENGINE_INCLUDE

#ifdef ENGINE_EDITOR
#include "editor/EditorController.h"
#include "editor/IEditor.h"
#endif // ENGINE_EDITOR

#ifdef RESOURCE_DIRECTORY
#include <string>
static const ::std::string kApplicationResourceDirectory = "./application/resource";
static const ::std::string kEngineResourceDirectory      = "./engine/resource";
#endif // RESOURCE_DIRECTORY

#ifdef ENGINE_SCENE
#include "scene/Scene.h"
#include "scene/SceneManager.h"
#endif // ENGINE_SCENE

#ifdef ENGEINE_ECS
#define ENGINE_ENTITY
#define ENGINE_SYSTEMS
#define ENGINE_COMPONENTS
#endif // ENGINE_ECS

#ifdef ENGINE_ENTITY
#include "entity/Entity.h"
#endif // ENGINE_ECS

#ifdef ENGINE_SYSTEMS
#include "system/ISystem.h"
#include "system/SystemRegistry.h"
#include "system/SystemRunner.h"

#include "system/movement/SubSceneUpdate.h"
#include "system/postRender/SubSceneRender.h"

#include "system/initialize/CameraInitialize.h"
#include "system/initialize/GpuParticleInitialize.h"
#include "system/initialize/RegisterWindowResizeEvent.h"
#include "system/initialize/ResolveEntityReferences.h"

#include "system/collision/CollisionCheckSystem.h"
#include "system/collision/CollisionPushBackSystem.h"

#include "system/movement/MoveSystemByRigidBody.h"

#include "system/effect/CameraActionSystem.h"
#include "system/effect/EmitterWorkSystem.h"
#include "system/effect/GpuParticleEmitterWorkSystem.h"
#include "system/effect/MaterialAnimationWorkSystem.h"
#include "system/effect/MaterialEffect.h"
#include "system/effect/PrimitiveNodeAnimationWorkSystem.h"
#include "system/effect/SkinningAnimationSystem.h"
#include "system/effect/SpriteAnimationSystem.h"

#include "system/render/BackGroundSpriteRenderSystem.h"
#include "system/render/ColliderRenderingSystem.h"
#include "system/render/GpuParticleRenderSystem.h"
#include "system/render/LineRenderSystem.h"
#include "system/render/OverlayRenderSystem.h"
#include "system/render/ParticleRenderSystem.h"
#include "system/render/SkeletonRenderSystem.h"
#include "system/render/SkinningMeshRenderSystem.h"
#include "system/render/SkyboxRender.h"
#include "system/render/SpriteRenderSystem.h"
#include "system/render/TexturedMeshRenderSystem.h"
#include "system/render/VelocityRenderingSystem.h"

#include "system/postRender/DissolveEffect.h"
#include "system/postRender/DistortionEffect.h"
#include "system/postRender/GradationEffect.h"
#include "system/postRender/GrayscaleEffect.h"
#include "system/postRender/RadialBlurEffect.h"
#include "system/postRender/RandomEffect.h"
#include "system/postRender/SmoothingEffect.h"
#include "system/postRender/SpeedlineEffect.h"
#include "system/postRender/VignetteEffect.h"
#endif // ENGINE_SYSTEMS

#ifdef ENGINE_COMPONENTS
#include "component/ComponentArray.h"
#include "component/ComponentRegistry.h"
#include "component/ComponentRepository.h"
#include "component/IComponent.h"

#include "audio/Audio.h"

#include "component/EntityReferenceList.h"
#include "component/SubScene.h"

#include "component/transform/CameraTransform.h"
#include "component/transform/Transform.h"

#include "component/material/light/DirectionalLight.h"
#include "component/material/light/PointLight.h"
#include "component/material/light/SpotLight.h"
#include "component/material/Material.h"

#include "component/animation/MaterialAnimation.h"
#include "component/animation/ModelNodeAnimation.h"
#include "component/animation/PrimitiveNodeAnimation.h"
#include "component/animation/SkinningAnimationComponent.h"
#include "component/animation/SpriteAnimation.h"
#include "component/effect/CameraAction.h"

#include "component/collision/collider/AABBCollider.h"
#include "component/collision/collider/Collider.h"
#include "component/collision/collider/OBBCollider.h"
#include "component/collision/collider/SphereCollider.h"

#include "component/collision/CollisionPushBackInfo.h"
#include "component/physics/Rigidbody.h"

#include "component/effect/MaterialEffectPipeLine.h"
#include "component/effect/particle/emitter/Emitter.h"
#include "component/effect/particle/gpuParticle/GpuParticle.h"
#include "component/effect/post/DissolveEffectParam.h"
#include "component/effect/post/DistortionEffectParam.h"
#include "component/effect/post/GradationComponent.h"
#include "component/effect/post/RadialBlurParam.h"
#include "component/effect/post/RandomEffectParam.h"
#include "component/effect/post/SpeedlineEffectParam.h"
#include "component/effect/post/VignetteParam.h"

#include "component/renderer/MeshRenderer.h"
#include "component/renderer/primitive/base/PrimitiveMeshRendererBase.h"
#include "component/renderer/primitive/BoxRenderer.h"
#include "component/renderer/primitive/CylinderRenderer.h"
#include "component/renderer/primitive/PlaneRenderer.h"
#include "component/renderer/primitive/RingRenderer.h"
#include "component/renderer/primitive/SphereRenderer.h"

#include "component/renderer/SkyboxRenderer.h"
#include "component/renderer/Sprite.h"

#endif // ENGINE_COMPONENTS

#ifdef ENGINE_INPUT
#include "input/InputManager.h"
#endif // ENGINE_INPUT

#ifdef LIBRARY_INCLUDE
#define GLOBALVARIABLES_INCLUDE
#define MY_FILESYSTEM
#define MY_RANDOM
#define DELTA_TIME
#endif RARY_INCLUDE

#ifdef GLOBALVARIABLES_INCLUDE
#include "globalVariables/GlobalVariables.h"
#include "globalVariables/SerializedField.h"
#endif // GLOBALVARIABLES_INCLUDE

#ifdef MY_FS
#define MY_FILESYSTEM
#endif // MY_FS

#ifdef MY_FILESYSTEM
#include "myFileSystem/MyFileSystem.h"
#endif // MY_FILESYSTEM

#ifdef MY_RANDOM
#include "myRandom/MyRandom.h"
#endif // MY_RANDOM

#ifdef DELTA_TIME
#ifndef ENGINE_INCLUDE
#include "Engine.h"
#endif // !ENGINE_INCLUDE

inline float GetMainDeltaTime() {
    return OriGine::Engine::GetInstance()->GetDeltaTime();
}
#endif // DELTA_TIME
