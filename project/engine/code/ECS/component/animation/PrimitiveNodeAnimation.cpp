#include "PrimitiveNodeAnimation.h"

/// engine
#include "editor/EditorController.h"
#include "editor/IEditor.h"
#include "engine/code/Engine.h"
// component
#include "component/material/Material.h"
#include "component/transform/Transform.h"

/// gui
#ifdef _DEBUG
#include "imgui/imgui.h"
#include "myGui/MyGui.h"
#include "util/timeline/Timeline.h"

namespace OriGine {

class GenerateUvAnimationCommand : public IEditCommand {
public:
    GenerateUvAnimationCommand(
        float& duration,
        AnimationCurve<Vec2f>& uvScaleCurve,
        AnimationCurve<Vec2f>& uvTranslateCurve,
        InterpolationType& uvInterpolationType,
        const Vector2f& tileSize,
        const Vector2f& textureSize,
        float& tilePerTime,
        float& startAnimationTime,
        float& animationTimeLength)
        : duration_(duration),
          uvscaleCurve_(uvScaleCurve),
          uvTranslateCurve_(uvTranslateCurve),
          uvInterpolationType_(uvInterpolationType),
          tileSize_(tileSize),
          textureSize_(textureSize),
          tilePerTime_(tilePerTime),
          startAnimationTime_(startAnimationTime),
          animationTimeLength_(animationTimeLength),
          previousDuration_(duration),
          previousUvscaleCurve_(uvScaleCurve),
          previousUvTranslateCurve_(uvTranslateCurve),
          previousUvInterpolationType_(uvInterpolationType) {}

    void Execute() override {
        // 現在の状態を保存
        previousDuration_            = duration_;
        previousUvscaleCurve_        = uvscaleCurve_;
        previousUvTranslateCurve_    = uvTranslateCurve_;
        previousUvInterpolationType_ = uvInterpolationType_;

        // 新しい状態を生成
        duration_ = animationTimeLength_;

        uvscaleCurve_.clear();
        uvTranslateCurve_.clear();

        // uvScale は Animation しない
        uvscaleCurve_.emplace_back(0.f, Vector2f(tileSize_ / textureSize_));

        // uv Translate は Animation する
        uvInterpolationType_ = InterpolationType::STEP;

        // 最大タイル数と最大時間を計算
        int32_t maxTilesX = int32_t(textureSize_[X] / tileSize_[X]);
        int32_t maxTilesY = int32_t(textureSize_[Y] / tileSize_[Y]);
        int32_t maxTiles  = maxTilesX * maxTilesY;
        float maxTime     = maxTiles * tilePerTime_;

        // startAnimationTime_ を最大時間内に収める
        startAnimationTime_ = fmod(startAnimationTime_, maxTime);

        // 初期の col と row を計算
        int32_t startTileIndex = int32_t(startAnimationTime_ / tilePerTime_);
        float col              = float(startTileIndex % maxTilesX);
        float row              = float(startTileIndex / maxTilesX);

        // UV座標を計算
        float x = col * (tileSize_[X] / textureSize_[X]);
        float y = row * (tileSize_[Y] / textureSize_[Y]);

        uvTranslateCurve_.emplace_back(0.f, Vector2f(x, y));

        int32_t tileNum = int32_t(animationTimeLength_ / tilePerTime_);
        for (int32_t i = 0; i < tileNum; i++) {
            float time = (tilePerTime_ * i);

            col += 1.f;
            if (col >= maxTilesX) {
                col = 0.f;
                row += 1.f;
            }

            // UV座標を計算
            x = col * (tileSize_[X] / textureSize_[X]);
            y = row * (tileSize_[Y] / textureSize_[Y]);
            uvTranslateCurve_.emplace_back(time, Vector2f(x, y));
        }
    }

    void Undo() override {
        // 保存した状態に戻す
        duration_            = previousDuration_;
        uvscaleCurve_        = previousUvscaleCurve_;
        uvTranslateCurve_    = previousUvTranslateCurve_;
        uvInterpolationType_ = previousUvInterpolationType_;
    }

private:
    float& duration_;
    AnimationCurve<Vec2f>& uvscaleCurve_;
    AnimationCurve<Vec2f>& uvTranslateCurve_;
    InterpolationType& uvInterpolationType_;
    const Vector2f& tileSize_;
    const Vector2f& textureSize_;
    float& tilePerTime_;
    float& startAnimationTime_;
    float& animationTimeLength_;

    // Undo用に保存する以前の状態
    float previousDuration_;
    AnimationCurve<Vec2f> previousUvscaleCurve_;
    AnimationCurve<Vec2f> previousUvTranslateCurve_;
    InterpolationType previousUvInterpolationType_;
};

#endif // _DEBUG

void PrimitiveNodeAnimation::Initialize(Entity* /*_entity*/) {
    // Initialize animation state
    currentTime_ = 0.0f;
}

void PrimitiveNodeAnimation::Edit(Scene* /*_scene*/, Entity* /*_entity*/, [[maybe_unused]] [[maybe_unused]] const std::string& _parentLabel) {
#ifdef _DEBUG

    std::string label = "duration##" + _parentLabel;
    ImGui::InputFloat(label.c_str(), &duration_);

    ImGuiTableFlags tableFlags = ImGuiTableFlags_ScrollX;

    CheckBoxCommand("Is Loop", animationState_.isLoop_);
    CheckBoxCommand("Is Play", animationState_.isPlay_);

    if (ImGui::BeginCombo("TransformAnimation InterpolationType", InterpolationTypeName[int(interpolationType_)])) {
        for (int i = 0; i < (int)InterpolationType::COUNT; ++i) {
            if (ImGui::Selectable(InterpolationTypeName[i], interpolationType_ == InterpolationType(i))) {
                OriGine::EditorController::GetInstance()->PushCommand(
                    std::make_unique<SetterCommand<InterpolationType>>(&interpolationType_, InterpolationType(i)));
            }
        }
        ImGui::EndCombo();
    }

    if (ImGui::BeginTable("TransformKeyFrames", 2, tableFlags)) {
        ImGui::TableSetupColumn("Name");
        ImGui::TableSetupColumn("Edit");
        ImGui::TableHeadersRow();

        /// =====================================================
        // scale
        // =====================================================
        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::TextUnformatted("Scale");
        ImGui::TableSetColumnIndex(1);
        ImGui::EditKeyFrame(
            "##TransformAnimation Scale",
            scaleCurve_,
            duration_);

        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::Separator();
        ImGui::TableSetColumnIndex(1);
        ImGui::Separator();

        /// =====================================================
        // rotate
        // =====================================================
        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::TextUnformatted("Rotate");
        ImGui::TableSetColumnIndex(1);
        ImGui::EditKeyFrame(
            "##TransformAnimation Rotate",
            rotateCurve_,
            duration_);

        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::Separator();
        ImGui::TableSetColumnIndex(1);
        ImGui::Separator();

        /// =====================================================
        // transform
        // =====================================================
        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::TextUnformatted("Transform");
        ImGui::TableSetColumnIndex(1);
        ImGui::EditKeyFrame(
            "##TransformAnimation Transform",
            translateCurve_,
            duration_);

        ImGui::EndTable();
    }

#endif // _DEBUG
}

void PrimitiveNodeAnimation::Finalize() {
    animationState_.isLoop_         = false;
    animationState_.isPlay_         = false;
    animationState_.isEnd_          = false;
    currentTime_                    = 0.0f;

    scaleCurve_.clear();
    rotateCurve_.clear();
    translateCurve_.clear();
}

void PrimitiveNodeAnimation::Update(float _deltaTime, Transform* _transform) {
    animationState_.isEnd_ = false;

    currentTime_ += _deltaTime;
    if (currentTime_ >= duration_) {
        if (animationState_.isLoop_) {
            currentTime_ = 0.0f;
        } else {
            animationState_.isEnd_  = true;
            animationState_.isPlay_ = false;
        }
    }

    UpdateTransformAnimation(_transform);
}

void PrimitiveNodeAnimation::PlayStart() {
    currentTime_ = 0.f;

    animationState_.isEnd_  = false;
    animationState_.isPlay_ = true;
}

void PrimitiveNodeAnimation::Stop() {
    currentTime_                    = 0.f;
    animationState_.isEnd_          = true;
    animationState_.isPlay_         = false;
}

void PrimitiveNodeAnimation::UpdateTransformAnimation(Transform* _transform) {
    if (!animationState_.isLoop_) {
        if (!animationState_.isPlay_) {
            return;
        }
    }

    animationState_.isEnd_ = false;

    switch (interpolationType_) {
    case InterpolationType::LINEAR:
        // scale
        _transform->scale = CalculateValue::Linear(scaleCurve_, currentTime_);
        // rotate
        _transform->rotate = CalculateValue::Linear(rotateCurve_, currentTime_);
        // translate
        _transform->translate = CalculateValue::Linear(translateCurve_, currentTime_);
        break;
    case InterpolationType::STEP:
        // scale
        _transform->scale = CalculateValue::Step(scaleCurve_, currentTime_);
        // rotate
        _transform->rotate = CalculateValue::Step(rotateCurve_, currentTime_);
        // translate
        _transform->translate = CalculateValue::Step(translateCurve_, currentTime_);
        break;
    default:
        break;
    }
    _transform->UpdateMatrix();
}
void to_json(nlohmann::json& _json, const PrimitiveNodeAnimation& _primitiveNodeAnimation) {
    _json["duration"] = _primitiveNodeAnimation.duration_;
    _json["isLoop"]   = _primitiveNodeAnimation.animationState_.isLoop_;
    _json["isPlay"]   = _primitiveNodeAnimation.animationState_.isPlay_;

    auto writeCurve = [&_json](const std::string& _name, const auto& _curve) {
        nlohmann::json curveJson = nlohmann::json::array();
        for (const auto& key : _curve) {
            nlohmann::json keyJson;
            keyJson["time"]  = key.time;
            keyJson["value"] = key.value;
            curveJson.push_back(keyJson);
        }
        _json[_name] = curveJson;
    };

    _json["transformInterpolationType"] = _primitiveNodeAnimation.interpolationType_;
    writeCurve("scaleCurve", _primitiveNodeAnimation.scaleCurve_);
    writeCurve("rotateCurve", _primitiveNodeAnimation.rotateCurve_);
    writeCurve("translateCurve", _primitiveNodeAnimation.translateCurve_);
}
void from_json(const nlohmann::json& _json, PrimitiveNodeAnimation& _primitiveNodeAnimation) {
    _json.at("duration").get_to(_primitiveNodeAnimation.duration_);
    _json.at("isLoop").get_to(_primitiveNodeAnimation.animationState_.isLoop_);
    _json.at("isPlay").get_to(_primitiveNodeAnimation.animationState_.isPlay_);

    auto readCurve = [&_json](const std::string& _name, auto& _curve) {
        for (const auto& keyJson : _json.at(_name)) {
            typename std::remove_reference<decltype(_curve)>::type::value_type key;
            keyJson.at("time").get_to(key.time);
            keyJson.at("value").get_to(key.value);
            _curve.push_back(key);
        }
    };
    _json.at("transformInterpolationType").get_to(_primitiveNodeAnimation.interpolationType_);
    readCurve("scaleCurve", _primitiveNodeAnimation.scaleCurve_);
    readCurve("rotateCurve", _primitiveNodeAnimation.rotateCurve_);
    readCurve("translateCurve", _primitiveNodeAnimation.translateCurve_);
}

} // namespace OriGine
