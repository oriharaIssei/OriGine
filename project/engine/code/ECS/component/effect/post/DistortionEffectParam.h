#pragma once

#include "component/IComponent.h"

/// stl
#include <memory>
#include <string>
#include <vector>

/// engine
// directX12
#include "directX12/IConstantBuffer.h"
class PrimitiveMeshRendererBase;
enum class PrimitiveType : int32_t;

// component
#include "component/material/Material.h"

struct DistortionParamData {
    UVTransform uvTransform;
    Matrix4x4 uvMat;

    float distortionBias     = 0.f;
    float distortionStrength = 0.f;

    void UpdateUVMat();

    struct ConstantBuffer {
        Matrix4x4 uvMat;
        float distortionBias;
        float distortionStrength;
        ConstantBuffer& operator=(const DistortionParamData& transform) {
            uvMat              = transform.uvMat;
            distortionBias     = transform.distortionBias;
            distortionStrength = transform.distortionStrength;
            return *this;
        }
    };
};

class DistortionEffectParam
    : public IComponent {
    friend void to_json(nlohmann::json& j, const DistortionEffectParam& param);
    friend void from_json(const nlohmann::json& j, DistortionEffectParam& param);

public:
    DistortionEffectParam() {}
    ~DistortionEffectParam() override {}

    void Initialize(GameEntity* _hostEntity) override;
    void Edit(Scene* _scene,GameEntity* _entity,const std::string& _parentLabel);
    void Finalize();

private:
    IConstantBuffer<DistortionParamData> effectParamData_;

    std::vector<std::pair<std::shared_ptr<PrimitiveMeshRendererBase>, PrimitiveType>> distortionObjects_;

public:
    DistortionParamData& getEffectParamData() {
        return effectParamData_.openData_;
    }
    IConstantBuffer<DistortionParamData>& getEffectParamBuffer() {
        return effectParamData_;
    }

    std::vector<std::pair<std::shared_ptr<PrimitiveMeshRendererBase>, PrimitiveType>>& getDistortionObjects() {
        return distortionObjects_;
    }
};
