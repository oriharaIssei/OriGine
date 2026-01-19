#include "CameraAction.h"

#ifdef DEBUG
/// gui
#include "myGui/MyGui.h"
#include "util/timeline/Timeline.h"
/// math
#include <math/mathEnv.h>
#endif // DEBUG

using namespace OriGine;

CameraAction::CameraAction() {}
CameraAction::~CameraAction() {}

void CameraAction::Initialize(Scene* /*_scene*/, EntityHandle /*_entity*/) {}

void CameraAction::Edit(Scene* /*_scene*/, EntityHandle /*_entity*/,[[maybe_unused]] const std::string& _parentLabel) {

#ifdef DEBUG
    std::string label = "AnimationState##" + _parentLabel;
    if (ImGui::TreeNode(label.c_str())) {
        CheckBoxCommand("Is Playing##" + _parentLabel, animationState_.isPlay_);
        CheckBoxCommand("Is Looping##" + _parentLabel, animationState_.isLoop_);
        ImGui::TreePop();
    }

    DragGuiCommand("Duration##" + _parentLabel, duration_, 0.01f, 0.001f);

    ImGui::Separator();
    ImGui::Spacing();

    ImGui::Text("FOV Animation");
    ImGui::EditKeyFrame(
        "FOV Animation##" + _parentLabel,
        fovCurve_,
        duration_,
        DegreeToRadian(45.0f));

    ImGui::Text("Aspect Ratio Animation");
    ImGui::EditKeyFrame(
        "Aspect Ratio Animation##" + _parentLabel,
        aspectRatioCurve_,
        duration_,
        16.0f / 9.0f);
    ImGui::Text("Near Z Animation");
    ImGui::EditKeyFrame(
        "Near Z Animation##" + _parentLabel,
        nearZCurve_,
        duration_,
        0.1f);

    ImGui::Text("Far Z Animation");
    ImGui::EditKeyFrame(
        "Far Z Animation##" + _parentLabel,
        farZCurve_,
        duration_,
        1000.0f);

    ImGui::Text("Rotation Animation");
    ImGui::EditKeyFrame(
        "Rotation Animation##" + _parentLabel,
        rotationCurve_,
        duration_,
        Quaternion(0.0f, 0.0f, 0.0f, 1.0f));

    ImGui::Text("Position Animation");
    ImGui::EditKeyFrame(
        "Position Animation##" + _parentLabel,
        positionCurve_,
        duration_);

#endif // DEBUG
}

void CameraAction::Finalize() {}

void OriGine::to_json(nlohmann::json& _j, const CameraAction& _comp) {
    _j = nlohmann::json{
        {"duration", _comp.duration_},
        {"isLoop", _comp.animationState_.isLoop_},
        {"isPlay", _comp.animationState_.isPlay_}};

    auto curveSave = [](const auto& _curve) {
        nlohmann::json curve = nlohmann::json::array();

        for (auto& keyframe : _curve) {
            nlohmann::json keyframeJson = {
                {"time", keyframe.time},
                {"value", keyframe.value}};
            curve.push_back(keyframeJson);
        }
        return curve;
    };

    _j["fovCurve"]         = curveSave(_comp.fovCurve_);
    _j["aspectRatioCurve"] = curveSave(_comp.aspectRatioCurve_);
    _j["nearZCurve"]       = curveSave(_comp.nearZCurve_);
    _j["farZCurve"]        = curveSave(_comp.farZCurve_);
    _j["positionCurve"]    = curveSave(_comp.positionCurve_);
    _j["rotationCurve"]    = curveSave(_comp.rotationCurve_);
}

void OriGine::from_json(const nlohmann::json& _j, CameraAction& _comp) {
    _j.at("duration").get_to(_comp.duration_);
    _j.at("isLoop").get_to(_comp.animationState_.isLoop_);
    _j.at("isPlay").get_to(_comp.animationState_.isPlay_);

    auto curveLoad = [](const nlohmann::json& _curveJson, auto& _curve) {
        for (auto& keyframeJson : _curveJson) {
            typename std::remove_reference<decltype(_curve)>::type::value_type key;
            keyframeJson.at("time").get_to(key.time);
            keyframeJson.at("value").get_to(key.value);
            _curve.push_back(key);
        }
    };

    curveLoad(_j.at("fovCurve"), _comp.fovCurve_);
    curveLoad(_j.at("aspectRatioCurve"), _comp.aspectRatioCurve_);
    curveLoad(_j.at("nearZCurve"), _comp.nearZCurve_);
    curveLoad(_j.at("farZCurve"), _comp.farZCurve_);
    curveLoad(_j.at("positionCurve"), _comp.positionCurve_);
    curveLoad(_j.at("rotationCurve"), _comp.rotationCurve_);
}
