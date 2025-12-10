#include "VignetteParam.h"

/// engine
#define ENGINE_ECS
#define ENGINE_INCLUDE
#include "engine/EngineInclude.h"
// directX12
#include "directX12/DxDevice.h"

#ifdef _DEBUG
/// util
#include "myGui/MyGui.h"
/// externals
#include "imgui/imgui.h"
#endif // _DEBUG

using namespace OriGine;

void VignetteParam::Initialize(Entity* /*_entity*/) {
    paramBuffer.CreateBuffer(Engine::GetInstance()->GetDxDevice()->device_);
}

void VignetteParam::Finalize() {
    paramBuffer.Finalize();
}

void VignetteParam::Edit(Scene* /*_scene*/,Entity*/* _entity*/,[[maybe_unused]] const std::string& _parentLabel) {
    
#ifdef _DEBUG

    ImGui::Text("VignettePow");
    DragGuiCommand("##VignettePow" + _parentLabel, paramBuffer.openData_.vignettePow, 0.01f, 0.0f);
    ImGui::Text("VignetteScale");
    DragGuiCommand("##VignetteScale" + _parentLabel, paramBuffer.openData_.vignetteScale, 0.01f, 0.0f);
    ImGui::Text("VignetteColor");
    ColorEditGuiCommand("##VignetteColor" + _parentLabel, paramBuffer.openData_.color);

#endif // _DEBUG
}

void OriGine::to_json(nlohmann::json& j, const VignetteParam& p) {
    j["vignettePow"]   = p.paramBuffer->vignettePow;
    j["vignetteScale"] = p.paramBuffer->vignetteScale;
    j["color"]         = p.paramBuffer->color;
}

void OriGine::from_json(const nlohmann::json& j, VignetteParam& p) {
    j.at("vignettePow").get_to(p.paramBuffer.openData_.vignettePow);
    j.at("vignetteScale").get_to(p.paramBuffer.openData_.vignetteScale);
    j.at("color").get_to(p.paramBuffer.openData_.color);
}
