#pragma once
#include "component/IComponent.h"

/// engine

// directX12
#include "directX12/buffer/IConstantBuffer.h"
#include "directX12/ShaderManager.h"

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

class RandomEffectParam
    : public IComponent {
public:
    friend void to_json(nlohmann::json& j, const RandomEffectParam& param);
    friend void from_json(const nlohmann::json& j, RandomEffectParam& param);

public:
    RandomEffectParam() {}
    ~RandomEffectParam() override {}
    void Initialize(Entity* _hostEntity) override;
    void Edit(Scene* _scene, Entity* _entity, const std::string& _parentLabel) override;
    void Finalize();

private:
    IConstantBuffer<RandomEffectParamData> effectParamData_;
    float maxTime_ = 1000.f; // 最大時間

    BlendMode blendMode_ = BlendMode::Alpha;

public:
    float getCurrentTime() const {
        return effectParamData_->time;
    }
    void setCurrentTime(float time) {
        effectParamData_->time = time;
    }
    float getMaxTime() const {
        return maxTime_;
    }
    BlendMode getBlendMode() const {
        return blendMode_;
    }
    void setBlendMode(BlendMode mode) {
        blendMode_ = mode;
    }
    const IConstantBuffer<RandomEffectParamData>& getConstantBuffer() const {
        return effectParamData_;
    }
    IConstantBuffer<RandomEffectParamData>& getConstantBufferRef() {
        return effectParamData_;
    }
};

inline void to_json(nlohmann::json& j, const RandomEffectParam& param) {
    j = nlohmann::json{
        {"maxTime", param.maxTime_},
        {"blendMode", static_cast<int>(param.blendMode_)}};
}

inline void from_json(const nlohmann::json& j, RandomEffectParam& param) {
    j.at("maxTime").get_to(param.maxTime_);
    int blendModeInt;
    j.at("blendMode").get_to(blendModeInt);
    param.blendMode_ = static_cast<BlendMode>(blendModeInt);
}
