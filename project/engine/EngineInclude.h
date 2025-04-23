#pragma once

#ifdef ENGINE_INCLUDE
#include "Engine.h"
#endif // ENGINE_INCLUDE

#ifdef RESOURCE_DIRECTORY
#include <string>
static const std::string kApplicationResourceDirectory = "application/resource";
static const std::string kEngineResourceDirectory      = "engine/resource";
#endif // RESOURCE_DIRECTORY

#ifdef ENGINE_SCENE
#include "iScene/IScene.h"
#include "sceneManager/SceneManager.h"
#endif // ENGINE_SCENE

#ifdef ENGINE_ECS
#include "ECS/ECSManager.h"
#include "ECS/Entity.h"

#define ENGINE_SYSTEMS
#define ENGINE_COMPONENTS
#endif // ENGINE_ECS

#ifdef ENGINE_SYSTEMS
#include "system/ISystem.h"

#include "system/collision/CollisionCheckSystem.h"

#include "system/effect/EmitterWorkSystem.h"
#include "system/effect/PrimitiveNodeAnimationWorkSystem.h"
#include "system/movement/MoveSystemByRigidBody.h"

#include "system/postRender/Grayscale.h"
#include "system/postRender/Vignette.h"

#include "system/render/ColliderRenderingSystem.h"
#include "system/render/LineRenderSystem.h"
#include "system/render/ParticleRenderSystem.h"
#include "system/render/SpriteRenderSystem.h"
#include "system/render/TexturedMeshRenderSystem.h"
#endif // ENGINE_SYSTEMS

#ifdef ENGINE_COMPONENTS
#include "component/IComponent.h"

#include "component/material/light/DirectionalLight.h"
#include "component/material/light/PointLight.h"
#include "component/material/light/SpotLight.h"
#include "component/material/Material.h"
#include "component/animation/ModelNodeAnimation.h"
#include "component/animation/PrimitiveNodeAnimation.h"

#include "component/collider/Collider.h"

#include "component/effect/particle/emitter/Emitter.h"

#include "component/physics/Rigidbody.h"

#include "component/renderer/MeshRenderer.h"
#include "component/renderer/Sprite.h"
#include "component/renderer/primitive/Primitive.h"

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
float getMainDeltaTime() {
    return Engine::getInstance()->getDeltaTime();
}
#endif // DELTA_TIME
