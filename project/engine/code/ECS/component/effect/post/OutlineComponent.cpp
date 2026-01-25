#include "OutlineComponent.h"

/// engine
#include "Engine.h"
#include "scene/Scene.h"

/// ECS
// component
#include "component/material/Material.h"

#include "component/renderer/MeshRenderer.h"
#include "component/renderer/primitive/BoxRenderer.h"
#include "component/renderer/primitive/CylinderRenderer.h"
#include "component/renderer/primitive/PlaneRenderer.h"
#include "component/renderer/primitive/RingRenderer.h"
#include "component/renderer/primitive/SphereRenderer.h"

#ifdef _DEBUG
#include "myGui/MyGui.h"
#endif // DEBUG

using namespace OriGine;

OutlineComponent::OutlineComponent() {}
OutlineComponent::~OutlineComponent() {}

void OutlineComponent::Initialize(Scene* /*_scene*/, EntityHandle /*_owner*/) {
    paramData.CreateBuffer(Engine::GetInstance()->GetDxDevice()->device_);
}

void OutlineComponent::Finalize() {
    paramData.Finalize();
}

void OutlineComponent::Edit([[maybe_unused]] Scene* _scene, [[maybe_unused]] EntityHandle _owner, [[maybe_unused]] const std::string& _parentLabel) {
#ifdef _DEBUG
    CheckBoxCommand("Is Active##" + _parentLabel, isActive);

    ImGui::Spacing();

    ColorEditGui("Outline Color##" + _parentLabel, paramData.openData_.outlineColor);

    ImGui::Spacing();

    DragGuiCommand("Outline Width##" + _parentLabel, paramData.openData_.outlineWidth, 0.01f);

    ImGui::Spacing();

    // material
    auto& materials = _scene->GetComponents<Material>(_owner);
    if (materials.empty()) {
        ImGui::Text("Material is not assigned!");
    } else {
        Material* material = nullptr;
        if (usingMaterialHandle.IsValid()) {
            material = _scene->GetComponent<Material>(usingMaterialHandle);
        }
        if (!material) {
            material = _scene->GetComponent<Material>(_owner);
        }

        int32_t preMaterialIndex = -1;
        if (usingMaterialHandle.IsValid()) {
            for (int32_t mIndex = 0; mIndex < materials.size(); ++mIndex) {
                if (materials[mIndex].GetHandle() == usingMaterialHandle) {
                    preMaterialIndex = mIndex;
                    break;
                }
            }
        }
        int32_t materialIndex = preMaterialIndex;
        ImGui::InputInt(("Material Index##" + _parentLabel).c_str(), &materialIndex);
        materialIndex = std::clamp(materialIndex, 0, static_cast<int32_t>(materials.size()) - 1);
        if (materialIndex != preMaterialIndex) {
            ComponentHandle newMaterialHandle;
            if (materialIndex != -1) {
                newMaterialHandle = materials[materialIndex].GetHandle();
            }
            auto setMaterialCommand = std::make_unique<SetterCommand<ComponentHandle>>(
                &usingMaterialHandle,
                newMaterialHandle);
            OriGine::EditorController::GetInstance()->PushCommand(std::move(setMaterialCommand));
        }
    }

#endif // DEBUG
}

void OriGine::to_json(nlohmann::json& _j, const OutlineComponent& _comp) {
    _j["isActive"]            = _comp.isActive;
    _j["usingMaterialHandle"] = _comp.usingMaterialHandle;
    _j["outlineWidth"]        = _comp.paramData.openData_.outlineWidth;
    _j["outlineColor"]        = _comp.paramData.openData_.outlineColor;
}

void OriGine::from_json(const nlohmann::json& _j, OutlineComponent& _comp) {
    _j.at("isActive").get_to(_comp.isActive);
    _j.at("usingMaterialHandle").get_to(_comp.usingMaterialHandle);
    _j.at("outlineWidth").get_to(_comp.paramData.openData_.outlineWidth);
    _j.at("outlineColor").get_to(_comp.paramData.openData_.outlineColor);
}
