#include "CameraAction.h"

#ifdef DEBUG
///gui
#include "myGui/MyGui.h"
#include "util/timeline/Timeline.h"
///math
#include <math/mathEnv.h>
#endif // DEBUG

using namespace OriGine;

CameraAction::CameraAction() {}
CameraAction::~CameraAction() {}

void CameraAction::Initialize(Entity* /*_entity*/) {}

void CameraAction::Edit(Scene* /*_scene*/, Entity* /*_entity*/, [[maybe_unused]] [[maybe_unused]] const std::string& _parentLabel) {

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

void OriGine::to_json(nlohmann::json& j, const CameraAction& action) {
    j = nlohmann::json{
        {"duration", action.duration_},
        {"isLoop", action.animationState_.isLoop_},
        {"isPlay", action.animationState_.isPlay_}};

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

    j["fovCurve"]         = curveSave(action.fovCurve_);
    j["aspectRatioCurve"] = curveSave(action.aspectRatioCurve_);
    j["nearZCurve"]       = curveSave(action.nearZCurve_);
    j["farZCurve"]        = curveSave(action.farZCurve_);
    j["positionCurve"]    = curveSave(action.positionCurve_);
    j["rotationCurve"]    = curveSave(action.rotationCurve_);
}

void OriGine::from_json(const nlohmann::json& j, CameraAction& action) {
    j.at("duration").get_to(action.duration_);
    j.at("isLoop").get_to(action.animationState_.isLoop_);
    j.at("isPlay").get_to(action.animationState_.isPlay_);

    auto curveLoad = [](const nlohmann::json& _curveJson, auto& _curve) {
        for (auto& keyframeJson : _curveJson) {
            typename std::remove_reference<decltype(_curve)>::type::value_type key;
            keyframeJson.at("time").get_to(key.time);
            keyframeJson.at("value").get_to(key.value);
            _curve.push_back(key);
        }
    };

    curveLoad(j.at("fovCurve"), action.fovCurve_);
    curveLoad(j.at("aspectRatioCurve"), action.aspectRatioCurve_);
    curveLoad(j.at("nearZCurve"), action.nearZCurve_);
    curveLoad(j.at("farZCurve"), action.farZCurve_);
    curveLoad(j.at("positionCurve"), action.positionCurve_);
    curveLoad(j.at("rotationCurve"), action.rotationCurve_);
}
