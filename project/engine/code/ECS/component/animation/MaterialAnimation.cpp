#include "MaterialAnimation.h"

/// engine
#include "editor/EditorController.h"
#include "editor/IEditor.h"
#include "engine/code/Engine.h"
#include "scene/Scene.h"
// component
#include "component/material/Material.h"
#include "component/transform/Transform.h"

using namespace OriGine;

/// gui
#ifdef _DEBUG
#include "imgui/imgui.h"
#include "myGui/MyGui.h"
#include "util/timeline/Timeline.h"

class GenerateUvAnimationCommand : public IEditCommand {
public:
    GenerateUvAnimationCommand(
        float& _duration,
        AnimationCurve<Vec2f>& _uvScaleCurve,
        AnimationCurve<Vec2f>& _uvTranslateCurve,
        InterpolationType& _uvInterpolationType,
        const Vector2f& _tileSize,
        const Vector2f& _textureSize,
        float& _tilePerTime,
        float& _startAnimationTime,
        float& _animationTimeLength)
        : duration_(_duration),
          uvscaleCurve_(_uvScaleCurve),
          uvTranslateCurve_(_uvTranslateCurve),
          interpolationType_(_uvInterpolationType),
          tileSize_(_tileSize),
          textureSize_(_textureSize),
          tilePerTime_(_tilePerTime),
          startAnimationTime_(_startAnimationTime),
          animationTimeLength_(_animationTimeLength),
          previousDuration_(_duration),
          previousUvscaleCurve_(_uvScaleCurve),
          previousUvTranslateCurve_(_uvTranslateCurve),
          previousinterpolationType_(_uvInterpolationType) {}

    void Execute() override {
        // 現在の状態を保存
        previousDuration_          = duration_;
        previousUvscaleCurve_      = uvscaleCurve_;
        previousUvTranslateCurve_  = uvTranslateCurve_;
        previousinterpolationType_ = interpolationType_;

        // 新しい状態を生成
        duration_ = animationTimeLength_;

        uvscaleCurve_.clear();
        uvTranslateCurve_.clear();

        // uvScale は Animation しない
        uvscaleCurve_.emplace_back(0.f, Vector2f(tileSize_ / textureSize_));

        // uv Translate は Animation する
        interpolationType_ = InterpolationType::STEP;

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
        duration_          = previousDuration_;
        uvscaleCurve_      = previousUvscaleCurve_;
        uvTranslateCurve_  = previousUvTranslateCurve_;
        interpolationType_ = previousinterpolationType_;
    }

private:
    float& duration_;
    AnimationCurve<Vec2f>& uvscaleCurve_;
    AnimationCurve<Vec2f>& uvTranslateCurve_;
    InterpolationType& interpolationType_;
    const Vector2f& tileSize_;
    const Vector2f& textureSize_;
    float& tilePerTime_;
    float& startAnimationTime_;
    float& animationTimeLength_;

    // Undo用に保存する以前の状態
    float previousDuration_;
    AnimationCurve<Vec2f> previousUvscaleCurve_;
    AnimationCurve<Vec2f> previousUvTranslateCurve_;
    InterpolationType previousinterpolationType_;
};

#endif // _DEBUG

void MaterialAnimation::Initialize(Scene* /*_scene*/, EntityHandle /*_entity*/) { // Initialize animation state
    currentTime_ = 0.0f;
}

void MaterialAnimation::Edit([[maybe_unused]] Scene* _scene, [[maybe_unused]] EntityHandle _handle, [[maybe_unused]] [[maybe_unused]] const std::string& _parentLabel) {
#ifdef _DEBUG

    std::string label = "Duration##" + _parentLabel;
    DragGuiCommand(label, duration_);

    label = "LocalTime##" + _parentLabel;
    SlideGuiCommand("LocalTime", currentTime_, 0.f, duration_);

    ImGui::Spacing();

    label                      = "MaterialIndex##" + _parentLabel;
    auto& materials            = _scene->GetComponents<Material>(_handle);
    int32_t entityMaterialSize = static_cast<int32_t>(materials.size());

    InputGuiCommand(label, materialIndex_);
    materialIndex_ = std::clamp(materialIndex_, -1, entityMaterialSize);

    ImGui::Spacing();

    CheckBoxCommand("Is Loop##" + _parentLabel, animationState_.isLoop_);
    CheckBoxCommand("Is Play##" + _parentLabel, animationState_.isPlay_);

    CheckBoxCommand("Is Debug Play##" + _parentLabel, isDebugPlay_);
    ImGui::Spacing();

    label = "InterpolationType##" + _parentLabel;
    if (ImGui::BeginCombo(label.c_str(), InterpolationTypeName[int(interpolationType_)])) {
        for (int i = 0; i < (int)InterpolationType::COUNT; ++i) {
            if (ImGui::Selectable(InterpolationTypeName[i], interpolationType_ == InterpolationType(i))) {
                OriGine::EditorController::GetInstance()->PushCommand(
                    std::make_unique<SetterCommand<InterpolationType>>(&interpolationType_, InterpolationType(i)));
            }
        }
        ImGui::EndCombo();
    }

    ImGui::Spacing();

    label = "UvCurveGenerator Form TextureAnimation##" + _parentLabel;
    if (ImGui::TreeNode(label.c_str())) {
        DragGuiVectorCommand<2, float>("TileSize", tileSize_, 0.1f);
        DragGuiVectorCommand<2, float>("TextureSize", textureSize_, 0.1f);
        DragGuiCommand<float>("tilePerTime_", tilePerTime_);
        DragGuiCommand<float>("StartAnimationTime", startAnimationTime_, 0.1f, 0);
        DragGuiCommand<float>("AnimationTimeLength", animationTimeLength_, 0.1f, 0);

        if (ImGui::Button("Generate Curve")) {
            OriGine::EditorController::GetInstance()->PushCommand(
                std::make_unique<GenerateUvAnimationCommand>(
                    duration_,
                    uvscaleCurve_,
                    uvTranslateCurve_,
                    interpolationType_,
                    tileSize_,
                    textureSize_,
                    tilePerTime_,
                    startAnimationTime_,
                    animationTimeLength_));
        }
        ImGui::TreePop();
    }

    label = "Material Animation##" + _parentLabel;

    ImGuiTableFlags tableFlags = ImGuiTableFlags_ScrollX;
    if (ImGui::TreeNode(label.c_str())) {
        label = "MaterialKeyFrames##" + _parentLabel;
        if (ImGui::BeginTable(label.c_str(), 2, tableFlags)) {
            ImGui::TableSetupColumn("Name");
            ImGui::TableSetupColumn("Edit");
            ImGui::TableHeadersRow();
            /// =====================================================
            // color
            // =====================================================
            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            ImGui::TextUnformatted("Color");
            ImGui::TableSetColumnIndex(1);
            ImGui::EditColorKeyFrame(
                " Color" + _parentLabel,
                colorCurve_,
                duration_);
            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            ImGui::Separator();
            ImGui::TableSetColumnIndex(1);
            ImGui::Separator();
            /// =====================================================
            // uv Scale
            // =====================================================
            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            ImGui::TextUnformatted("UV Scale");
            ImGui::TableSetColumnIndex(1);
            ImGui::EditKeyFrame(
                " UV Scale" + _parentLabel,
                uvscaleCurve_,
                duration_,
                Vec2f(1.f, 1.f));

            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            ImGui::Separator();
            ImGui::TableSetColumnIndex(1);
            ImGui::Separator();

            /// =====================================================
            // uv Rotate
            // =====================================================
            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            ImGui::TextUnformatted("UV Rotate");
            ImGui::TableSetColumnIndex(1);
            ImGui::EditKeyFrame(
                " UV Rotate" + _parentLabel,
                uvRotateCurve_,
                duration_);
            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            ImGui::Separator();
            ImGui::TableSetColumnIndex(1);
            ImGui::Separator();

            /// =====================================================
            // uv Translate
            // =====================================================
            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            ImGui::TextUnformatted("UV Translate");
            ImGui::TableSetColumnIndex(1);

            ImGui::EditKeyFrame(
                "##MaterialAnimation UV Translate##" + _parentLabel,
                uvTranslateCurve_,
                duration_);

            ImGui::EndTable();
        }
        ImGui::TreePop();
    }
#endif // _DEBUG
}

void MaterialAnimation::Finalize() {
    animationState_.isLoop_ = false;
    animationState_.isPlay_ = false;
    animationState_.isEnd_  = false;
    currentTime_            = 0.0f;

    uvscaleCurve_.clear();
    uvRotateCurve_.clear();
    uvTranslateCurve_.clear();
}

void MaterialAnimation::Update(float _deltaTime, Material* _material) {
    animationState_.isEnd_ = false;

    if (!GetAnimationIsPlay()) {
        return;
    }

    currentTime_ += _deltaTime;

    if (currentTime_ >= duration_) {
        if (animationState_.isLoop_) {
            currentTime_ = 0.0f;
        } else {
            EndAnimation();
        }
    }

    UpdateMaterialAnimation(_material);
}

void MaterialAnimation::PlayStart() {
    currentTime_ = 0.f;

    animationState_.isEnd_  = false;
    animationState_.isPlay_ = true;
}

void MaterialAnimation::Stop() {
    animationState_.isEnd_  = true;
    animationState_.isPlay_ = false;
}

void MaterialAnimation::RescaleDuration(float _newDuration) {
    auto rescaleCurve = [_newDuration, this](auto& _curve) {
        for (auto& key : _curve) {
            key.time = (key.time / duration_) * _newDuration;
        }
    };
    rescaleCurve(colorCurve_);
    rescaleCurve(uvscaleCurve_);
    rescaleCurve(uvRotateCurve_);
    rescaleCurve(uvTranslateCurve_);
    duration_ = _newDuration;
}

void MaterialAnimation::UpdateMaterialAnimation(Material* _material) {
    if (!_material) {
        return;
    }
    if (animationState_.isLoop_) {
        if (!GetAnimationIsPlay()) {
            return;
        }
    }
    animationState_.isEnd_ = false;

    switch (interpolationType_) {
    case InterpolationType::LINEAR:
        if (!colorCurve_.empty()) {
            _material->color_ = CalculateValue::Linear(colorCurve_, currentTime_);
        }
        if (!uvscaleCurve_.empty()) {
            _material->uvTransform_.scale_ = CalculateValue::Linear(uvscaleCurve_, currentTime_);
        }
        if (!uvRotateCurve_.empty()) {
            _material->uvTransform_.rotate_ = CalculateValue::Linear(uvRotateCurve_, currentTime_);
        }
        if (!uvTranslateCurve_.empty()) {
            _material->uvTransform_.translate_ = CalculateValue::Linear(uvTranslateCurve_, currentTime_);
        }

        break;
    case InterpolationType::STEP:
        if (!colorCurve_.empty()) {
            _material->color_ = CalculateValue::Step(colorCurve_, currentTime_);
        }
        if (!uvscaleCurve_.empty()) {
            _material->uvTransform_.scale_ = CalculateValue::Step(uvscaleCurve_, currentTime_);
        }
        if (!uvRotateCurve_.empty()) {
            _material->uvTransform_.rotate_ = CalculateValue::Step(uvRotateCurve_, currentTime_);
        }
        if (!uvTranslateCurve_.empty()) {
            _material->uvTransform_.translate_ = CalculateValue::Step(uvTranslateCurve_, currentTime_);
        }
        break;
    default:
        break;
    }

    _material->UpdateUvMatrix();
}

void OriGine::to_json(nlohmann::json& _json, const MaterialAnimation& _animation) {
    _json["duration"] = _animation.duration_;
    _json["isLoop"]   = _animation.animationState_.isLoop_;
    _json["isPlay"]   = _animation.animationState_.isPlay_;
    auto writeCurve   = [&_json](const std::string& _name, const auto& _curve) {
        nlohmann::json curveJson = nlohmann::json::array();
        for (const auto& key : _curve) {
            nlohmann::json keyJson;
            keyJson["time"]  = key.time;
            keyJson["value"] = key.value;
            curveJson.push_back(keyJson);
        }
        _json[_name] = curveJson;
    };

    _json["InterpolationType"] = _animation.interpolationType_;
    writeCurve("colorCurve", _animation.colorCurve_);
    writeCurve("uvScaleCurve", _animation.uvscaleCurve_);
    writeCurve("uvRotateCurve", _animation.uvRotateCurve_);
    writeCurve("uvTranslateCurve", _animation.uvTranslateCurve_);
}
void OriGine::from_json(const nlohmann::json& _json, MaterialAnimation& _animation) {
    _json.at("duration").get_to(_animation.duration_);
    _json.at("isLoop").get_to(_animation.animationState_.isLoop_);
    _json.at("isPlay").get_to(_animation.animationState_.isPlay_);

    auto readCurve = [&_json](const std::string& _name, auto& _curve) {
        for (const auto& keyJson : _json.at(_name)) {
            typename std::remove_reference<decltype(_curve)>::type::value_type key;
            keyJson.at("time").get_to(key.time);
            keyJson.at("value").get_to(key.value);
            _curve.push_back(key);
        }
    };
    _json.at("InterpolationType").get_to(_animation.interpolationType_);

    readCurve("colorCurve", _animation.colorCurve_);

    readCurve("uvScaleCurve", _animation.uvscaleCurve_);
    readCurve("uvRotateCurve", _animation.uvRotateCurve_);
    readCurve("uvTranslateCurve", _animation.uvTranslateCurve_);
}
