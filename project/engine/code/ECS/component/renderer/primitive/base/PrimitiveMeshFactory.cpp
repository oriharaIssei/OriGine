#include "PrimitiveMeshFactory.h"

/// engine
// log
#include "logger/Logger.h"

/// ecs
// primitiveMesh
#include "component/renderer/primitive/BoxRenderer.h"
#include "component/renderer/primitive/CylinderRenderer.h"
#include "component/renderer/primitive/PlaneRenderer.h"
#include "component/renderer/primitive/RingRenderer.h"
#include "component/renderer/primitive/SphereRenderer.h"

using namespace OriGine;

std::shared_ptr<PrimitiveMeshRendererBase> PrimitiveMeshFactory::CreatePrimitiveMeshBy(PrimitiveType _type) {
    auto itr = primitiveMeshFactory_.find(_type);
    if (itr != primitiveMeshFactory_.end()) {
        return itr->second();
    }
    // 見つからなかった場合 Log出す
    LOG_WARN("Factory function for PrimitiveType not found. Type : {}", std::to_string(_type));
    return std::shared_ptr<PrimitiveMeshRendererBase>();
}

void PrimitiveMeshFactory::Initialize() {
    // Box
    RegistryFactoryFunction(PrimitiveType::Box, []() {
        return std::make_shared<BoxRenderer>();
    });
    // Sphere
    RegistryFactoryFunction(PrimitiveType::Sphere, []() {
        return std::make_shared<SphereRenderer>();
    });
    // Cylinder
    RegistryFactoryFunction(PrimitiveType::Cylinder, []() {
        return std::make_shared<CylinderRenderer>();
    });
    // Plane
    RegistryFactoryFunction(PrimitiveType::Plane, []() {
        return std::make_shared<PlaneRenderer>();
    });
    // Ring
    RegistryFactoryFunction(PrimitiveType::Ring, []() {
        return std::make_shared<RingRenderer>();
    });
}

void PrimitiveMeshFactory::RegistryFactoryFunction(PrimitiveType _type, std::function<std::shared_ptr<PrimitiveMeshRendererBase>()> _func) {
    auto itr = primitiveMeshFactory_.find(_type);
    if (itr != primitiveMeshFactory_.end()) {
        LOG_WARN("Factory function for PrimitiveType already exists. Type : {}", std::to_string(_type));
        return;
    }
    primitiveMeshFactory_[_type] = _func;
}
