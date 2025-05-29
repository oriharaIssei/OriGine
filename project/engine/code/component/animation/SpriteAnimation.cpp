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

bool SpriteAnimation::Edit() {
    bool isEdit = false;

#ifdef _DEBUG
    isEdit = DragGuiCommand("Duration", duration_, 0.1f, 0.0f, 100.0f, "%.3f");

    ImGui::Spacing();
    ImGui::SeparatorText("Color Animation");
    isEdit |= ImGui::Checkbox("Color Animation Is Loop", &colorAnimationState_.isLoop_);
    isEdit |= ImGui::Checkbox("Color Animation Is Play", &colorAnimationState_.isPlay_);

    if (ImGui::BeginCombo("Color Interpolation Type", InterpolationTypeName[int(colorInterpolationType_)])) {
        for (int i = 0; i < (int)InterpolationType::COUNT; ++i) {
            if (ImGui::Selectable(InterpolationTypeName[i], colorInterpolationType_ == InterpolationType(i))) {
                EditorGroup::getInstance()->pushCommand(
                    std::make_unique<SetterCommand<InterpolationType>>(&colorInterpolationType_, InterpolationType(i)));
            }
        }
        ImGui::EndCombo();
    }
    isEdit |= ImGui::EditKeyFrame(
        "##Color Animation",
        colorCurve_,
        duration_,
        {1.f, 1.f, 1.f, 1.f});

    ImGui::Spacing();
    ImGui::SeparatorText("Transform Animation");
    isEdit |= ImGui::Checkbox("Transform Animation Is Loop", &transformAnimationState_.isLoop_);
    isEdit |= ImGui::Checkbox("Transform Animation Is Play", &transformAnimationState_.isPlay_);
    if (ImGui::BeginCombo("Transform Interpolation Type", InterpolationTypeName[int(transformInterpolationType_)])) {
        for (int i = 0; i < (int)InterpolationType::COUNT; ++i) {
            if (ImGui::Selectable(InterpolationTypeName[i], transformInterpolationType_ == InterpolationType(i))) {
                EditorGroup::getInstance()->pushCommand(
                    std::make_unique<SetterCommand<InterpolationType>>(&transformInterpolationType_, InterpolationType(i)));
            }
        }
        ImGui::EndCombo();
    }

    ImGui::Spacing();

    if (ImGui::BeginTable("Transform Key Frames", 2, ImGuiTableFlags_Resizable | ImGuiTableFlags_ScrollX)) {
        ImGui::TableSetupColumn("Name");
        ImGui::TableSetupColumn("Edit");
        ImGui::TableHeadersRow();
        // Scale
        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::TextUnformatted("Scale");
        ImGui::TableSetColumnIndex(1);
        isEdit |= ImGui::EditKeyFrame(
            "##Transform Animation Scale",
            scaleCurve_,
            duration_,
            {1.f, 1.f});
        // Rotate
        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::TextUnformatted("Rotate");
        ImGui::TableSetColumnIndex(1);
        isEdit |= ImGui::EditKeyFrame(
            "##Transform Animation Rotate",
            rotateCurve_,
            duration_);
        // Translate
        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::TextUnformatted("Translate");
        ImGui::TableSetColumnIndex(1);
        isEdit |= ImGui::EditKeyFrame(
            "##Transform Animation Translate",
            translateCurve_,
            duration_);
        ImGui::EndTable();
    }

    ImGui::Spacing();
    ImGui::SeparatorText("UV Animation");
    isEdit |= ImGui::Checkbox("UV Animation Is Loop", &uvAnimationState_.isLoop_);
    isEdit |= ImGui::Checkbox("UV Animation Is Play", &uvAnimationState_.isPlay_);

    if (ImGui::BeginCombo("UV Interpolation Type", InterpolationTypeName[int(uvInterpolationType_)])) {
        for (int i = 0; i < (int)InterpolationType::COUNT; ++i) {
            if (ImGui::Selectable(InterpolationTypeName[i], uvInterpolationType_ == InterpolationType(i))) {
                EditorGroup::getInstance()->pushCommand(
                    std::make_unique<SetterCommand<InterpolationType>>(&uvInterpolationType_, InterpolationType(i)));
            }
        }
        ImGui::EndCombo();
    }

    ImGui::Spacing();
    if (ImGui::BeginTable("UV Key Frames", 2, ImGuiTableFlags_Resizable | ImGuiTableFlags_ScrollX)) {
        ImGui::TableSetupColumn("Name");
        ImGui::TableSetupColumn("Edit");
        ImGui::TableHeadersRow();
        // UV Scale
        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::TextUnformatted("UV Scale");
        ImGui::TableSetColumnIndex(1);
        isEdit |= ImGui::EditKeyFrame(
            "##UV Animation Scale",
            uvScaleCurve_,
            duration_,
            {1.f, 1.f});
        // UV Rotate
        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::TextUnformatted("UV Rotate");
        ImGui::TableSetColumnIndex(1);
        isEdit |= ImGui::EditKeyFrame(
            "##UV Animation Rotate",
            uvRotateCurve_,
            duration_);
        // UV Translate
        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::TextUnformatted("UV Translate");
        ImGui::TableSetColumnIndex(1);
        isEdit |= ImGui::EditKeyFrame(
            "##UV Animation Translate",
            uvTranslateCurve_,
            duration_);
        ImGui::EndTable();
    }

#endif // _DEBUG

    return isEdit;
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
    readCurve(j["colorCurve"], r.colorCurve_);
    r.transformAnimationState_.isLoop_ = j["transformAnimationState"].value("isLoop", false);
    r.transformAnimationState_.isPlay_ = j["transformAnimationState"].value("isPlay", false);
    r.transformInterpolationType_      = InterpolationType(j.value("transformInterpolationType", int(InterpolationType::LINEAR)));
    readCurve(j["scaleCurve"], r.scaleCurve_);
    readCurve(j["rotateCurve"], r.rotateCurve_);
    readCurve(j["translateCurve"], r.translateCurve_);
    r.uvAnimationState_.isLoop_ = j["uvAnimationState"].value("isLoop", false);
    r.uvAnimationState_.isPlay_ = j["uvAnimationState"].value("isPlay", false);
    r.uvInterpolationType_      = InterpolationType(j.value("uvInterpolationType", int(InterpolationType::LINEAR)));
    readCurve(j["uvScaleCurve"], r.uvScaleCurve_);
    readCurve(j["uvRotateCurve"], r.uvRotateCurve_);
    readCurve(j["uvTranslateCurve"], r.uvTranslateCurve_);
}
