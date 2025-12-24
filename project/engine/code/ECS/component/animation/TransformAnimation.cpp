#include "TransformAnimation.h"

#ifdef _DEBUG

/// engine
#include "scene/Scene.h"

/// util
#include "myGui/MyGui.h"
#include "util/timeline/Timeline.h"

#endif // _DEBUG
#include <component/transform/Transform.h>

using namespace OriGine;

TransformAnimation::TransformAnimation() {}
TransformAnimation::~TransformAnimation() {}

void TransformAnimation::Initialize(Scene* /*_scene*/, EntityHandle /*_entity*/) {
    currentTime_ = 0.0f;
}

void TransformAnimation::Finalize() {
    animationState_ = {};
    currentTime_    = 0.0f;

    scaleCurve_.clear();
    rotateCurve_.clear();
    translateCurve_.clear();
}

void TransformAnimation::Edit(
    [[maybe_unused]] Scene* _scene,
    [[maybe_unused]] EntityHandle _entity,
    [[maybe_unused]] const ::std::string& _parentLabel) {

#ifdef _DEBUG

    ::std::string label;

    // -----------------------------
    // Duration
    // -----------------------------
    label = "Duration##" + _parentLabel;
    DragGuiCommand(label, duration_);

    label = "LocalTime##" + _parentLabel;
    SlideGuiCommand("LocalTime", currentTime_, 0.f, duration_);
    ::ImGui::Spacing();

    label                       = "Target Transform Index##" + _parentLabel;
    auto& transforms            = _scene->GetComponents<Transform>(_entity);
    int32_t entityTransformSize = static_cast<int32_t>(transforms.size()) - 1;

    InputGuiCommand(label, targetTransformIndex_);
    targetTransformIndex_ = std::clamp(targetTransformIndex_, -1, entityTransformSize);

    ::ImGui::Spacing();

    // -----------------------------
    // Play / Loop
    // -----------------------------
    CheckBoxCommand("Is Loop##" + _parentLabel, animationState_.isLoop_);
    CheckBoxCommand("Is Play##" + _parentLabel, animationState_.isPlay_);

    CheckBoxCommand("Is Debug Play##" + _parentLabel, isDebugPlay_);
    ::ImGui::Spacing();

    // -----------------------------
    // Interpolation Type
    // -----------------------------
    label = "InterpolationType##" + _parentLabel;
    if (::ImGui::BeginCombo(label.c_str(), InterpolationTypeName[int(interpolationType_)])) {
        for (int i = 0; i < (int)InterpolationType::COUNT; ++i) {
            if (::ImGui::Selectable(
                    InterpolationTypeName[i],
                    interpolationType_ == InterpolationType(i))) {
                OriGine::EditorController::GetInstance()->PushCommand(
                    std::make_unique<SetterCommand<InterpolationType>>(
                        &interpolationType_,
                        InterpolationType(i)));
            }
        }
        ::ImGui::EndCombo();
    }

    ::ImGui::Spacing();

    // -----------------------------
    // Transform Animation Curves
    // -----------------------------
    label                      = "Transform Animation##" + _parentLabel;
    ImGuiTableFlags tableFlags = ImGuiTableFlags_ScrollX;

    if (::ImGui::TreeNode(label.c_str())) {
        label = "TransformKeyFrames##" + _parentLabel;
        if (::ImGui::BeginTable(label.c_str(), 2, tableFlags)) {
            ::ImGui::TableSetupColumn("Name");
            ::ImGui::TableSetupColumn("Edit");
            ::ImGui::TableHeadersRow();

            // =============================
            // Scale
            // =============================
            ::ImGui::TableNextRow();
            ::ImGui::TableSetColumnIndex(0);
            ::ImGui::TextUnformatted("Scale");
            ::ImGui::TableSetColumnIndex(1);
            ::ImGui::EditKeyFrame(
                " Scale##" + _parentLabel,
                scaleCurve_,
                duration_,
                Vec3f(1.f, 1.f, 1.f));

            ::ImGui::TableNextRow();
            ::ImGui::TableSetColumnIndex(0);
            ::ImGui::Separator();
            ::ImGui::TableSetColumnIndex(1);
            ::ImGui::Separator();

            // =============================
            // Rotate
            // =============================
            ::ImGui::TableNextRow();
            ::ImGui::TableSetColumnIndex(0);
            ::ImGui::TextUnformatted("Rotate");
            ::ImGui::TableSetColumnIndex(1);

            // Quaternion 用（既存の EditKeyFrameQuaternion がある前提）
            ::ImGui::EditKeyFrame(
                " Rotate##" + _parentLabel,
                rotateCurve_,
                duration_);

            ::ImGui::TableNextRow();
            ::ImGui::TableSetColumnIndex(0);
            ::ImGui::Separator();
            ::ImGui::TableSetColumnIndex(1);
            ::ImGui::Separator();

            // =============================
            // Translate
            // =============================
            ::ImGui::TableNextRow();
            ::ImGui::TableSetColumnIndex(0);
            ::ImGui::TextUnformatted("Translate");
            ::ImGui::TableSetColumnIndex(1);
            ::ImGui::EditKeyFrame(
                " Translate##" + _parentLabel,
                translateCurve_,
                duration_,
                Vec3f(0.f, 0.f, 0.f));

            ::ImGui::EndTable();
        }
        ::ImGui::TreePop();
    }

#endif // _DEBUG
}

void TransformAnimation::Update(float _deltaTime, Transform* _transform) {
    if (!IsPlaying()) {
        return;
    }

    animationState_.isEnd_ = false;
    currentTime_ += _deltaTime;

    if (currentTime_ >= duration_) {
        if (animationState_.isLoop_) {
            currentTime_ = 0.0f;
        } else {
            EndAnimation();
        }
    }

    UpdateTransform(_transform);
}

void TransformAnimation::UpdateTransform(Transform* _transform) {
    switch (interpolationType_) {
    case InterpolationType::LINEAR:
        if (!scaleCurve_.empty()) {
            _transform->scale = CalculateValue::Linear(scaleCurve_, currentTime_);
        }
        if (!rotateCurve_.empty()) {
            _transform->rotate = CalculateValue::Linear(rotateCurve_, currentTime_);
        }
        if (!translateCurve_.empty()) {
            _transform->translate = CalculateValue::Linear(translateCurve_, currentTime_);
        }
        break;

    case InterpolationType::STEP:
        if (!scaleCurve_.empty()) {
            _transform->scale = CalculateValue::Step(scaleCurve_, currentTime_);
        }
        if (!rotateCurve_.empty()) {
            _transform->rotate = CalculateValue::Step(rotateCurve_, currentTime_);
        }
        if (!translateCurve_.empty()) {
            _transform->translate = CalculateValue::Step(translateCurve_, currentTime_);
        }
        break;
    default:
        break;
    }

    _transform->UpdateMatrix();
}

void TransformAnimation::PlayStart() {
    currentTime_            = 0.0f;
    animationState_.isPlay_ = true;
    animationState_.isEnd_  = false;
}

void TransformAnimation::Stop() {
    animationState_.isPlay_ = false;
    animationState_.isEnd_  = true;
}

void TransformAnimation::RescaleDuration(float _newDuration) {
    auto rescale = [_newDuration, this](auto& _curve) {
        for (auto& key : _curve) {
            key.time = (key.time / duration_) * _newDuration;
        }
    };

    rescale(scaleCurve_);
    rescale(rotateCurve_);
    rescale(translateCurve_);

    duration_ = _newDuration;
}

void OriGine::to_json(nlohmann::json& _json, const TransformAnimation& _anim) {
    _json["duration"]             = _anim.duration_;
    _json["isLoop"]               = _anim.animationState_.isLoop_;
    _json["isPlay"]               = _anim.animationState_.isPlay_;
    _json["InterpolationType"]    = _anim.interpolationType_;
    _json["targetTransformIndex"] = _anim.targetTransformIndex_;

    auto writeCurve = [&_json](const std::string& name, const auto& curve) {
        nlohmann::json arr = nlohmann::json::array();
        for (const auto& k : curve) {
            arr.push_back({{"time", k.time}, {"value", k.value}});
        }
        _json[name] = arr;
    };

    writeCurve("scaleCurve", _anim.scaleCurve_);
    writeCurve("rotateCurve", _anim.rotateCurve_);
    writeCurve("translateCurve", _anim.translateCurve_);
}

void OriGine::from_json(const nlohmann::json& _json, TransformAnimation& _anim) {
    _json.at("duration").get_to(_anim.duration_);
    _json.at("isLoop").get_to(_anim.animationState_.isLoop_);
    _json.at("isPlay").get_to(_anim.animationState_.isPlay_);
    _json.at("InterpolationType").get_to(_anim.interpolationType_);
    _json.at("targetTransformIndex").get_to(_anim.targetTransformIndex_);

    auto readCurve = [&_json](const std::string& name, auto& curve) {
        for (const auto& k : _json.at(name)) {
            typename std::remove_reference<decltype(curve)>::type::value_type key;
            k.at("time").get_to(key.time);
            k.at("value").get_to(key.value);
            curve.push_back(key);
        }
    };

    readCurve("scaleCurve", _anim.scaleCurve_);
    readCurve("rotateCurve", _anim.rotateCurve_);
    readCurve("translateCurve", _anim.translateCurve_);
}
