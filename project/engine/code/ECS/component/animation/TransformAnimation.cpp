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

// ---------------------------------------------------------------------------
// 反転ヘルパー
// ---------------------------------------------------------------------------
Vec3f TransformAnimation::ApplyFlip(Vec3f _val, const FlipMask& _flip) {
    if (_flip.x) {
        _val[X] *= -1.0f;
    }
    if (_flip.y) {
        _val[Y] *= -1.0f;
    }
    if (_flip.z) {
        _val[Z] *= -1.0f;
    }
    return _val;
}

Quaternion TransformAnimation::ApplyFlipQ(Quaternion _val, const FlipMask& _flip) {
    // 各虚数成分の符号を反転することで、対応する軸の回転を鏡像化する
    if (_flip.x) {
        _val[X] *= -1.0f;
    }
    if (_flip.y) {
        _val[Y] *= -1.0f;
    }
    if (_flip.z) {
        _val[Z] *= -1.0f;
    }
    return _val;
}

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

    scaleFlip_     = {};
    rotateFlip_    = {};
    translateFlip_ = {};
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
    // Flip Mask (軸ごとの反転)
    // -----------------------------
    label = "Flip##" + _parentLabel;
    if (::ImGui::TreeNode(label.c_str())) {
        ::ImGui::TextUnformatted("Scale");
        CheckBoxCommand("X##scaleFlipX" + _parentLabel, scaleFlip_.x);
        ::ImGui::SameLine();
        CheckBoxCommand("Y##scaleFlipY" + _parentLabel, scaleFlip_.y);
        ::ImGui::SameLine();
        CheckBoxCommand("Z##scaleFlipZ" + _parentLabel, scaleFlip_.z);

        ::ImGui::TextUnformatted("Rotate");
        CheckBoxCommand("X##rotateFlipX" + _parentLabel, rotateFlip_.x);
        ::ImGui::SameLine();
        CheckBoxCommand("Y##rotateFlipY" + _parentLabel, rotateFlip_.y);
        ::ImGui::SameLine();
        CheckBoxCommand("Z##rotateFlipZ" + _parentLabel, rotateFlip_.z);

        ::ImGui::TextUnformatted("Translate");
        CheckBoxCommand("X##translateFlipX" + _parentLabel, translateFlip_.x);
        ::ImGui::SameLine();
        CheckBoxCommand("Y##translateFlipY" + _parentLabel, translateFlip_.y);
        ::ImGui::SameLine();
        CheckBoxCommand("Z##translateFlipZ" + _parentLabel, translateFlip_.z);

        ::ImGui::TreePop();
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
            _transform->scale = ApplyFlip(CalculateValue::Linear(scaleCurve_, currentTime_), scaleFlip_);
        }
        if (!rotateCurve_.empty()) {
            _transform->rotate = ApplyFlipQ(CalculateValue::Linear(rotateCurve_, currentTime_), rotateFlip_);
        }
        if (!translateCurve_.empty()) {
            _transform->translate = ApplyFlip(CalculateValue::Linear(translateCurve_, currentTime_), translateFlip_);
        }
        break;

    case InterpolationType::STEP:
        if (!scaleCurve_.empty()) {
            _transform->scale = ApplyFlip(CalculateValue::Step(scaleCurve_, currentTime_), scaleFlip_);
        }
        if (!rotateCurve_.empty()) {
            _transform->rotate = ApplyFlipQ(CalculateValue::Step(rotateCurve_, currentTime_), rotateFlip_);
        }
        if (!translateCurve_.empty()) {
            _transform->translate = ApplyFlip(CalculateValue::Step(translateCurve_, currentTime_), translateFlip_);
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

void OriGine::to_json(nlohmann::json& _j, const TransformAnimation& _comp) {
    _j["duration"]             = _comp.duration_;
    _j["isLoop"]               = _comp.animationState_.isLoop_;
    _j["isPlay"]               = _comp.animationState_.isPlay_;
    _j["InterpolationType"]    = _comp.interpolationType_;
    _j["targetTransformIndex"] = _comp.targetTransformIndex_;

    auto writeCurve = [&_j](const std::string& _name, const auto& _curve) {
        nlohmann::json arr = nlohmann::json::array();
        for (const auto& k : _curve) {
            arr.push_back({{"time", k.time}, {"value", k.value}});
        }
        _j[_name] = arr;
    };

    writeCurve("scaleCurve", _comp.scaleCurve_);
    writeCurve("rotateCurve", _comp.rotateCurve_);
    writeCurve("translateCurve", _comp.translateCurve_);

    auto writeFlip = [&_j](const std::string& _name, const TransformAnimation::FlipMask& _flip) {
        _j[_name] = {_flip.x, _flip.y, _flip.z};
    };
    writeFlip("scaleFlip", _comp.scaleFlip_);
    writeFlip("rotateFlip", _comp.rotateFlip_);
    writeFlip("translateFlip", _comp.translateFlip_);
}

void OriGine::from_json(const nlohmann::json& _j, TransformAnimation& _comp) {
    _j.at("duration").get_to(_comp.duration_);
    _j.at("isLoop").get_to(_comp.animationState_.isLoop_);
    _j.at("isPlay").get_to(_comp.animationState_.isPlay_);
    _j.at("InterpolationType").get_to(_comp.interpolationType_);
    _j.at("targetTransformIndex").get_to(_comp.targetTransformIndex_);

    auto readCurve = [&_j](const std::string& _name, auto& _curve) {
        for (const auto& k : _j.at(_name)) {
            typename std::remove_reference<decltype(_curve)>::type::value_type key;
            k.at("time").get_to(key.time);
            k.at("value").get_to(key.value);
            _curve.push_back(key);
        }
    };

    readCurve("scaleCurve", _comp.scaleCurve_);
    readCurve("rotateCurve", _comp.rotateCurve_);
    readCurve("translateCurve", _comp.translateCurve_);

    auto readFlip = [&_j](const std::string& _name, TransformAnimation::FlipMask& _flip) {
        if (_j.contains(_name) && _j.at(_name).is_array() && _j.at(_name).size() >= 3) {
            _flip.x = _j.at(_name)[0].get<bool>();
            _flip.y = _j.at(_name)[1].get<bool>();
            _flip.z = _j.at(_name)[2].get<bool>();
        }
    };
    readFlip("scaleFlip", _comp.scaleFlip_);
    readFlip("rotateFlip", _comp.rotateFlip_);
    readFlip("translateFlip", _comp.translateFlip_);
}
