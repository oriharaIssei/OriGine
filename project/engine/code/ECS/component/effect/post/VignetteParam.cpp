#include "VignetteParam.h"

#define ENGINE_ECS
#define ENGINE_INCLUDE
#include "engine/EngineInclude.h"
// directX12
#include "directX12/DxDevice.h"

#ifdef _DEBUG
#include "imgui/imgui.h"
#include "myGui/MyGui.h"
#endif // _DEBUG

void VignetteParam::Initialize(GameEntity* /*_entity*/) {
    paramBuffer.CreateBuffer(Engine::getInstance()->getDxDevice()->getDevice());
}

void VignetteParam::Finalize() {
    paramBuffer.Finalize();
}

bool VignetteParam::Edit() {
    bool isEdit = false;
#ifdef _DEBUG

    ImGui::Text("VignettePow");
    isEdit |= DragGuiCommand("##VignettePow", paramBuffer.openData_.vignettePow, 0.01f, 0.0f);
    ImGui::Text("VignetteScale");
    isEdit |= DragGuiCommand("##VignetteScale", paramBuffer.openData_.vignetteScale, 0.01f, 0.0f);
    ImGui::Text("VignetteColor");
    isEdit |= ColorEditGuiCommand("##VignetteColor", paramBuffer.openData_.color);

#endif // _DEBUG
    return isEdit;
}

void to_json(nlohmann::json& j, const VignetteParam& p) {
    j["vignettePow"]   = p.paramBuffer->vignettePow;
    j["vignetteScale"] = p.paramBuffer->vignetteScale;
    j["color"]         = p.paramBuffer->color;
}

void from_json(const nlohmann::json& j, VignetteParam& p) {
    j.at("vignettePow").get_to(p.paramBuffer.openData_.vignettePow);
    j.at("vignetteScale").get_to(p.paramBuffer.openData_.vignetteScale);
    j.at("color").get_to(p.paramBuffer.openData_.color);
}
