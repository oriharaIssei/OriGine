#pragma once

#include "component/IComponent.h"

/// stl
#include <string>

/// engine
// directX12 Object
#include "directX12/DxDescriptor.h"
#include "directX12/IConstantBuffer.h"
#include "directX12/IStructuredBuffer.h"
#include "directX12/ShaderManager.h"

/// ECS
// component
#include "component/material/Material.h"

/// math
#include <cstdint>
#include <math/Matrix4x4.h>
#include <math/Vector3.h>
#include <math/Vector4.h>

struct GpuParticleData {
    Vec3f scale;
    Vec3f translate;
    float lifeTime;
    Vec3f velocity;
    float currentTime;
    Vec4f color;

    struct ConstantBuffer {
        Vec3f scale;
        Vec3f translate;
        float lifeTime;
        Vec3f velocity;
        float currentTime;
        Vec4f color;
        ConstantBuffer& operator=(const GpuParticleData& other) {
            scale       = other.scale;
            translate   = other.translate;
            lifeTime    = other.lifeTime;
            velocity    = other.velocity;
            currentTime = other.currentTime;
            color       = other.color;
            return *this;
        }
    };
};
struct PerView {
    Matrix4x4 viewProjectionMat;
    Matrix4x4 billboardMat;

    struct ConstantBuffer {
        Matrix4x4 viewProjectionMat;
        Matrix4x4 billboardMat;

        ConstantBuffer& operator=(const PerView& other) {
            viewProjectionMat = other.viewProjectionMat;
            billboardMat      = other.billboardMat;
            return *this;
        }
    };
};

class GpuParticleEmitter
    : public IComponent {
    friend void to_json(nlohmann::json& j, const GpuParticleEmitter& p);
    friend void from_json(const nlohmann::json& j, GpuParticleEmitter& p);

public:
    GpuParticleEmitter()  = default;
    ~GpuParticleEmitter() = default;

    void Initialize(GameEntity* _entity) override;

    void Edit(Scene* _scene, GameEntity* _entity, const std::string& _parentLabel) override;

    void Finalize() override;

    /// <summary>
    /// Buffer を作成
    /// </summary>
    void CreateBuffer();

    void LoadTexture(const std::string& _path);

    void Play();
    void Stop();

private:
    bool isActive_         = false;
    BlendMode blendMode_   = BlendMode::Alpha;
    uint32_t particleSize_ = 1024;

    DxResource dxResource_;
    std::shared_ptr<DxSrvDescriptor> srvDescriptor_ = nullptr;
    std::shared_ptr<DxUavDescriptor> uavDescriptor_ = nullptr;

    IConstantBuffer<Material> materialBuffer_;

    std::string texturePath_ = "";
    uint32_t textureIndex_   = 0;

public:
    bool isActive() const { return isActive_; }
    uint32_t getParticleSize() const { return particleSize_; }
    BlendMode getBlendMode() const { return blendMode_; }
    const DxResource& getResource() const { return dxResource_; }

    std::shared_ptr<DxSrvDescriptor> getSrvDescriptor() const {
        return srvDescriptor_;
    }
    std::shared_ptr<DxUavDescriptor> getUavDescriptor() const {
        return uavDescriptor_;
    }

    const IConstantBuffer<Material>& getMaterialBuffer() const { return materialBuffer_; }

    const std::string& getTexturePath() const { return texturePath_; }
    uint32_t getTextureIndex() const { return textureIndex_; }
};
