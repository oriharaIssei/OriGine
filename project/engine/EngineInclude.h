#pragma once

#ifdef ENGINE_INCLUDE
#include "Engine.h"
#endif // ENGINE_INCLUDE

#ifdef RESOURCE_DIRECTORY
#include <string>
static const std::string kApplicationResourceDirectory = "application/resource";
static const std::string kEngineResourceDirectory      = "engine/resource";
#endif // RESOURCE_DIRECTORY

#ifdef ENGINE_ECS
#include "ECS/ECSManager.h"
#include "ECS/Entity.h"

#define ENGINE_SYSTEMS
#define ENGINE_COMPONENTS
#endif // ENGINE_ECS

#ifdef ENGINE_SYSTEMS
#include "system/ISystem.h"

#include "system/collision/CollisionCheckSystem.h"
#include "system/movement/MoveSystemByRigidBody.h"
#include "system/render/SpritRenderSystem.h"
#include "system/render/TexturedMeshRenderSystem.h"
#endif // ENGINE_SYSTEMS

#ifdef ENGINE_COMPONENTS
#include "component/collider/Collider.h"
#include "component/IComponent.h"
#include "component/physics/Rigidbody.h"
#include "component/renderer/MeshRender.h"
#include "component/renderer/Sprite.h"
#include "component/transform/CameraTransform.h"
#include "component/transform/Transform.h"
#endif // ENGINE_COMPONENTS
