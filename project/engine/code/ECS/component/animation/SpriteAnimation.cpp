#include "SpriteAnimation.h"

/// component
#include "component/renderer/Sprite.h"

#ifdef _DEBUG
#include "myGui/MyGui.h"
#include "util/timeline/Timeline.h"
#endif // _DEBUG

SpriteAnimation::SpriteAnimation() {
}

SpriteAnimation::~SpriteAnimation() {
}

void SpriteAnimation::Initialize(GameEntity* /*_hostEntity*/) {
    // 初期化
    currentTime_ = 0.0f;
}

void SpriteAnimation::Edit(Scene* /*_scene*/, GameEntity* /* _entity*/, [[maybe_unused]] [[maybe_unused]] const std::string& _parentLabel) {
#ifdef _DEBUG
    std::string label = "Duration##" + _parentLabel;
    DragGuiCommand(label, duration_, 0.1f, 0.0f, 100.0f, "%.3f");

    ImGui::Spacing();
    ImGui::SeparatorText("Color Animation");
    label = "Color Animation Is Play##" + _parentLabel;
    CheckBoxCommand(label, colorAnimationState_.isPlay_);
    label = "Color Animation Is Loop##" + _parentLabel;
    CheckBoxCommand(label, colorAnimationState_.isLoop_);

    label = "Color Interpolation Type##" + _parentLabel;
    if (ImGui::BeginCombo(label.c_str(), InterpolationTypeName[int(colorInterpolationType_)])) {
        for (int i = 0; i < (int)InterpolationType::COUNT; ++i) {
            if (ImGui::Selectable(InterpolationTypeName[i], colorInterpolationType_ == InterpolationType(i))) {
                EditorController::getInstance()->pushCommand(
                    std::make_unique<SetterCommand<InterpolationType>>(&colorInterpolationType_, InterpolationType(i)));
            }
        }
        ImGui::EndCombo();
    }
    ImGui::Text("Color Animation KeyFrames");
    label = "Color Animation ##" + _parentLabel;
    ImGui::EditColorKeyFrame(
        label,
        colorCurve_,
        duration_,
        {1.f, 1.f, 1.f, 1.f});

    ImGui::Spacing();
    ImGui::SeparatorText("Transform Animation");
    label = "Transform Animation Is Play##" + _parentLabel;
    CheckBoxCommand(label, transformAnimationState_.isPlay_);
    label = "Transform Animation Is Loop##" + _parentLabel;
    CheckBoxCommand(label, transformAnimationState_.isLoop_);

    label = "Transform Interpolation Type##" + _parentLabel;
    if (ImGui::BeginCombo(label.c_str(), InterpolationTypeName[int(transformInterpolationType_)])) {
        for (int i = 0; i < (int)InterpolationType::COUNT; ++i) {
            if (ImGui::Selectable(InterpolationTypeName[i], transformInterpolationType_ == InterpolationType(i))) {
                EditorController::getInstance()->pushCommand(
                    std::make_unique<SetterCommand<InterpolationType>>(&transformInterpolationType_, InterpolationType(i)));
            }
        }
        ImGui::EndCombo();
    }

    ImGui::Spacing();

    // Scale
    ImGui::Text("Scale");
    label = "##Transform Animation Scale" + _parentLabel;
    ImGui::EditKeyFrame(
        label,
        scaleCurve_,
        duration_,
        {1.f, 1.f});

    // Rotate
    ImGui::Text("Rotate");
    label = "##Transform Animation Rotate" + _parentLabel;
    ImGui::EditKeyFrame(
        label,
        rotateCurve_,
        duration_);

    // Translate
    label = "##Transform Animation Translate" + _parentLabel;
    ImGui::Text("Translate");
    ImGui::EditKeyFrame(
        label,
        translateCurve_,
        duration_);

    ImGui::Spacing();
    ImGui::SeparatorText("UV Animation");
    label = "UV Animation Is Play##" + _parentLabel;
    CheckBoxCommand(label, uvAnimationState_.isPlay_);
    label = "UV Animation Is Loop##" + _parentLabel;
    CheckBoxCommand(label, uvAnimationState_.isLoop_);

    label = "UV Interpolation Type##" + _parentLabel;
    if (ImGui::BeginCombo(label.c_str(), InterpolationTypeName[int(uvInterpolationType_)])) {
        for (int i = 0; i < (int)InterpolationType::COUNT; ++i) {
            if (ImGui::Selectable(InterpolationTypeName[i], uvInterpolationType_ == InterpolationType(i))) {
                EditorController::getInstance()->pushCommand(
                    std::make_unique<SetterCommand<InterpolationType>>(&uvInterpolationType_, InterpolationType(i)));
            }
        }
        ImGui::EndCombo();
    }

    ImGui::Spacing();
    // UV Scale
    ImGui::Text("UV Scale");
    label = "##UV Animation Scale" + _parentLabel;
    ImGui::EditKeyFrame(
        label,
        uvScaleCurve_,
        duration_,
        {1.f, 1.f});

    // UV Rotate
    ImGui::Text("UV Rotate");
    label = "##UV Animation Rotate" + _parentLabel;
    ImGui::EditKeyFrame(
        label,
        uvRotateCurve_,
        duration_);
    // UV Translate
    ImGui::Text("UV Translate");
    label = "##UV Animation Translate" + _parentLabel;
    ImGui::EditKeyFrame(
        label,
        uvTranslateCurve_,
        duration_);
#endif // _DEBUG
}

void SpriteAnimation::Finalize() {
}

void SpriteAnimation::UpdateSpriteAnimation(float _deltaTime, SpriteRenderer* _spriteRenderer) {
    currentTime_ += _deltaTime;

    if (currentTime_ >= duration_) {
        if (colorAnimationState_.isLoop_) {
            currentTime_ = 0.0f;
        } else {
            colorAnimationState_.isEnd_ = true;
        }
        if (transformAnimationState_.isLoop_) {
            currentTime_ = 0.0f;
        } else {
            transformAnimationState_.isEnd_ = true;
        }
        if (uvAnimationState_.isLoop_) {
            currentTime_ = 0.0f;
        } else {
            uvAnimationState_.isEnd_ = true;
        }
    }

    // Update color animation
    if (colorAnimationState_.isPlay_) {
        switch (colorInterpolationType_) {
        case InterpolationType::LINEAR:
            _spriteRenderer->setColor(CalculateValue::Linear(colorCurve_, currentTime_));
            break;
        case InterpolationType::STEP:
            _spriteRenderer->setColor(CalculateValue::Step(colorCurve_, currentTime_));
            break;
        default:
            break;
        }
    }

    // Update transform animation
    if (transformAnimationState_.isPlay_) {
        switch (transformInterpolationType_) {
        case InterpolationType::LINEAR:
            _spriteRenderer->setScale(CalculateValue::Linear(scaleCurve_, currentTime_));
            _spriteRenderer->setRotate(CalculateValue::Linear(rotateCurve_, currentTime_));
            _spriteRenderer->setTranslate(CalculateValue::Linear(translateCurve_, currentTime_));
            break;
        case InterpolationType::STEP:
            _spriteRenderer->setScale(CalculateValue::Step(scaleCurve_, currentTime_));
            _spriteRenderer->setRotate(CalculateValue::Step(rotateCurve_, currentTime_));
            _spriteRenderer->setTranslate(CalculateValue::Step(translateCurve_, currentTime_));
            break;
        default:
            break;
        }
    }

    // Update UV animation
    if (uvAnimationState_.isPlay_) {
        switch (uvInterpolationType_) {
        case InterpolationType::LINEAR:
            _spriteRenderer->setUVScale(CalculateValue::Linear(uvScaleCurve_, currentTime_));
            _spriteRenderer->setUVRotate(CalculateValue::Linear(uvRotateCurve_, currentTime_));
            _spriteRenderer->setUVTranslate(CalculateValue::Linear(uvTranslateCurve_, currentTime_));
            break;
        case InterpolationType::STEP:
            _spriteRenderer->setUVScale(CalculateValue::Step(uvScaleCurve_, currentTime_));
            _spriteRenderer->setUVRotate(CalculateValue::Step(uvRotateCurve_, currentTime_));
            _spriteRenderer->setUVTranslate(CalculateValue::Step(uvTranslateCurve_, currentTime_));
            break;
        default:
            break;
        }
    }
}

void SpriteAnimation::PlayStart() {
    // Reset current time
    currentTime_ = 0.0f;
    // Reset animation states
    colorAnimationState_.isEnd_      = false;
    colorAnimationState_.isPlay_     = true;
    transformAnimationState_.isEnd_  = false;
    transformAnimationState_.isPlay_ = true;
    uvAnimationState_.isEnd_         = false;
    uvAnimationState_.isPlay_        = true;
}

void SpriteAnimation::Stop() {
    // Stop all animations
    colorAnimationState_.isEnd_      = true;
    colorAnimationState_.isPlay_     = false;
    transformAnimationState_.isEnd_  = true;
    transformAnimationState_.isPlay_ = false;
    uvAnimationState_.isEnd_         = true;
    uvAnimationState_.isPlay_        = false;
}

void to_json(nlohmann::json& j, const SpriteAnimation& r) {
    auto writeCurve = [&j](const std::string& _name, const auto& _curve) {
        nlohmann::json curveJson = nlohmann::json::array();
        for (const auto& key : _curve) {
            nlohmann::json keyJson;
            keyJson["time"]  = key.time;
            keyJson["value"] = key.value;
            curveJson.push_back(keyJson);
        }
        j[_name] = curveJson;
    };

    j["duration"] = r.duration_;

    j["colorAnimationState"]["isLoop"] = r.colorAnimationState_.isLoop_;
    j["colorAnimationState"]["isPlay"] = r.colorAnimationState_.isPlay_;
    j["colorInterpolationType"]        = int(r.colorInterpolationType_);
    writeCurve("colorCurve", r.colorCurve_);

    j["transformAnimationState"]["isLoop"] = r.transformAnimationState_.isLoop_;
    j["transformAnimationState"]["isPlay"] = r.transformAnimationState_.isPlay_;
    j["transformInterpolationType"]        = int(r.transformInterpolationType_);
    writeCurve("scaleCurve", r.scaleCurve_);
    writeCurve("rotateCurve", r.rotateCurve_);
    writeCurve("translateCurve", r.translateCurve_);

    j["uvAnimationState"]["isLoop"] = r.uvAnimationState_.isLoop_;
    j["uvAnimationState"]["isPlay"] = r.uvAnimationState_.isPlay_;
    j["uvInterpolationType"]        = int(r.uvInterpolationType_);
    writeCurve("uvScaleCurve", r.uvScaleCurve_);
    writeCurve("uvRotateCurve", r.uvRotateCurve_);
    writeCurve("uvTranslateCurve", r.uvTranslateCurve_);
}

void from_json(const nlohmann::json& j, SpriteAnimation& r) {

    auto readCurve = [&j](const std::string& _name, auto& _curve) {
        for (const auto& keyJson : j.at(_name)) {
            typename std::remove_reference<decltype(_curve)>::type::value_type key;
            keyJson.at("time").get_to(key.time);
            keyJson.at("value").get_to(key.value);
            _curve.push_back(key);
        }
    };

    r.duration_                    = j.value("duration", 0.0f);
    r.colorAnimationState_.isLoop_ = j["colorAnimationState"].value("isLoop", false);
    r.colorAnimationState_.isPlay_ = j["colorAnimationState"].value("isPlay", false);
    r.colorInterpolationType_      = InterpolationType(j.value("colorInterpolationType", int(InterpolationType::LINEAR)));
    readCurve("colorCurve", r.colorCurve_);
    r.transformAnimationState_.isLoop_ = j["transformAnimationState"].value("isLoop", false);
    r.transformAnimationState_.isPlay_ = j["transformAnimationState"].value("isPlay", false);
    r.transformInterpolationType_      = InterpolationType(j.value("transformInterpolationType", int(InterpolationType::LINEAR)));
    readCurve("scaleCurve", r.scaleCurve_);
    readCurve("rotateCurve", r.rotateCurve_);
    readCurve("translateCurve", r.translateCurve_);
    r.uvAnimationState_.isLoop_ = j["uvAnimationState"].value("isLoop", false);
    r.uvAnimationState_.isPlay_ = j["uvAnimationState"].value("isPlay", false);
    r.uvInterpolationType_      = InterpolationType(j.value("uvInterpolationType", int(InterpolationType::LINEAR)));
    readCurve("uvScaleCurve", r.uvScaleCurve_);
    readCurve("uvRotateCurve", r.uvRotateCurve_);
    readCurve("uvTranslateCurve", r.uvTranslateCurve_);
}
