#pragma once
#include "component/IComponent.h"

/// engine
// DirectX12
#include "directX12/buffer/IConstantBuffer.h"

/// math
#include "Vector4.h"

namespace OriGine {

/// <summary>
/// Vignette の パラメーター情報(GPUに送るもの)
/// </summary>
class VignetteBufferParam {
public:
    VignetteBufferParam()  = default;
    ~VignetteBufferParam() = default;

    Vector4<float> color = {0.f, 0.f, 0.f, 1.f};
    float vignetteScale  = 16.f; // vignetteのスケール
    float vignettePow    = 0.5f; // vignetteの強さ

public:
    struct ConstantBuffer {
        Vector4<float> color;
        float vignetteScale = 16.f;
        float vignettePow   = 0.5f; // vignetteの強さ

        ConstantBuffer& operator=(const VignetteBufferParam& _p) {
            vignettePow   = _p.vignettePow;
            vignetteScale = _p.vignetteScale;
            color         = _p.color;
            return *this;
        }
    };
};

/// <summary>
/// VignetteEffectのパラメーター
/// </summary>
class VignetteParam
    : public IComponent {
    friend void to_json(nlohmann::json& j, const VignetteParam& p);
    friend void from_json(const nlohmann::json& j, VignetteParam& p);

public:
    VignetteParam()           = default;
    ~VignetteParam() override = default;

    void Initialize(OriGine::Scene* _scene, OriGine::EntityHandle _owner)override;

    void Edit(Scene* _scene, EntityHandle _owner, const std::string& _parentLabel) override;
    void Finalize() override;

private:
    IConstantBuffer<VignetteBufferParam> paramBuffer;

public:
    const IConstantBuffer<VignetteBufferParam>& GetVignetteBuffer() const {
        return paramBuffer;
    }
    const VignetteBufferParam& GetVignetteBufferData() const {
        return paramBuffer.openData_;
    }
    void SetVignetteBufferData(const VignetteBufferParam& _p) {
        paramBuffer.openData_ = _p;
    }
    void SetVignetteColor(const Vector4<float>& _color) {
        paramBuffer.openData_.color = _color;
    }
    void SetVignetteScale(float _scale) {
        paramBuffer.openData_.vignetteScale = _scale;
    }
    void SetVignettePow(float _pow) {
        paramBuffer.openData_.vignettePow = _pow;
    }

    const VignetteBufferParam& GetVignetteParam() const {
        return paramBuffer.openData_;
    }
    void SetVignetteParam(const VignetteBufferParam& _p) {
        paramBuffer.openData_ = _p;
    }
};

} // namespace OriGine
