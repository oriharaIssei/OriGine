#include "CameraAction.h"

#ifdef DEBUG
#include "math/RotateUtil.h"
#include "myGui/MyGui.h"
#include "util/timeline/Timeline.h"
#endif // DEBUG

CameraAction::CameraAction() {}

CameraAction::~CameraAction() {}

void CameraAction::Initialize(GameEntity* /*_entity*/) {}

bool CameraAction::Edit() {
    bool isChanged = false;

#ifdef DEBUG
    if (ImGui::TreeNode("AnimationState")) {
        isChanged |= CheckBoxCommand("Is Playing", animationState_.isPlay_);
        isChanged |= CheckBoxCommand("Is Looping", animationState_.isLoop_);
        ImGui::TreePop();
    }

    isChanged |= DragGuiCommand("Duration", duration_, 0.01f, 0.001f);

    ImGui::Separator();
    ImGui::Spacing();

    ImGui::Text("FOV Animation");
    isChanged |= ImGui::EditKeyFrame(
        "FOV Animation",
        fovCurve_,
        duration_,
        degreeToRadian(45.0f));

    ImGui::Text("Aspect Ratio Animation");
    isChanged |= ImGui::EditKeyFrame(
        "Aspect Ratio Animation",
        aspectRatioCurve_,
        duration_,
        16.0f / 9.0f);
    ImGui::Text("Near Z Animation");
    isChanged |= ImGui::EditKeyFrame(
        "Near Z Animation",
        nearZCurve_,
        duration_,
        0.1f);

    ImGui::Text("Far Z Animation");
    isChanged |= ImGui::EditKeyFrame(
        "Far Z Animation",
        farZCurve_,
        duration_,
        1000.0f);

    
    ImGui::Text("Rotation Animation");
    isChanged |= ImGui::EditKeyFrame(
        "Rotation Animation",
        rotationCurve_,
        duration_,
        Quaternion(0.0f, 0.0f, 0.0f, 1.0f));

    ImGui::Text("Position Animation");
    isChanged |= ImGui::EditKeyFrame(
        "Position Animation",
        positionCurve_,
        duration_);

#endif // DEBUG

    return isChanged;
}

void CameraAction::Finalize() {}

void to_json(nlohmann::json& j, const CameraAction& action) {
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

void from_json(const nlohmann::json& j, CameraAction& action) {
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
