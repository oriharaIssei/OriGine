#include "DissolveAnimation.h"

#ifdef _DEBUG
#include "myGui/MyGui.h"
#include "timeline/Timeline.h"
#endif // _DEBUG

using namespace OriGine;

DissolveAnimation::DissolveAnimation() {}
DissolveAnimation::~DissolveAnimation() {}

void DissolveAnimation::Initialize(Scene* /*_scene*/, EntityHandle /*_entity*/) {
    currentTime_ = 0.0f;
}

void DissolveAnimation::Finalize() {
    animationState_ = {};
    currentTime_    = 0.0f;

    thresholdCurve_.clear();
    edgeWidthCurve_.clear();
    outLineColorCurve_.clear();
}

void DissolveAnimation::Edit(
    [[maybe_unused]] Scene* _scene,
    [[maybe_unused]] EntityHandle _entity,
    [[maybe_unused]] const std::string& _parentLabel) {

#ifdef _DEBUG
    std::string label;

    label = "Duration##" + _parentLabel;
    DragGuiCommand(label, duration_);

    label = "LocalTime##" + _parentLabel;
    SlideGuiCommand(label, currentTime_, 0.f, duration_);
    ImGui::Spacing();

    CheckBoxCommand("Is Loop##" + _parentLabel, animationState_.isLoop_);
    CheckBoxCommand("Is Play##" + _parentLabel, animationState_.isPlay_);
    CheckBoxCommand("Is Debug Play##" + _parentLabel, isDebugPlay_);

    ImGui::Spacing();

    label = "InterpolationType##" + _parentLabel;
    if (ImGui::BeginCombo(label.c_str(), InterpolationTypeName[int(interpolationType_)])) {
        for (int i = 0; i < (int)InterpolationType::COUNT; ++i) {
            if (ImGui::Selectable(
                    InterpolationTypeName[i],
                    interpolationType_ == InterpolationType(i))) {
                EditorController::GetInstance()->PushCommand(
                    std::make_unique<SetterCommand<InterpolationType>>(
                        &interpolationType_,
                        InterpolationType(i)));
            }
        }
        ImGui::EndCombo();
    }

    ImGui::Spacing();

    label = "Dissolve Animation##" + _parentLabel;
    if (ImGui::TreeNode(label.c_str())) {
        ImGui::EditKeyFrame(
            " Threshold##" + _parentLabel,
            thresholdCurve_,
            duration_,
            0.0f);

        ImGui::EditKeyFrame(
            " EdgeWidth##" + _parentLabel,
            edgeWidthCurve_,
            duration_,
            0.05f);

        ImGui::EditKeyFrame(
            " OutLineColor##" + _parentLabel,
            outLineColorCurve_,
            duration_,
            Vec4f(1, 1, 1, 1));

        ImGui::TreePop();
    }
#endif
}

void DissolveAnimation::Update(float _deltaTime, DissolveEffectParam* _param) {
    if (!IsPlaying() || !_param) {
        return;
    }

    animationState_.isEnd_ = false;
    currentTime_ += _deltaTime;

    if (currentTime_ >= duration_) {
        if (animationState_.isLoop_) {
            currentTime_ = 0.0f;
        } else {
            Stop();
            animationState_.isEnd_ = true;
            currentTime_           = duration_;
        }
    }

    UpdateParam(_param);
}

void DissolveAnimation::UpdateParam(DissolveEffectParam* _param) {
    switch (interpolationType_) {
    case InterpolationType::LINEAR:
        if (!thresholdCurve_.empty()) {
            _param->SetThreshold(
                CalculateValue::Linear(thresholdCurve_, currentTime_));
        }
        if (!edgeWidthCurve_.empty()) {
            _param->SetEdgeWidth(
                CalculateValue::Linear(edgeWidthCurve_, currentTime_));
        }
        if (!outLineColorCurve_.empty()) {
            _param->SetOutLineColor(
                CalculateValue::Linear(outLineColorCurve_, currentTime_));
        }
        break;

    case InterpolationType::STEP:
        if (!thresholdCurve_.empty()) {
            _param->SetThreshold(
                CalculateValue::Step(thresholdCurve_, currentTime_));
        }
        if (!edgeWidthCurve_.empty()) {
            _param->SetEdgeWidth(
                CalculateValue::Step(edgeWidthCurve_, currentTime_));
        }
        if (!outLineColorCurve_.empty()) {
            _param->SetOutLineColor(
                CalculateValue::Step(outLineColorCurve_, currentTime_));
        }
        break;
    default:
        break;
    }
}

void DissolveAnimation::PlayStart() {
    currentTime_            = 0.0f;
    animationState_.isPlay_ = true;
    animationState_.isEnd_  = false;
}

void DissolveAnimation::Stop() {
    animationState_.isPlay_ = false;
}

void DissolveAnimation::RescaleDuration(float _newDuration) {
    auto rescale = [_newDuration, this](auto& curve) {
        for (auto& k : curve) {
            k.time = (k.time / duration_) * _newDuration;
        }
    };

    rescale(thresholdCurve_);
    rescale(edgeWidthCurve_);
    rescale(outLineColorCurve_);

    duration_ = _newDuration;
}

void OriGine::to_json(nlohmann::json& _j, const DissolveAnimation& _comp) {
    _j["duration"]          = _comp.duration_;
    _j["isLoop"]            = _comp.animationState_.isLoop_;
    _j["isPlay"]            = _comp.animationState_.isPlay_;
    _j["InterpolationType"] = _comp.interpolationType_;

    auto write = [&_j](const char* _name, const auto& _curve) {
        nlohmann::json arr = nlohmann::json::array();
        for (const auto& k : _curve) {
            arr.push_back({{"time", k.time}, {"value", k.value}});
        }
        _j[_name] = arr;
    };

    write("thresholdCurve", _comp.thresholdCurve_);
    write("edgeWidthCurve", _comp.edgeWidthCurve_);
    write("outLineColorCurve", _comp.outLineColorCurve_);
}

void OriGine::from_json(const nlohmann::json& _j, DissolveAnimation& _comp) {
    _j.at("duration").get_to(_comp.duration_);
    _j.at("isLoop").get_to(_comp.animationState_.isLoop_);
    _j.at("isPlay").get_to(_comp.animationState_.isPlay_);
    _j.at("InterpolationType").get_to(_comp.interpolationType_);

    auto read = [&_j](const char* _name, auto& _curve) {
        for (const auto& k : _j.at(_name)) {
            typename std::remove_reference<decltype(_curve)>::type::value_type key;
            k.at("time").get_to(key.time);
            k.at("value").get_to(key.value);
            _curve.push_back(key);
        }
    };

    read("thresholdCurve", _comp.thresholdCurve_);
    read("edgeWidthCurve", _comp.edgeWidthCurve_);
    read("outLineColorCurve", _comp.outLineColorCurve_);
}
