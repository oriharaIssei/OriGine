#include "PrimitiveNodeAnimation.h"

/// engine
#include "engine/code/Engine.h"
#include "editor/EditorController.h"
#include "editor/IEditor.h"
// component
#include "component/material/Material.h"
#include "component/transform/Transform.h"

/// gui
#ifdef _DEBUG
#include "imgui/imgui.h"
#include "myGui/MyGui.h"
#include "util/timeline/Timeline.h"
#endif // _DEBUG

void PrimitiveNodeAnimation::Initialize(GameEntity* /*_entity*/) {
    // Initialize animation state
    currentTime_ = 0.0f;
}

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
          uvScaleCurve_(uvScaleCurve),
          uvTranslateCurve_(uvTranslateCurve),
          uvInterpolationType_(uvInterpolationType),
          tileSize_(tileSize),
          textureSize_(textureSize),
          tilePerTime_(tilePerTime),
          startAnimationTime_(startAnimationTime),
          animationTimeLength_(animationTimeLength),
          previousDuration_(duration),
          previousUvScaleCurve_(uvScaleCurve),
          previousUvTranslateCurve_(uvTranslateCurve),
          previousUvInterpolationType_(uvInterpolationType) {}

    void Execute() override {
        // 現在の状態を保存
        previousDuration_            = duration_;
        previousUvScaleCurve_        = uvScaleCurve_;
        previousUvTranslateCurve_    = uvTranslateCurve_;
        previousUvInterpolationType_ = uvInterpolationType_;

        // 新しい状態を生成
        duration_ = animationTimeLength_;

        uvScaleCurve_.clear();
        uvTranslateCurve_.clear();

        // uvScale は Animation しない
        uvScaleCurve_.emplace_back(0.f, Vector2f(tileSize_ / textureSize_));

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
        uvScaleCurve_        = previousUvScaleCurve_;
        uvTranslateCurve_    = previousUvTranslateCurve_;
        uvInterpolationType_ = previousUvInterpolationType_;
    }

private:
    float& duration_;
    AnimationCurve<Vec2f>& uvScaleCurve_;
    AnimationCurve<Vec2f>& uvTranslateCurve_;
    InterpolationType& uvInterpolationType_;
    const Vector2f& tileSize_;
    const Vector2f& textureSize_;
    float& tilePerTime_;
    float& startAnimationTime_;
    float& animationTimeLength_;

    // Undo用に保存する以前の状態
    float previousDuration_;
    AnimationCurve<Vec2f> previousUvScaleCurve_;
    AnimationCurve<Vec2f> previousUvTranslateCurve_;
    InterpolationType previousUvInterpolationType_;
};

bool PrimitiveNodeAnimation::Edit() {
#ifdef _DEBUG
    bool isChange = false;

    isChange |= ImGui::InputFloat("duration", &duration_);

    ImGuiTableFlags tableFlags = ImGuiTableFlags_Resizable | ImGuiTableFlags_ScrollX;
    if (ImGui::TreeNode("Transform_Animation")) {
        isChange |= CheckBoxCommand("TransformAnimation Is Loop", transformAnimationState_.isLoop_);
        isChange |= CheckBoxCommand("TransformAnimation Is Play", transformAnimationState_.isPlay_);

        if (ImGui::BeginCombo("TransformAnimation InterpolationType", InterpolationTypeName[int(transformInterpolationType_)])) {
            for (int i = 0; i < (int)InterpolationType::COUNT; ++i) {
                if (ImGui::Selectable(InterpolationTypeName[i], transformInterpolationType_ == InterpolationType(i))) {
                    EditorController::getInstance()->pushCommand(
                        std::make_unique<SetterCommand<InterpolationType>>(&transformInterpolationType_, InterpolationType(i)));
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
            isChange |= ImGui::EditKeyFrame(
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
            isChange |= ImGui::EditKeyFrame(
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
            isChange |= ImGui::EditKeyFrame(
                "##TransformAnimation Transform",
                translateCurve_,
                duration_);

            ImGui::EndTable();
        }
        ImGui::TreePop();
    }

    if (ImGui::TreeNode("UvCurveGenerator Form TextureAnimation")) {
        DragGuiVectorCommand<2, float>("TileSize", tileSize_, 0.1f);
        DragGuiVectorCommand<2, float>("TextureSize", textureSize_, 0.1f);
        DragGuiCommand<float>("tilePerTime_", tilePerTime_);
        DragGuiCommand<float>("StartAnimationTime", startAnimationTime_, 0.1f, 0);
        DragGuiCommand<float>("AnimationTimeLength", animationTimeLength_, 0.1f, 0);

        if (ImGui::Button("Generate Curve")) {
            EditorController::getInstance()->pushCommand(
                std::make_unique<GenerateUvAnimationCommand>(
                    duration_,
                    uvScaleCurve_,
                    uvTranslateCurve_,
                    uvInterpolationType_,
                    tileSize_,
                    textureSize_,
                    tilePerTime_,
                    startAnimationTime_,
                    animationTimeLength_));
        }
        ImGui::TreePop();
    }

    if (ImGui::TreeNode("Material_Animation")) {
        isChange |= ImGui::Checkbox("MaterialAnimation Is Loop", &materialAnimationState_.isLoop_);
        isChange |= ImGui::Checkbox("MaterialAnimation Is Play", &materialAnimationState_.isPlay_);

        if (ImGui::BeginCombo("MaterialAnimation InterpolationType", InterpolationTypeName[int(uvInterpolationType_)])) {
            for (int i = 0; i < (int)InterpolationType::COUNT; ++i) {
                if (ImGui::Selectable(InterpolationTypeName[i], uvInterpolationType_ == InterpolationType(i))) {
                    EditorController::getInstance()->pushCommand(
                        std::make_unique<SetterCommand<InterpolationType>>(&uvInterpolationType_, InterpolationType(i)));
                }
            }
            ImGui::EndCombo();
        }
        if (ImGui::BeginCombo("Color InterpolationType", InterpolationTypeName[int(colorInterpolationType_)])) {
            for (int i = 0; i < (int)InterpolationType::COUNT; ++i) {
                if (ImGui::Selectable(InterpolationTypeName[i], colorInterpolationType_ == InterpolationType(i))) {
                    EditorController::getInstance()->pushCommand(
                        std::make_unique<SetterCommand<InterpolationType>>(&colorInterpolationType_, InterpolationType(i)));
                }
            }
            ImGui::EndCombo();
        }

        if (ImGui::BeginTable("MaterialKeyFrames", 2, tableFlags)) {
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
            isChange |= ImGui::EditColorKeyFrame(
                "##MaterialAnimation Color",
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
            ImGui::TextUnformatted("UV");
            ImGui::TableSetColumnIndex(1);
            isChange |= ImGui::EditKeyFrame(
                "##MaterialAnimation UV Scale",
                uvScaleCurve_,
                duration_);

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
            isChange |= ImGui::EditKeyFrame(
                "##MaterialAnimation UV Rotate",
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

            isChange |= ImGui::EditKeyFrame(
                "##MaterialAnimation UV Translate",
                uvTranslateCurve_,
                duration_);

            ImGui::EndTable();
        }

        ImGui::TreePop();
    }

    return isChange;
#else
    return false;
#endif // _DEBUG
}

void PrimitiveNodeAnimation::Finalize() {
    transformAnimationState_.isLoop_ = false;
    transformAnimationState_.isPlay_ = false;
    transformAnimationState_.isEnd_  = false;
    materialAnimationState_.isLoop_  = false;
    materialAnimationState_.isPlay_  = false;
    materialAnimationState_.isEnd_   = false;
    currentTime_                     = 0.0f;

    scaleCurve_.clear();
    rotateCurve_.clear();
    translateCurve_.clear();

    colorCurve_.clear();
    uvScaleCurve_.clear();
    uvRotateCurve_.clear();
    uvTranslateCurve_.clear();
}

void PrimitiveNodeAnimation::Update(float _deltaTime, Transform* _transform, Material* _material) {
    transformAnimationState_.isEnd_ = false;

    currentTime_ += _deltaTime;
    if (currentTime_ >= duration_) {
        if (transformAnimationState_.isLoop_) {
            currentTime_ = 0.0f;
        } else {
            transformAnimationState_.isEnd_  = true;
            transformAnimationState_.isPlay_ = false;
        }
        if (materialAnimationState_.isLoop_) {
            currentTime_ = 0.0f;
        } else {
            materialAnimationState_.isEnd_  = true;
            materialAnimationState_.isPlay_ = false;
        }
    }

    UpdateTransformAnimation(_transform);
    UpdateMaterialAnimation(_material);
}

void PrimitiveNodeAnimation::PlayStart() {
    currentTime_ = 0.f;

    transformAnimationState_.isEnd_  = false;
    transformAnimationState_.isPlay_ = true;

    materialAnimationState_.isEnd_  = false;
    materialAnimationState_.isPlay_ = true;
}

void PrimitiveNodeAnimation::Stop() {
    currentTime_                     = 0.f;
    transformAnimationState_.isEnd_  = true;
    transformAnimationState_.isPlay_ = false;
    materialAnimationState_.isEnd_   = true;
    materialAnimationState_.isPlay_  = false;
}

void PrimitiveNodeAnimation::UpdateTransformAnimation(Transform* _transform) {
    if (!transformAnimationState_.isLoop_) {
        if (!transformAnimationState_.isPlay_) {
            return;
        }
    }

    transformAnimationState_.isEnd_ = false;

    switch (transformInterpolationType_) {
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
    _transform->Update();
}

void PrimitiveNodeAnimation::UpdateMaterialAnimation(Material* _material) {
    if (materialAnimationState_.isLoop_) {
        if (!materialAnimationState_.isPlay_) {
            return;
        }
    }
    materialAnimationState_.isEnd_ = false;

    switch (colorInterpolationType_) {
    case InterpolationType::LINEAR:
        _material->color_ = CalculateValue::Linear(colorCurve_, currentTime_);
        break;
    case InterpolationType::STEP:
        _material->color_ = CalculateValue::Step(colorCurve_, currentTime_);
        break;
    default:
        break;
    }

    switch (uvInterpolationType_) {
    case InterpolationType::LINEAR:
        _material->uvTransform_.scale_     = CalculateValue::Linear(uvScaleCurve_, currentTime_);
        _material->uvTransform_.rotate_    = CalculateValue ::Linear(uvRotateCurve_, currentTime_);
        _material->uvTransform_.translate_ = CalculateValue::Linear(uvTranslateCurve_, currentTime_);
        break;
    case InterpolationType::STEP:
        _material->uvTransform_.scale_     = CalculateValue::Step(uvScaleCurve_, currentTime_);
        _material->uvTransform_.rotate_    = CalculateValue ::Step(uvRotateCurve_, currentTime_);
        _material->uvTransform_.translate_ = CalculateValue::Step(uvTranslateCurve_, currentTime_);
        break;
    default:
        break;
    }

    _material->UpdateUvMatrix();
}

void to_json(nlohmann::json& _json, const PrimitiveNodeAnimation& _primitiveNodeAnimation) {
    _json["duration"] = _primitiveNodeAnimation.duration_;
    _json["isLoop"]   = _primitiveNodeAnimation.transformAnimationState_.isLoop_;
    _json["isPlay"]   = _primitiveNodeAnimation.transformAnimationState_.isPlay_;
    _json["uvIsLoop"] = _primitiveNodeAnimation.materialAnimationState_.isLoop_;
    _json["uvIsPlay"] = _primitiveNodeAnimation.materialAnimationState_.isPlay_;

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

    _json["transformInterpolationType"] = _primitiveNodeAnimation.transformInterpolationType_;
    writeCurve("scaleCurve", _primitiveNodeAnimation.scaleCurve_);
    writeCurve("rotateCurve", _primitiveNodeAnimation.rotateCurve_);
    writeCurve("translateCurve", _primitiveNodeAnimation.translateCurve_);

    _json["colorInterpolationType"] = _primitiveNodeAnimation.colorInterpolationType_;
    writeCurve("colorCurve", _primitiveNodeAnimation.colorCurve_);

    _json["uvInterpolationType"] = _primitiveNodeAnimation.uvInterpolationType_;
    writeCurve("uvScaleCurve", _primitiveNodeAnimation.uvScaleCurve_);
    writeCurve("uvRotateCurve", _primitiveNodeAnimation.uvRotateCurve_);
    writeCurve("uvTranslateCurve", _primitiveNodeAnimation.uvTranslateCurve_);
}
void from_json(const nlohmann::json& _json, PrimitiveNodeAnimation& _primitiveNodeAnimation) {
    _json.at("duration").get_to(_primitiveNodeAnimation.duration_);
    _json.at("isLoop").get_to(_primitiveNodeAnimation.transformAnimationState_.isLoop_);
    _json.at("isPlay").get_to(_primitiveNodeAnimation.transformAnimationState_.isPlay_);

    _json.at("uvIsLoop").get_to(_primitiveNodeAnimation.materialAnimationState_.isLoop_);
    _json.at("uvIsPlay").get_to(_primitiveNodeAnimation.materialAnimationState_.isPlay_);

    auto readCurve = [&_json](const std::string& _name, auto& _curve) {
        for (const auto& keyJson : _json.at(_name)) {
            typename std::remove_reference<decltype(_curve)>::type::value_type key;
            keyJson.at("time").get_to(key.time);
            keyJson.at("value").get_to(key.value);
            _curve.push_back(key);
        }
    };
    _json.at("transformInterpolationType").get_to(_primitiveNodeAnimation.transformInterpolationType_);
    readCurve("scaleCurve", _primitiveNodeAnimation.scaleCurve_);
    readCurve("rotateCurve", _primitiveNodeAnimation.rotateCurve_);
    readCurve("translateCurve", _primitiveNodeAnimation.translateCurve_);

    _json.at("colorInterpolationType").get_to(_primitiveNodeAnimation.colorInterpolationType_);
    readCurve("colorCurve", _primitiveNodeAnimation.colorCurve_);

    _json.at("uvInterpolationType").get_to(_primitiveNodeAnimation.uvInterpolationType_);
    readCurve("uvScaleCurve", _primitiveNodeAnimation.uvScaleCurve_);
    readCurve("uvRotateCurve", _primitiveNodeAnimation.uvRotateCurve_);
    readCurve("uvTranslateCurve", _primitiveNodeAnimation.uvTranslateCurve_);
}
