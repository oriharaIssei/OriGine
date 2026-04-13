#include "TransformRateAnimation.h"

#ifdef _DEBUG

/// engine
#include "scene/Scene.h"

/// util
#include "myGui/MyGui.h"

#endif // _DEBUG

#include <component/transform/Transform.h>

using namespace OriGine;

TransformRateAnimation::TransformRateAnimation() {}
TransformRateAnimation::~TransformRateAnimation() {}

void TransformRateAnimation::Initialize(Scene* /*_scene*/, EntityHandle /*_entity*/) {}

void TransformRateAnimation::Finalize() {
    isPlay_              = false;
    targetTransformIndex_ = -1;
    scaleRate_            = {};
    rotateRate_           = {};
    translateRate_        = {};
}

void TransformRateAnimation::Edit(
    [[maybe_unused]] Scene* _scene,
    [[maybe_unused]] EntityHandle _entity,
    [[maybe_unused]] const ::std::string& _parentLabel) {

#ifdef _DEBUG

    ::std::string label;

    // Target Transform Index
    label                       = "Target Transform Index##" + _parentLabel;
    auto& transforms            = _scene->GetComponents<Transform>(_entity);
    int32_t entityTransformSize = static_cast<int32_t>(transforms.size()) - 1;

    InputGuiCommand(label, targetTransformIndex_);
    targetTransformIndex_ = std::clamp(targetTransformIndex_, -1, entityTransformSize);

    ::ImGui::Spacing();

    // Play
    CheckBoxCommand("Is Play##" + _parentLabel, isPlay_);

    ::ImGui::Spacing();

    // Scale
    ::ImGui::TextUnformatted("Scale");
    DragGuiVectorCommand<3, float>("Velocity##scaleRateV" + _parentLabel, scaleRate_.velocity, 0.01f);
    DragGuiVectorCommand<3, float>("Acceleration##scaleRateA" + _parentLabel, scaleRate_.acceleration, 0.01f);
    ::ImGui::Spacing();

    // Rotate (rad/s)
    ::ImGui::TextUnformatted("Rotate (rad/s)");
    DragGuiVectorCommand<3, float>("Velocity##rotateRateV" + _parentLabel, rotateRate_.velocity, 0.01f);
    DragGuiVectorCommand<3, float>("Acceleration##rotateRateA" + _parentLabel, rotateRate_.acceleration, 0.01f);
    ::ImGui::Spacing();

    // Translate
    ::ImGui::TextUnformatted("Translate");
    DragGuiVectorCommand<3, float>("Velocity##translateRateV" + _parentLabel, translateRate_.velocity, 0.01f);
    DragGuiVectorCommand<3, float>("Acceleration##translateRateA" + _parentLabel, translateRate_.acceleration, 0.01f);

#endif // _DEBUG
}

void TransformRateAnimation::Update(float _deltaTime, Transform* _transform) {
    if (!isPlay_) {
        return;
    }

    // Scale: velocity 更新 → 値に加算
    scaleRate_.velocity = scaleRate_.velocity + scaleRate_.acceleration * _deltaTime;
    _transform->scale   = _transform->scale + scaleRate_.velocity * _deltaTime;

    // Rotate: angular velocity 更新 → 各軸の回転をクォータニオンで適用（ラジアン/秒）
    rotateRate_.velocity    = rotateRate_.velocity + rotateRate_.acceleration * _deltaTime;
    Vec3f angleDelta        = rotateRate_.velocity * _deltaTime;
    Quaternion rotationStep = Quaternion::FromEulerAngles(angleDelta);
    _transform->rotate      = _transform->rotate * rotationStep;

    // Translate: velocity 更新 → 値に加算
    translateRate_.velocity = translateRate_.velocity + translateRate_.acceleration * _deltaTime;
    _transform->translate   = _transform->translate + translateRate_.velocity * _deltaTime;

    _transform->UpdateMatrix();
}

// =============================================================================
// JSON シリアライズ
// =============================================================================

void OriGine::to_json(nlohmann::json& _j, const TransformRateAnimation& _comp) {
    _j["targetTransformIndex"] = _comp.targetTransformIndex_;
    _j["isPlay"]               = _comp.isPlay_;

    auto writeRate = [&_j](const std::string& _name, const TransformRateAnimation::RateParam& _rate) {
        _j[_name] = {
            {"velocity", _rate.velocity},
            {"acceleration", _rate.acceleration}
        };
    };
    writeRate("scaleRate", _comp.scaleRate_);
    writeRate("rotateRate", _comp.rotateRate_);
    writeRate("translateRate", _comp.translateRate_);
}

void OriGine::from_json(const nlohmann::json& _j, TransformRateAnimation& _comp) {
    _j.at("targetTransformIndex").get_to(_comp.targetTransformIndex_);
    _j.at("isPlay").get_to(_comp.isPlay_);

    auto readRate = [&_j](const std::string& _name, TransformRateAnimation::RateParam& _rate) {
        if (_j.contains(_name)) {
            const auto& r = _j.at(_name);
            r.at("velocity").get_to(_rate.velocity);
            r.at("acceleration").get_to(_rate.acceleration);
        }
    };
    readRate("scaleRate", _comp.scaleRate_);
    readRate("rotateRate", _comp.rotateRate_);
    readRate("translateRate", _comp.translateRate_);
}
