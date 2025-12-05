#pragma once
#include "component/IComponent.h"

/// engine
// directX12
#include "directX12/buffer/ConstantBuffer.h"

/// math
#include "Vector2.h"
#include "Vector4.h"

/// <summary>
/// SpeedlineEffectのパラメーター情報(GPUに送る情報)
/// </summary>
struct SpeedlineBufferParamData {
    Vec2f screenCenterUV = {0.5f, 0.5f};
    float intensity      = 0.1f;
    float density        = 16.f;
    Vec4f color          = Vec4f(1.f, 1.f, 1.f, 1.f);
    float time           = 0.f;
    float fadeStart      = 0.f;
    float fadePow        = 1.f;

    struct ConstantBuffer {
        Vec2f screenCenterUV;
        float intensity;
        float density;
        Vec4f color;
        float time;
        float fadeStart = 0.f;
        float fadePow   = 1.f;
        ConstantBuffer& operator=(const SpeedlineBufferParamData& _p) {
            screenCenterUV = _p.screenCenterUV;
            intensity      = _p.intensity;
            density        = _p.density;
            color          = _p.color;
            time           = _p.time;
            fadeStart      = _p.fadeStart;
            fadePow        = _p.fadePow;
            return *this;
        }
    };
};

/// <summary>
/// SpeedlineEffectのパラメーター
/// </summary>
class SpeedlineEffectParam
    : public OriGine::IComponent {
    friend void to_json(nlohmann::json& j, const SpeedlineEffectParam& p);
    friend void from_json(const nlohmann::json& j, SpeedlineEffectParam& p);

public:
    SpeedlineEffectParam()           = default;
    ~SpeedlineEffectParam() override = default;
    void Initialize(Entity* _entity);
    void Edit(Scene* _scene, Entity* _entity, const std::string& _parentLabel);
    void Finalize();

    void Play();
    void Stop();

    void LoadRadialTexture(const std::string& _path);

private:
    bool isActive_ = false;
    ConstantBuffer<SpeedlineBufferParamData> cBuffer_;

    std::string radialTextureFilePath_ = "";
    uint32_t radialTextureIndex_        = 0;

public:
    bool IsActive() const { return isActive_; }

    uint32_t GetRadialTextureIndex() const { return radialTextureIndex_; }
    const std::string& GetRadialTextureFilePath() const { return radialTextureFilePath_; }

    const ConstantBuffer<SpeedlineBufferParamData>& GetBuffer() const { return cBuffer_; }
    SpeedlineBufferParamData& GetParamData() { return cBuffer_.openData_; }
};
