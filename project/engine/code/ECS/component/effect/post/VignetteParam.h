#pragma once
#include "component/IComponent.h"

/// engine
// DirectX12
#include "directX12/IConstantBuffer.h"

/// math
#include "Vector4.h"

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

class VignetteParam
    : public IComponent {
    friend void to_json(nlohmann::json& j, const VignetteParam& p);
    friend void from_json(const nlohmann::json& j, VignetteParam& p);

public:
    VignetteParam()           = default;
    ~VignetteParam() override = default;

    void Initialize(GameEntity* _entity);

    void Edit(Scene* _scene,GameEntity* _entity,const std::string& _parentLabel);
    void Finalize();

private:
    IConstantBuffer<VignetteBufferParam> paramBuffer;

public:
    const IConstantBuffer<VignetteBufferParam>& getVignetteBuffer() const {
        return paramBuffer;
    }
    const VignetteBufferParam& getVignetteBufferData() const {
        return paramBuffer.openData_;
    }
    void setVignetteBufferData(const VignetteBufferParam& _p) {
        paramBuffer.openData_ = _p;
    }
    void setVignetteColor(const Vector4<float>& _color) {
        paramBuffer.openData_.color = _color;
    }
    void setVignetteScale(float _scale) {
        paramBuffer.openData_.vignetteScale = _scale;
    }
    void setVignettePow(float _pow) {
        paramBuffer.openData_.vignettePow = _pow;
    }

    const VignetteBufferParam& getVignetteParam() const {
        return paramBuffer.openData_;
    }
    void setVignetteParam(const VignetteBufferParam& _p) {
        paramBuffer.openData_ = _p;
    }
};
