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

    float time = 0.f;

    struct ConstantBuffer {
        float time = 0.f;
        ConstantBuffer& operator=(const RandomEffectParamData& _param) {
            time = _param.time;
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
    friend void to_json(nlohmann::json& _j, const RandomEffectParam& _comp);
    friend void from_json(const nlohmann::json& _j, RandomEffectParam& _comp);

public:
    RandomEffectParam() {}
    ~RandomEffectParam() override {}
    void Initialize(Scene* _scene, EntityHandle _entity) override;
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
    void SetIsActive(bool _isActive) {
        isActive_ = _isActive;
    }

    float GetCurrentTime() const {
        return effectParamData_->time;
    }
    void SetCurrentTime(float _time) {
        effectParamData_->time = _time;
    }
    float GetMaxTime() const {
        return maxTime_;
    }
    BlendMode GetBlendMode() const {
        return blendMode_;
    }
    void SetBlendMode(BlendMode _mode) {
        blendMode_ = _mode;
    }
    const IConstantBuffer<RandomEffectParamData>& GetConstantBuffer() const {
        return effectParamData_;
    }
    IConstantBuffer<RandomEffectParamData>& GetConstantBufferRef() {
        return effectParamData_;
    }
};

} // namespace OriGine
