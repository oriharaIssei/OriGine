#pragma once
#include "component/IComponent.h"

/// engine
// directX12
#include "directX12/buffer/IConstantBuffer.h"
#include "directX12/ShaderManager.h"

namespace OriGine {

/// <summary>
/// RandomEffectのパラメーター情報(GPUに送る情報)
/// </summary>
struct RandomEffectParamData {
    RandomEffectParamData()  = default;
    ~RandomEffectParamData() = default;

    float time               = 0.f;

    struct ConstantBuffer {
        float time = 0.f;
        ConstantBuffer& operator=(const RandomEffectParamData& param) {
            time = param.time;
            return *this;
        }
    };
};

/// <summary>
/// RandomEffectのパラメーター
/// </summary>
class RandomEffectParam
    : public IComponent {
public:
    friend void to_json(nlohmann::json& j, const RandomEffectParam& param);
    friend void from_json(const nlohmann::json& j, RandomEffectParam& param);

public:
    RandomEffectParam() {}
    ~RandomEffectParam() override {}
    void Initialize(Scene* _scene, EntityHandle _hostEntity) override;
    void Edit(Scene* _scene, EntityHandle _entity, const std::string& _parentLabel) override;
    void Finalize();

private:
    bool isActive_ = true; // エフェクトが有効かどうか

    IConstantBuffer<RandomEffectParamData> effectParamData_;
    float maxTime_ = 1000.f; // 最大時間

    BlendMode blendMode_ = BlendMode::Alpha;

public:
    bool IsActive() const {
        return isActive_;
    }
    void SetIsActive(bool isActive) {
        isActive_ = isActive;
    }

    float GetCurrentTime() const {
        return effectParamData_->time;
    }
    void SetCurrentTime(float time) {
        effectParamData_->time = time;
    }
    float GetMaxTime() const {
        return maxTime_;
    }
    BlendMode GetBlendMode() const {
        return blendMode_;
    }
    void SetBlendMode(BlendMode mode) {
        blendMode_ = mode;
    }
    const IConstantBuffer<RandomEffectParamData>& GetConstantBuffer() const {
        return effectParamData_;
    }
    IConstantBuffer<RandomEffectParamData>& GetConstantBufferRef() {
        return effectParamData_;
    }
};

} // namespace OriGine
