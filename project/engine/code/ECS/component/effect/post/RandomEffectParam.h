#pragma once
#include "component/IComponent.h"

/// engine

// directX12
#include "directX12/IConstantBuffer.h"
#include "directX12/ShaderManager.h"

struct RandomEffectParamData {
    float time = 0.f;
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
    friend void to_json(nlohmann::json& j, const RandomEffectParam& param);
    friend void from_json(const nlohmann::json& j, RandomEffectParam& param);

public:
    RandomEffectParam() {}
    ~RandomEffectParam() override {}
    void Initialize(GameEntity* _hostEntity) override;
    bool Edit();
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
