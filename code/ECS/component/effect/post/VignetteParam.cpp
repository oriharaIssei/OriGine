#include "VignetteParam.h"

#define ENGINE_ECS
#define ENGINE_INCLUDE
#include "directX12/DxDevice.h"
#include "engine/EngineInclude.h"

#ifdef _DEBUG
#include "imgui/imgui.h"
#include "myGui/MyGui.h"
#endif // _DEBUG

using namespace OriGine;

void VignetteParam::Initialize(Scene* /*_scene*/, EntityHandle /*_owner*/) {
    paramBuffer.CreateBuffer(Engine::GetInstance()->GetDxDevice()->device_);
}

void VignetteParam::Finalize() {
    paramBuffer.Finalize();
}

void VignetteParam::Edit(Scene* /*_scene*/, EntityHandle /*_owner*/, [[maybe_unused]] const std::string& _parentLabel) {

#ifdef _DEBUG

    ImGui::Text("VignettePow");
    DragGuiCommand("##VignettePow" + _parentLabel, paramBuffer.openData_.vignettePow, 0.01f, 0.0f);
    ImGui::Text("VignetteScale");
    DragGuiCommand("##VignetteScale" + _parentLabel, paramBuffer.openData_.vignetteScale, 0.01f, 0.0f);
    ImGui::Text("VignetteColor");
    ColorEditGuiCommand("##VignetteColor" + _parentLabel, paramBuffer.openData_.color);

#endif // _DEBUG
}

void OriGine::to_json(nlohmann::json& _j, const VignetteParam& _comp) {
    _j["vignettePow"]   = _comp.paramBuffer.openData_.vignettePow;
    _j["vignetteScale"] = _comp.paramBuffer.openData_.vignetteScale;
    _j["color"]         = _comp.paramBuffer.openData_.color;
}

void OriGine::from_json(const nlohmann::json& _j, VignetteParam& _comp) {
    _j.at("vignettePow").get_to(_comp.paramBuffer.openData_.vignettePow);
    _j.at("vignetteScale").get_to(_comp.paramBuffer.openData_.vignetteScale);
    _j.at("color").get_to(_comp.paramBuffer.openData_.color);
}
