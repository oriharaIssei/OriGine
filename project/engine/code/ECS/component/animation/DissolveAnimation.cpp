#include "DissolveAnimation.h"

#ifdef _DEBUG
#include "myGui/MyGui.h"
#include "timeline/Timeline.h"
#endif // _DEBUG

using namespace OriGine;

DissolveAnimation::DissolveAnimation() {}
DissolveAnimation::~DissolveAnimation() {}

void DissolveAnimation::Initialize(Scene*, EntityHandle) {
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

void OriGine::to_json(nlohmann::json& j, const DissolveAnimation& a) {
    j["duration"]          = a.duration_;
    j["isLoop"]            = a.animationState_.isLoop_;
    j["isPlay"]            = a.animationState_.isPlay_;
    j["InterpolationType"] = a.interpolationType_;

    auto write = [&j](const char* name, const auto& curve) {
        nlohmann::json arr = nlohmann::json::array();
        for (const auto& k : curve) {
            arr.push_back({{"time", k.time}, {"value", k.value}});
        }
        j[name] = arr;
    };

    write("thresholdCurve", a.thresholdCurve_);
    write("edgeWidthCurve", a.edgeWidthCurve_);
    write("outLineColorCurve", a.outLineColorCurve_);
}

void OriGine::from_json(const nlohmann::json& j, DissolveAnimation& a) {
    j.at("duration").get_to(a.duration_);
    j.at("isLoop").get_to(a.animationState_.isLoop_);
    j.at("isPlay").get_to(a.animationState_.isPlay_);
    j.at("InterpolationType").get_to(a.interpolationType_);

    auto read = [&j](const char* name, auto& curve) {
        for (const auto& k : j.at(name)) {
            typename std::remove_reference<decltype(curve)>::type::value_type key;
            k.at("time").get_to(key.time);
            k.at("value").get_to(key.value);
            curve.push_back(key);
        }
    };

    read("thresholdCurve", a.thresholdCurve_);
    read("edgeWidthCurve", a.edgeWidthCurve_);
    read("outLineColorCurve", a.outLineColorCurve_);
}
