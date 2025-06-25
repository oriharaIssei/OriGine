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
static const std::string kApplicationResourceDirectory = "./application/resource";
static const std::string kEngineResourceDirectory      = "./engine/resource";
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
#include "ECS/Entity.h"
#endif // ENGINE_ECS

#ifdef ENGINE_SYSTEMS
#include "system/ISystem.h"

#include "system/collision/CollisionCheckSystem.h"
#include "system/collision/CollisionPushBackSystem.h"

#include "system/movement/MoveSystemByRigidBody.h"

#include "system/effect/EmitterWorkSystem.h"
#include "system/effect/PrimitiveNodeAnimationWorkSystem.h"
#include "system/effect/SpriteAnimationSystem.h"
#include "system/effect/TextureEffectAnimation.h"

#include "system/postRender/DistortionEffect.h"
#include "system/postRender/GrayscaleEffect.h"
#include "system/postRender/SmoothingEffect.h"
#include "system/postRender/VignetteEffect.h"

#include "system/render/BackGroundSpriteRenderSystem.h"
#include "system/render/ColliderRenderingSystem.h"
#include "system/render/EffectTexturedMeshRenderSystem.h"
#include "system/render/LineRenderSystem.h"
#include "system/render/ParticleRenderSystem.h"
#include "system/render/SkyboxRender.h"
#include "system/render/SpriteRenderSystem.h"
#include "system/render/TexturedMeshRenderSystem.h"
#endif // ENGINE_SYSTEMS

#ifdef ENGINE_COMPONENTS
#include "component/IComponent.h"

#include "component/animation/ModelNodeAnimation.h"
#include "component/animation/PrimitiveNodeAnimation.h"
#include "component/animation/SpriteAnimation.h"
#include "component/material/light/DirectionalLight.h"
#include "component/material/light/PointLight.h"
#include "component/material/light/SpotLight.h"
#include "component/material/Material.h"

#include "component/collider/Collider.h"
#include "component/collider/CollisionPushBackInfo.h"

#include "component/effect/particle/emitter/Emitter.h"
#include "component/effect/post/DistortionEffectParam.h"
#include "component/effect/post/VignetteParam.h"
#include "component/effect/TextureEffectParam.h"

#include "component/physics/Rigidbody.h"

#include "component/renderer/MeshRenderer.h"
#include "component/renderer/primitive/Primitive.h"
#include "component/renderer/SkyboxRenderer.h"
#include "component/renderer/Sprite.h"

#include "component/transform/CameraTransform.h"
#include "component/transform/Transform.h"

#include "audio/Audio.h"
#endif // ENGINE_COMPONENTS

#ifdef ENGINE_INPUT
#include "input/Input.h"
#endif // ENGINE_INPUT

#ifdef LIBRARY_INCLUDE
#define GLOBALVARIABLES_INCLUDE
#define MY_FILESYSTEM
#define MY_RANDOM
#define DELTA_TIME
#endif // LIBRARY_INCLUDE

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

inline float getMainDeltaTime() {
    return Engine::getInstance()->getDeltaTime();
}
#endif // DELTA_TIME
