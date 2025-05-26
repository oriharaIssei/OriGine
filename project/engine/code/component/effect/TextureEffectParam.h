#pragma once
#include "component/IComponent.h"

/// stl
#include <string>

/// engine
// directX12
#include "directX12/IConstantBuffer.h"
// component
#include "component/material/Material.h"
#include "component/animation/AnimationData.h"

/// math
#include <Matrix4x4.h>
#include <Vector2.h>



struct TextureEffectParamData {
    enum class EffectFlag : int32_t {
        None       = 0,
        Dissolve   = 1 << 0,
        Distortion = 1 << 1,
        Mask       = 1 << 2,
    };
    int32_t effectFlag = 0;

    UVTransform dissolveUV;
    Matrix4x4 dissolveUVMat;
    UVTransform distortionUV;
    Matrix4x4 distortionUVMat;
    UVTransform maskUV;
    Matrix4x4 maskUVMat;

    float distortionStrength = 0.f;
    float distortionBias     = 0.f;
    float dissolveThreshold  = 0.f;

    void UpdateTransform();

    struct ConstantBuffer {
        Matrix4x4 maskUVMat;
        Matrix4x4 dissolveUVMat;
        Matrix4x4 distortionUVMat;
        float distortionBias;
        float distortionStrength;
        float dissolveThreshold;
        int flag;
        int pad;

        ConstantBuffer& operator=(const TextureEffectParamData& transform) {
            maskUVMat          = transform.maskUVMat;
            dissolveUVMat      = transform.dissolveUVMat;
            distortionUVMat    = transform.distortionUVMat;
            dissolveThreshold  = transform.dissolveThreshold;
            distortionStrength = transform.distortionStrength;
            distortionBias     = transform.distortionBias;
            flag               = static_cast<int>(transform.effectFlag);

            return *this;
        }
    };
};

class TextureEffectParam
    : public IComponent {
    friend void to_json(nlohmann::json& j, const TextureEffectParam& param);
    friend void from_json(const nlohmann::json& j, TextureEffectParam& param);

public:
    TextureEffectParam() {}
    ~TextureEffectParam() = default;

    void Initialize(GameEntity* _hostEntity) override;
    bool Edit();
    void Finalize();

public:
    struct UVAnimation {
        UVAnimation()  = default;
        ~UVAnimation() = default;

        float duration    = 0.f;
        float currentTime = 0.f;
        AnimationCurve<Vec2f> scale;
        AnimationCurve<float> rotate;
        AnimationCurve<Vec2f> translate;
    };

private:
    std::string dissolveTexPath_   = "";
    std::string maskTexPath_       = "";
    std::string distortionTexPath_ = "";

    int32_t dissolveTexIndex_   = 0;
    int32_t maskTexIndex_       = 0;
    int32_t distortionTexIndex_ = 0;

    AnimationState dissolveAnimState_;

    UVAnimation dissolveAnim_;
    AnimationState distortionAnimState_;
    UVAnimation distortionAnim_;
    AnimationState maskAnimState_;
    UVAnimation maskAnim_;

    IConstantBuffer<TextureEffectParamData> effectParamData_;

public:
    TextureEffectParamData& getEffectParamData() {
        return effectParamData_.openData_;
    }
    IConstantBuffer<TextureEffectParamData>& getEffectParamBuffer() {
        return effectParamData_;
    }

    int32_t getDissolveTexIndex() const {
        return dissolveTexIndex_;
    }
    int32_t getDistortionTexIndex() const {
        return distortionTexIndex_;
    }
    int32_t getMaskTexIndex() const {
        return maskTexIndex_;
    }

    void LoadDissolveTexture(const std::string& path);
    void LoadDistortionTexture(const std::string& path);
    void LoadMaskTexture(const std::string& path);

    UVAnimation& getDissolveAnim() {
        return dissolveAnim_;
    }
    UVAnimation& getDistortionAnim() {
        return distortionAnim_;
    }
    UVAnimation& getMaskAnim() {
        return maskAnim_;
    }

    AnimationState& getDissolveAnimState() {
        return dissolveAnimState_;
    }
    AnimationState& getDistortionAnimState() {
        return distortionAnimState_;
    }
    AnimationState& getMaskAnimState() {
        return maskAnimState_;
    }
};
