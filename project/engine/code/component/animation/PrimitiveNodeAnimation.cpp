#include "PrimitiveNodeAnimation.h"

/// engine
#include "engine/code/Engine.h"

/// gui
#ifdef _DEBUG
#include "imgui/imgui.h"
#include "util/timeline/Timeline.h"
#endif // _DEBUG

void PrimitiveNodeAnimation::Initialize(GameEntity* /*_entity*/) {
    // Initialize animation state
    transformAnimationState_.isLoop_ = false;
    transformAnimationState_.isPlay_ = false;
    transformAnimationState_.isEnd_  = false;
    materialAnimationState_.isLoop_  = false;
    materialAnimationState_.isPlay_  = false;
    materialAnimationState_.isEnd_   = false;
    currentTime_                     = 0.0f;
}

bool PrimitiveNodeAnimation::Edit() {
#ifdef _DEBUG
    bool isChange = false;

    isChange |= ImGui::InputFloat("duration", &duration_);

    ImGuiTableFlags tableFlags = ImGuiTableFlags_Resizable | ImGuiTableFlags_ScrollX;
    if (ImGui::TreeNode("Transform_Animation")) {
        isChange |= ImGui::Checkbox("TransformAnimation Is Loop", &transformAnimationState_.isLoop_);
        isChange |= ImGui::Checkbox("TransformAnimation Is Play", &transformAnimationState_.isPlay_);
        ImGui::Combo(
            "TransformAnimation InterpolationType",
            (int*)&transformInterpolationType_,
            "LINEAR\0STEP\0\0");

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
        ImGui::DragFloat2("TileSize", tileSize_.v, 0.1f);
        ImGui::DragFloat2("TextureSize", textureSize_.v, 0.1f);
        ImGui::DragFloat("tilePerTime_", &tilePerTime_);
        ImGui::DragFloat("StartAnimationTime", &startAnimationTime_, 0.1f, 0);
        ImGui::DragFloat("AnimationTimeLength", &animationTimeLength_, 0.1f, 0);
        if (ImGui::Button("Generate Curve")) {
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

        ImGui::TreePop();
    }

    if (ImGui::TreeNode("Material_Animation")) {
        isChange |= ImGui::Checkbox("MaterialAnimation Is Loop", &materialAnimationState_.isLoop_);
        isChange |= ImGui::Checkbox("MaterialAnimation Is Play", &materialAnimationState_.isPlay_);

        ImGui::Combo(
            "MaterialAnimation InterpolationType",
            (int*)&uvInterpolationType_,
            "LINEAR\0STEP\0\0");
        ImGui::Combo(
            "Color InterpolationType",
            (int*)&colorInterpolationType_,
            "LINEAR\0STEP\0\0");

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

void PrimitiveNodeAnimation::Save(BinaryWriter& _writer) {
    _writer.Write("duration", duration_);

    _writer.Write("isLoop", transformAnimationState_.isLoop_);
    _writer.Write("isPlay", transformAnimationState_.isPlay_);
    _writer.Write("transformInterpolationType", transformInterpolationType_);

    _writer.Write("uvIsLoop", materialAnimationState_.isLoop_);
    _writer.Write("uvIsPlay", materialAnimationState_.isPlay_);
    _writer.Write("colorInterpolationType", colorInterpolationType_);
    _writer.Write("uvInterpolationType", uvInterpolationType_);

    WriteCurve("scaleCurve", scaleCurve_, _writer);
    WriteCurve("rotateCurve", rotateCurve_, _writer);
    WriteCurve("translateCurve", translateCurve_, _writer);

    WriteCurve("colorCurve", colorCurve_, _writer);
    WriteCurve("uvScaleCurve", uvScaleCurve_, _writer);
    WriteCurve("uvRotateCurve", uvRotateCurve_, _writer);
    WriteCurve("uvTranslateCurve", uvTranslateCurve_, _writer);
}

void PrimitiveNodeAnimation::Load(BinaryReader& _reader) {
    _reader.Read("duration", duration_);
    _reader.Read("isLoop", transformAnimationState_.isLoop_);
    _reader.Read("isPlay", transformAnimationState_.isPlay_);
    _reader.Read("transformInterpolationType", transformInterpolationType_);
    _reader.Read("uvIsLoop", materialAnimationState_.isLoop_);
    _reader.Read("uvIsPlay", materialAnimationState_.isPlay_);
    _reader.Read("colorInterpolationType", colorInterpolationType_);
    _reader.Read("uvInterpolationType", uvInterpolationType_);
    ReadCurve("scaleCurve", scaleCurve_, _reader);
    ReadCurve("rotateCurve", rotateCurve_, _reader);
    ReadCurve("translateCurve", translateCurve_, _reader);
    ReadCurve("colorCurve", colorCurve_, _reader);
    ReadCurve("uvScaleCurve", uvScaleCurve_, _reader);
    ReadCurve("uvRotateCurve", uvRotateCurve_, _reader);
    ReadCurve("uvTranslateCurve", uvTranslateCurve_, _reader);
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
        _material->uvScale_     = Vec3f(CalculateValue::Linear(uvScaleCurve_, currentTime_), 0.0f);
        _material->uvRotate_    = Vec3f(0.0f, 0.0f, CalculateValue ::Linear(uvRotateCurve_, currentTime_));
        _material->uvTranslate_ = Vec3f(CalculateValue::Linear(uvTranslateCurve_, currentTime_), 0.0f);
        break;
    case InterpolationType::STEP:
        _material->uvScale_     = Vec3f(CalculateValue::Step(uvScaleCurve_, currentTime_), 0.0f);
        _material->uvRotate_    = Vec3f(0.0f, 0.0f, CalculateValue::Step(uvRotateCurve_, currentTime_));
        _material->uvTranslate_ = Vec3f(CalculateValue::Step(uvTranslateCurve_, currentTime_), 0.f);
        break;
    default:
        break;
    }

    _material->UpdateUvMatrix();
}
