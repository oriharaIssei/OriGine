#pragma once

#include "component/IComponent.h"

/// stl
#include <string>

/// engine
// directX12
#include "directX12/buffer/IConstantBuffer.h"
#include "directX12/buffer/SimpleConstantBuffer.h"

// component
#include "component/material/Material.h"

/// math
#include "mathEnv.h"

namespace OriGine {

enum class GradationType : int {
    Directional = 0,
    Radial      = 1,
};

/// <summary>
/// Gradation のパラメーター情報(GPUに送る情報)
/// </summary>
struct GradationParam {
    Vec2f centerUv              = Vec2f(0.5f, 0.5f); // グラデ中心点
    Vec2f direction             = Vec2f(0.f, 1.f); // Directional のときだけ使用
    float scale                 = 1.f; // 変化スピード
    float pow                   = 1.f; // ベキ乗
    ColorChannel colorChannel   = ColorChannel::R;
    GradationType gradationType = GradationType::Directional;

    struct ConstantBuffer {
        Vec2f centerUv    = Vec2f(0.5f, 0.5f); // グラデ中心点
        Vec2f direction   = Vec2f(0.f, 1.f); // Directional のときだけ使用
        float scale       = 1.f; // 変化スピード
        float pow         = 1.f; // ベキ乗
        int colorChannel  = 0;
        int gradationType = 0; // 0 = Directional、1 = Radial

        ConstantBuffer& operator=(const GradationParam& _param) {
            centerUv  = _param.centerUv;
            direction = _param.direction;
            scale     = _param.scale;
            pow       = _param.pow;

            colorChannel  = static_cast<int>(_param.colorChannel);
            gradationType = static_cast<int>(_param.gradationType);
            return *this;
        }
    };
};

/// <summary>
/// GradiationTextureEffectのパラメーター
/// </summary>
class GradationComponent
    : public IComponent {
    friend void to_json(nlohmann::json& j, const GradationComponent& _g);
    friend void from_json(const nlohmann::json& j, GradationComponent& _g);

public:
    GradationComponent()           = default;
    ~GradationComponent() override = default;

    void Initialize(Scene* _scene, EntityHandle _entity) override;
    void Edit(Scene* _scene, EntityHandle _entity, const std::string& _parentLabel) override;
    void Finalize() override;

private:
    bool isActive_ = true;

    int32_t materialIndex_ = -1; // マテリアルインデックス

    IConstantBuffer<GradationParam> paramBuff_;
    SimpleConstantBuffer<ColorAndUvTransform> materialBuff_;

public:
    bool IsActive() const { return isActive_; }
    void SetActive(bool _active) { isActive_ = _active; }

    int32_t GetMaterialIndex() const { return materialIndex_; }

    IConstantBuffer<GradationParam>& GetParamBuff() { return paramBuff_; }
    SimpleConstantBuffer<ColorAndUvTransform>& GetMaterialBuff() {
        return materialBuff_;
    }
};

} // namespace OriGine
